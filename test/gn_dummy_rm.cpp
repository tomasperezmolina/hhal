#include <map>
#include <vector>
#include <cstdio>
#include <exception>

#include "arguments.h"

#include "mango_arguments.h"
#include "gn_dummy_rm.h"
#include "rm_common.h"

#include "hhal_client.h"

using namespace hhal;

std::map<int, int> kernel_id_to_unit_id;
std::map<int, addr_t> event_addresses;

namespace gn_rm {

template <class H>
void resource_allocation(
    H &hhal,
    GNManager &gn_manager, 
    const std::vector<registered_kernel> &kernels, 
    const std::vector<registered_buffer> &buffers, 
    const std::vector<mango_event> &events
) {
	printf("[DummyRM] resource_allocation\n");

	static int u_pid = 0;
	static int kernel_address = 0xC0FFEE;
	static int event_address  = 0xDECADE;
	static int buffer_address = 0xBEEF;

    uint32_t default_cluster_id = 0;
    uint32_t default_memory = 0;
    uint32_t num_tiles = kernels.size();

    std::vector<uint32_t> tiles_dst(num_tiles);
    
    auto status = gn_manager.find_units_set(default_cluster_id, num_tiles, tiles_dst);
    if (status == GNManagerExitCode::ERROR){
        for (unsigned int i = 0; i< num_tiles; i++){
            tiles_dst[i]= u_pid++;
        }
    } else if (status != GNManagerExitCode::OK){
        printf("[DummyRM] resource_allocation: tiles mapping not found\n");
        throw std::runtime_error("DummyRM resource allocation error!");
    }

    printf("[DummyRM] resource_allocation: %d tiles found\n", num_tiles);

    status = gn_manager.reserve_units_set(default_cluster_id, tiles_dst);
    if (status != GNManagerExitCode::OK){
        printf("[DummyRM] resource_allocation: tiles reservation failed\n");
        throw std::runtime_error("DummyRM resource allocation error!");
    }

    uint32_t unit = 0;
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

        printf("[DummyRM] resource_allocation: kernel=%d, phy_addr=0x%x\n", kernel_info.id, kernel_info.physical_addr);

        hhal.assign_kernel(hhal::Unit::GN, (hhal_kernel *) &kernel_info);
        gn_manager.assign_kernel(&kernel_info);
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

        addr_t phy_addr = event_address++;
        auto status = gn_manager.get_synch_register_addr(default_cluster_id, &phy_addr, 1);
        if (status != GNManagerExitCode::OK){
            printf("[DummyRM] resource_allocation: not enough registers\n");
            gn_manager.release_units_set(default_cluster_id, tiles_dst);
            throw std::runtime_error("DummyRM resource allocation error!");
        }
        printf("[DummyRM] resource_allocation: event=%d, phy_addr=0x%x\n", et.id, phy_addr);
        info.physical_addr = phy_addr;
        // hack for deallocation
        event_addresses[info.id] = info.physical_addr;
        hhal.assign_event(hhal::Unit::GN, (hhal_event *) &info);
        gn_manager.assign_event(&info);
	}

	for(auto &bt : buffers) {
        gn_buffer info;
        info.id = bt.b.id;
        info.cluster_id = default_cluster_id;
        info.size = bt.b.size;
        info.event = bt.event;
        info.kernels_in = bt.b.kernels_in;
        info.kernels_out = bt.b.kernels_out;
        uint32_t memory = default_memory;
        addr_t phy_addr = buffer_address++;

        uint32_t default_unit;
        if (bt.b.kernels_in.size() != 0) {
            default_unit = kernel_id_to_unit_id[bt.b.kernels_in.back()];
        } else if (bt.b.kernels_out.size() != 0) {
            default_unit = kernel_id_to_unit_id[bt.b.kernels_out.back()];
        }
        else {
            printf("[DummyRM] resource_allocation: buffer %d does not have any kernels in/out\n", bt.b.id);
            throw std::runtime_error("DummyRM resource allocation error!");
        }
        printf("[DummyRM] Finding memory for cluster=%d, unit=%d, size=%zu\n", default_cluster_id, default_unit, info.size);
        auto status = gn_manager.find_memory(default_cluster_id, default_unit, info.size, &memory, &phy_addr);
        if (status != GNManagerExitCode::OK){
            printf("[DummyRM] resource_allocation: cannot find memory for buffer %d\n", info.id);
            throw std::runtime_error("DummyRM resource allocation error!");
        }
        printf("[DummyRM] resource_allocation: buffer=%d, memory=%d, phy_addr=0x%x\n", info.id, memory, phy_addr);
        info.mem_tile = memory;
        info.physical_addr = phy_addr;
        hhal.assign_buffer(hhal::Unit::GN, (hhal_buffer *) &info);
        gn_manager.assign_buffer(&info);
        gn_manager.allocate_memory(info.id);
	}

    gn_manager.prepare_events_registers();
}

template void resource_allocation<hhal::HHAL>(
    hhal::HHAL &hhal, 
    GNManager &gn_manager, 
    const std::vector<registered_kernel> &kernels, 
    const std::vector<registered_buffer> &buffers, 
    const std::vector<mango_event> &events
);

template void resource_allocation<hhal_daemon::HHALClient>(
    hhal_daemon::HHALClient &hhal, 
    GNManager &gn_manager, 
    const std::vector<registered_kernel> &kernels, 
    const std::vector<registered_buffer> &buffers, 
    const std::vector<mango_event> &events
);

template <class H>
void resource_deallocation(
    H &hhal, 
    GNManager &gn_manager, 
    const std::vector<mango_kernel> &kernels, 
    const std::vector<mango_buffer> &buffers, 
    const std::vector<mango_event> &events
) {
	printf("[DummyRM] resource_deallocation\n");
    uint32_t default_cluster_id = 0;
    uint32_t num_tiles = kernels.size();

    std::vector<uint32_t> tiles_dst;

    for (auto &k : kernels) {
        tiles_dst.push_back(kernel_id_to_unit_id[k.id]);
    }

    auto status = gn_manager.release_units_set(default_cluster_id, tiles_dst);
    if (status != GNManagerExitCode::OK){
        printf("[DummyRM] resource_deallocation: tiles release failed\n");
        throw std::runtime_error("DummyRM resource allocation error!");
    }
    printf("[DummyRM] resource_deallocation: %d tiles released\n", num_tiles);

    for(auto &et : events) {
        gn_manager.release_synch_register_addr(default_cluster_id, event_addresses[et.id]);
        printf("[DummyRM] resource_deallocation: event=%d released\n", et.id);
    }

    for(auto &bt : buffers) {
        hhal.release_memory(bt.id);
    }

}

template void resource_deallocation<hhal::HHAL>(
    hhal::HHAL &hhal, 
    GNManager &gn_manager, 
    const std::vector<mango_kernel> &kernels, 
    const std::vector<mango_buffer> &buffers, 
    const std::vector<mango_event> &events
);

template void resource_deallocation<hhal_daemon::HHALClient>(
    hhal_daemon::HHALClient &hhal, 
    GNManager &gn_manager, 
    const std::vector<mango_kernel> &kernels, 
    const std::vector<mango_buffer> &buffers, 
    const std::vector<mango_event> &events
);

int get_new_event_id() {
    return rm_common::get_new_event_id();
}

registered_kernel register_kernel(mango_kernel kernel) {
    return {
        kernel,
        rm_common::get_new_event_id(),
        // 3 "Kernel tasks events" are added for some reason, 
        // there is a TODO in libmango because it is not clear what this is doing or if its needed.
        {rm_common::get_new_event_id(), rm_common::get_new_event_id(), rm_common::get_new_event_id()},
    };
}

registered_buffer register_buffer(mango_buffer buffer) {
    return {
        buffer,
        rm_common::get_new_event_id(),
    };
};

}