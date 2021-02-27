#include <map>
#include <vector>
#include <cstdio>
#include <exception>

#include "hhal.h"

#include "arguments.h"

#include "mango_arguments.h"
#include "gn_dummy_rm.h"

int event_id_gen = 0;
std::map<int, int> kernel_id_to_unit_id;
std::map<int, mango::mango_addr_t> event_addresses;

using namespace hhal;

void resource_allocation(HHAL &hhal, std::vector<registered_kernel> kernels, std::vector<registered_buffer> buffers, std::vector<mango_event> events) {
	printf("[DummyRM] resource_allocation\n");

	static int u_pid = 0;
	static int kernel_address = 0xC0FFEE;
	static int event_address  = 0xDECADE;
	static int buffer_address = 0xBEEF;

    mango_cluster_id_t default_cluster_id = 0;
    mango_mem_id_t default_memory = 0;
    mango_size_t num_tiles = kernels.size();

    std::vector<mango_unit_type_t> types;
    std::vector<mango_unit_id_t> tiles_dst(num_tiles);

    auto unit_type = UnitType::GN;
    for(auto &k : kernels) {
        types.push_back(unit_type);
        printf("[DummyRM] resource_allocation: kernel %d type %d\n", k.k.id, static_cast<int>(unit_type));
    }
    
    auto status = hhal.gn_manager.find_units_set(default_cluster_id, types, tiles_dst);
    if (status == GNManagerExitCode::ERROR){
        for (unsigned int i = 0; i< num_tiles; i++){
            tiles_dst[i]= u_pid++;
        }
    } else if (status != GNManagerExitCode::OK){
        printf("[DummyRM] resource_allocation: tiles mapping not found\n");
        throw std::runtime_error("DummyRM resource allocation error!");
    }

    printf("[DummyRM] resource_allocation: %d tiles found\n", num_tiles);

    status = hhal.gn_manager.reserve_units_set(default_cluster_id, tiles_dst);
    if (status != GNManagerExitCode::OK){
        printf("[DummyRM] resource_allocation: tiles reservation failed\n");
        throw std::runtime_error("DummyRM resource allocation error!");
    }

    mango_unit_id_t unit = 0;
    for (auto &k : kernels) {
        gn_kernel kernel_info;
        kernel_info.id = k.k.id;
        kernel_info.cluster_id = default_cluster_id;
        kernel_info.mem_tile = default_memory;
        kernel_info.physical_addr = kernel_address++;
        kernel_info.size = k.k.image_size;
        kernel_info.unit_id = tiles_dst[unit];
        kernel_info.task_events = k.task_events;
        kernel_info.termination_event = k.kernel_termination_event;

        // Hack for deallocation
        kernel_id_to_unit_id[kernel_info.id] = kernel_info.unit_id;

        hhal.assign_kernel(hhal::Unit::GN, (hhal_kernel *) &kernel_info);
        unit++;
    }

    printf("[DummyRM] resource_allocation: %d tiles reserved\n", num_tiles);

	for(auto &et : events) {
        gn_event info;
        info.id = et.id;
        info.physical_addr = event_address++;
        info.cluster_id = default_cluster_id;
        info.kernels_in = et.kernels_in;
        info.kernels_out = et.kernels_out;

        mango_addr_t phy_addr = event_address++;
        auto status = hhal.gn_manager.get_synch_register_addr(default_cluster_id, &phy_addr, 1);
        if (status != GNManagerExitCode::OK){
            printf("[DummyRM] resource_allocation: not enough registers\n");
            hhal.gn_manager.release_units_set(default_cluster_id, tiles_dst);
            throw std::runtime_error("DummyRM resource allocation error!");
        }
        printf("[DummyRM] resource_allocation: event=%d, phy_addr=0x%x\n", et.id, phy_addr);
        info.physical_addr = phy_addr;
        // hack for deallocation
        event_addresses[info.id] = info.physical_addr;
        hhal.assign_event(hhal::Unit::GN, (hhal_event *) &info);
	}

	for(auto &bt : buffers) {
        gn_buffer info;
        info.id = bt.b.id;
        info.cluster_id = default_cluster_id;
        info.size = bt.b.size;
        info.event = bt.event;
        info.kernels_in = bt.b.kernels_in;
        info.kernels_out = bt.b.kernels_out;
        mango_mem_id_t memory = default_memory;
        mango_addr_t phy_addr = buffer_address++;

        mango_unit_id_t default_unit;
        if (bt.b.kernels_in.size() != 0) {
            default_unit = kernel_id_to_unit_id[bt.b.kernels_in.back()];
        } else if (bt.b.kernels_out.size() != 0) {
            default_unit = kernel_id_to_unit_id[bt.b.kernels_out.back()];
        }
        else {
            printf("[DummyRM] resource_allocation: buffer %d does not have any kernels in/out\n", bt.b.id);
            throw std::runtime_error("DummyRM resource allocation error!");
        }
        printf("[DummyRM] Fiding memory for cluster=%d, unit=%d, size=%zu\n", default_cluster_id, default_unit, info.size);
        auto status = hhal.gn_manager.find_memory(default_cluster_id, default_unit, info.size, &memory, &phy_addr);
        if (status != GNManagerExitCode::OK){
            printf("[DummyRM] resource_allocation: cannot find memory for buffer %d\n", info.id);
            throw std::runtime_error("DummyRM resource allocation error!");
        }
        printf("[DummyRM] resource_allocation: buffer=%d, memory=%d, phy_addr=0x%x\n", info.id, memory, phy_addr);
        info.mem_tile = memory;
        info.physical_addr = phy_addr;
        hhal.assign_buffer(hhal::Unit::GN, (hhal_buffer *) &info);
        hhal.allocate_memory(info.id);
	}

    hhal.gn_manager.do_memory_management();
    hhal.gn_manager.prepare_events_registers();
}

void resource_deallocation(HHAL &hhal, std::vector<mango_kernel> kernels, std::vector<mango_buffer> buffers, std::vector<mango_event> events) {
	printf("[DummyRM] resource_deallocation\n");
    mango_cluster_id_t default_cluster_id = 0;
    mango_size_t num_tiles = kernels.size();

    std::vector<mango_unit_type_t> types;
    std::vector<mango_unit_id_t> tiles_dst;

    auto unit_type = UnitType::GN;
    for (auto &k : kernels) {
        types.push_back(unit_type);
        tiles_dst.push_back(kernel_id_to_unit_id[k.id]);
    }

    auto status = hhal.gn_manager.release_units_set(default_cluster_id, tiles_dst);
    if (status != GNManagerExitCode::OK){
        printf("[DummyRM] resource_deallocation: tiles release failed\n");
        throw std::runtime_error("DummyRM resource allocation error!");
    }
    printf("[DummyRM] resource_deallocation: %d tiles released\n", num_tiles);

    for(auto &et : events) {
        hhal.gn_manager.release_synch_register_addr(default_cluster_id, event_addresses[et.id]);
        printf("[DummyRM] resource_deallocation: event=%d released\n", et.id);
    }

    for(auto &bt : buffers) {
        auto status = hhal.release_memory(bt.id);
        if (status != HHALExitCode::OK){
            printf("[DummyRM] resource_deallocation: release memory buffer %d failed\n", bt.id);
        }
        printf("[DummyRM] resource_deallocation: memory buffer %d released\n", bt.id);
    }

}

registered_kernel register_kernel(mango_kernel kernel) {
    return {
        kernel,
        event_id_gen++,
        // 3 "Kernel tasks events" are added for some reason, 
        // there is a TODO in libmango because it is not clear what this is doing or if its needed.
        {event_id_gen++, event_id_gen++, event_id_gen++},
    };
}

registered_buffer register_buffer(mango_buffer buffer) {
    return {
        buffer,
        event_id_gen++,
    };
};