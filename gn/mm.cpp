#include "gn/manager.h"

#include <assert.h>
#include <iostream>

using namespace mango;

namespace hhal {

void GNManager::MM::set_tlb_kb(GNManager &manager, mango_id_t unit, mango_id_t mem_bank, mango_addr_t starting_addr,
            mango_size_t size, mango_addr_t phy_addr, int is_kernel,
            uint32_t cluster_id) const noexcept {

    auto ec = manager.set_tlb_entry(cluster_id, unit,
                                is_kernel ? EXECUTABLE_CODE : NORMAL_DATA, starting_addr,
                                size, phy_addr, mem_bank);

    if (ec != GNManagerExitCode::OK) {
        printf("[Error] Setting TLB for cluster %d, unit %d, bank %d, addr 0x%lx, size %d, phy_addr 0x%lx FAILED\n", cluster_id, unit, mem_bank, starting_addr, size, phy_addr);
    }
}

GNManager::MM::MM() noexcept {
    printf("[info] Local Memory Manager (virtual addresses) initializing...");
}

GNManager::MM::~MM() noexcept {
    // Nothing to do
}

mango_exit_code_t GNManager::MM::set_vaddr_kernels(GNManager &manager, std::vector<gn_kernel> &kernels) noexcept {

    entries.clear();

    for(auto &k : kernels) {
        auto &tlb = manager.tlbs[k.id];

        mango_addr_t virt_addr_kernel;
        mango_addr_t phy_addr  = k.physical_addr;
        mango_id_t   mem_bank  = k.mem_tile;
        uint32_t   cluster_id  = k.cluster_id;
        mango_id_t   tile_unit = k.unit_id;
        mango_size_t kern_size = k.size;
        mango_size_t tlb_area_size = kern_size;

        auto unit_arch = UnitType::GN;
        if (unit_arch == mango_unit_type_t::PEAK) {
            // In PEAK the stack area is fixed and we do not have paging,
            // thus we have to allocate a fixed 256MB area for the kernel
            // @see issue#9
            // https://bitbucket.org/mango_developers/mangolibs/issues/9/
            tlb_area_size = 256 * 1024 * 1024;
        }

        virtual_address_pool[k.id] = manager.get_first_virtual_address(unit_arch, hhal_tlb_entry_type_t::NORMAL_DATA);
        virt_addr_kernel           = manager.get_first_virtual_address(unit_arch, hhal_tlb_entry_type_t::EXECUTABLE_CODE);

        // We have only one kernel per unit, so the virtual address is fixed
        set_tlb_kb(manager, tile_unit, mem_bank, virt_addr_kernel, tlb_area_size, phy_addr, 1, cluster_id);
        tlb.set_virt_addr(k, virt_addr_kernel);

        printf("[Notice] Mapped kernel image. [tile=%d, mem_bank=%d, phy_addr=0x%lx, "
                    "virt_addr_kernel=0x%lx, size=%d]\n", tile_unit, mem_bank, phy_addr,
                    virt_addr_kernel, tlb_area_size);
    }
    return ExitCode::SUCCESS;
}

mango_exit_code_t GNManager::MM::set_vaddr_buffers(GNManager &manager, std::vector<gn_buffer> &buffers) noexcept {

    for(auto &b : buffers){
        printf("[Debug] Mapping input buffers...\n");

        for(auto k_id : b.kernels_in) {
            if (k_id==0) continue;		// TODO What?
            set_buff_tlb(manager, manager.kernel_info[k_id], b);
        }

        printf("[Debug] Mapping output buffers...\n");
        for(auto k_id : b.kernels_out) {
            if (k_id==0) continue;		// TODO What?
            set_buff_tlb(manager, manager.kernel_info[k_id], b);
        }


    }
    return ExitCode::SUCCESS;

}

mango_exit_code_t GNManager::MM::set_vaddr_events(GNManager &manager, std::vector<gn_event> &events) noexcept {
    printf("[Debug] Mapping events...\n");

    for(auto &e : events) {
        for(auto &k_id : e.kernels_in) {
            if (k_id==0)
                continue;	 // TODO Check

            set_event_tlb(manager, manager.kernel_info[k_id], e);
        }

        for(auto &k_id : e.kernels_out) {
            if (k_id==0)
                continue;	 // TODO Check

            set_event_tlb(manager, manager.kernel_info[k_id], e);
        }
    }

    for(auto& k_pair : manager.kernel_info) {
        auto &k = k_pair.second;

        // const auto unit = k->get_assigned_unit();
        uint32_t cluster_id = k.cluster_id;
        mango_id_t   tile_unit;
        mango_addr_t start_addr;
        mango_addr_t end_addr;

        // const auto unit_arch = unit->get_arch();
        const auto unit_arch = UnitType::GN;

        tile_unit  = k.unit_id;
        start_addr = manager.get_first_virtual_address(unit_arch, hhal_tlb_entry_type_t::SYNCHRONIZATION_REGS);
        end_addr   = start_addr + 0xFFFFFF;

        printf("[Notice] Configured TLB for events of tile %d [0x%lx - 0x%lx]\n", tile_unit, start_addr, end_addr);

        manager.set_tlb_entry(cluster_id, tile_unit,
                            SYNCHRONIZATION_REGS, start_addr,
                            end_addr - start_addr + 1, 0, 0);

        for(auto e_id : k.task_events) {
            printf("[Info] Kernel id=%d: event id=%d\n", k.id, e_id);
            set_event_tlb(manager, k, manager.event_info[e_id]);
        }
    }

    return ExitCode::SUCCESS;
}

void GNManager::MM::set_buff_tlb(GNManager &manager, gn_kernel &k, gn_buffer &b) noexcept {

    auto &tlb = manager.tlbs[k.id];

    // The base for the virtual addresses of buffers must be set before call this function
    assert(virtual_address_pool.find(k.id) != virtual_address_pool.end());

    auto next_virtual_address = virtual_address_pool.at(k.id);

    printf("[Debug] Adding TLB entry for buffer %d address 0x%lx\n", b.id, next_virtual_address);
    tlb.set_virt_addr(b, next_virtual_address);

    mango_id_t   mem_bank  = b.mem_tile;
    mango_id_t   tile_unit = k.unit_id;
    mango_addr_t phy_addr  = b.physical_addr;
    mango_size_t buff_size = b.size;
    uint32_t    cluster_id = b.cluster_id;
    int entry = entries[tile_unit];		// If it doesn't exist, it will be created at 0

    set_tlb_kb(manager, tile_unit, mem_bank, next_virtual_address, buff_size, phy_addr, 0, cluster_id);

    entries[tile_unit] = entry + 1;

    printf("[Notice] Mapped buffer %d to kernel %d [virt_addr=0x%lx] [entry=%d]\n",
                b.id, k.id, next_virtual_address, entry);

    // Update the base address for the next buffer to allocate in the TLB
    next_virtual_address += b.size;

    auto alignment = manager.get_virtual_address_alignment(hhal_tlb_entry_type_t::NORMAL_DATA);
    if (alignment > 0 && ((next_virtual_address % alignment) != 0))
        next_virtual_address += alignment - (next_virtual_address % alignment);

    virtual_address_pool[k.id] = next_virtual_address;

}

void GNManager::MM::set_event_tlb(GNManager &manager, gn_kernel &k, gn_event &e) noexcept {

    // const auto unit = k->get_assigned_unit();
    const auto unit_arch = UnitType::GN;

    auto &tlb = manager.tlbs[k.id];

    const auto offset = manager.get_first_virtual_address(unit_arch, hhal_tlb_entry_type_t::SYNCHRONIZATION_REGS);

    // According to MANGO implementation, the physical address of synch register is actually the
    // offset of the virtual address (synchronization registers do not have a real physical
    // address!)
    tlb.set_virt_addr(e, offset + e.physical_addr);
}

} // namespace mango
