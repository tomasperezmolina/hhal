#ifndef GN_MANAGER_H
#define GN_MANAGER_H

#include <map>
#include <string>
#include <cstdint>
#include <semaphore.h>

#include "gn/gn/hnemu/hnemu.h"
#include "gn/gn/mango_types.h"
#include "gn/types.h"

#include "gn/tlb.h"

using namespace mango;

typedef struct hhal_tile_description {
    int total_tiles;
    int tiles_x;
    int tiles_y;
} hhal_tile_description_t;

typedef enum hhal_tlb_entry_type {
    NORMAL_DATA,
    OPERATING_SYSTEM,
    EXECUTABLE_CODE,
    SYNCHRONIZATION_REGS
} hhal_tlb_entry_type_t;

namespace hhal {

enum class GNManagerExitCode {
    OK,
    ERROR,
};

class GNManager {
    public:
        GNManagerExitCode initialize();
        GNManagerExitCode finalize();

        GNManagerExitCode assign_kernel(gn_kernel *info);
        GNManagerExitCode assign_buffer(gn_buffer *info);
        GNManagerExitCode assign_event(gn_event *info);

        GNManagerExitCode kernel_write(int kernel_id, std::string image_path);
        GNManagerExitCode kernel_start_string_args(int kernel_id, std::string arguments);

        GNManagerExitCode allocate_memory(int buffer_id);
        GNManagerExitCode release_memory(int buffer_id);

        GNManagerExitCode write_to_memory(int buffer_id, const void *source, size_t size);
        GNManagerExitCode read_from_memory(int buffer_id, void *dest, size_t size);
        GNManagerExitCode write_sync_register(int event_id, uint8_t data);
        GNManagerExitCode read_sync_register(int event_id, uint8_t *data);

        mango_size_t get_memory_size(mango_cluster_id_t cluster, mango_mem_id_t memory) const;
        mango_addr_t get_first_virtual_address(mango_unit_type_t unit_type, hhal_tlb_entry_type_t entry_type) const;
        GNManagerExitCode find_memory(mango_cluster_id_t cluster, mango_unit_id_t unit, mango_size_t size, mango_mem_id_t *memory, mango_addr_t *phy_addr);
        GNManagerExitCode find_units_set(mango_cluster_id_t cluster, std::vector<mango_unit_type_t> &types, std::vector<mango_unit_id_t> &tiles_dst);
        GNManagerExitCode reserve_units_set(mango_cluster_id_t cluster, const std::vector<mango_unit_id_t> &tiles);
        GNManagerExitCode release_units_set(mango_cluster_id_t cluster, const std::vector<mango_unit_id_t> &tiles);

        GNManagerExitCode get_synch_register_addr(mango_cluster_id_t cluster, mango_addr_t *reg_address, bool isINCRWRITE_REG);
        void release_synch_register_addr(mango_cluster_id_t cluster, mango_addr_t reg_address);

        inline hhal_tile_description_t get_tiles(mango_cluster_id_t cluster_id) const { return tiles.at(cluster_id); }

        GNManagerExitCode set_tlb_entry(mango_cluster_id_t cluster, mango_unit_id_t unit,
                               hhal_tlb_entry_type_t type, mango_addr_t virtual_addr,
			                   mango_size_t size, mango_addr_t phy_addr, mango_mem_id_t memory) { return GNManagerExitCode::OK; }

        void clear_tlb() {}

        mango_addr_t get_virtual_address_alignment(hhal_tlb_entry_type_t) const { return 0x0; }

        GNManagerExitCode do_memory_management();
        GNManagerExitCode prepare_events_registers();

        int num_clusters;
        std::map<int, TLB> tlbs;
        std::map<int, gn_kernel> kernel_info;
        std::map<int, gn_event> event_info;
        std::map<int, gn_buffer> buffer_info;
    private:
        bool initialized = false;
        int max_buffers = 2048;
        int max_kernels = 2048;
        

        std::map<mango_cluster_id_t, hhal_tile_description_t> tiles;

        ConsoleLogger log_hhal;

        static mango_addr_t *mem;
        static sem_t *sem_id;
        static int f_mem;
        static std::map<mango_cluster_id_t, std::vector<mango_addr_t>> event_register_off;
        static void init_semaphore(void);
};

}

#endif
