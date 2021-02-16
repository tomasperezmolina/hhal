#ifndef GN_MANAGER_H
#define GN_MANAGER_H

#include <map>
#include <string>
#include <cstdint>
#include <semaphore.h>

#include "arguments.h"

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
        GNManagerExitCode kernel_start2(int kernel_id, const Arguments &arguments);

        GNManagerExitCode allocate_memory(int buffer_id);
        GNManagerExitCode release_memory(int buffer_id);

        GNManagerExitCode write_to_memory(int buffer_id, const void *source, size_t size);
        GNManagerExitCode read_from_memory(int buffer_id, void *dest, size_t size);
        GNManagerExitCode write_sync_register(int event_id, uint8_t data);
        GNManagerExitCode read_sync_register(int event_id, uint8_t *data);

        GNManagerExitCode find_memory(mango_cluster_id_t cluster, mango_unit_id_t unit, mango_size_t size, mango_mem_id_t *memory, mango_addr_t *phy_addr);
        GNManagerExitCode find_units_set(mango_cluster_id_t cluster, std::vector<mango_unit_type_t> &types, std::vector<mango_unit_id_t> &tiles_dst);
        GNManagerExitCode reserve_units_set(mango_cluster_id_t cluster, const std::vector<mango_unit_id_t> &tiles);
        GNManagerExitCode release_units_set(mango_cluster_id_t cluster, const std::vector<mango_unit_id_t> &tiles);

        GNManagerExitCode get_synch_register_addr(mango_cluster_id_t cluster, mango_addr_t *reg_address, bool isINCRWRITE_REG);
        void release_synch_register_addr(mango_cluster_id_t cluster, mango_addr_t reg_address);

        GNManagerExitCode do_memory_management();
        GNManagerExitCode prepare_events_registers();

    private:
        int num_clusters;
        bool initialized = false;
        int max_buffers = 2048;
        int max_kernels = 2048;

        std::map<int, TLB> tlbs;
        std::map<int, gn_kernel> kernel_info;
        std::map<int, gn_event> event_info;
        std::map<int, gn_buffer> buffer_info;
        
        std::map<mango_cluster_id_t, hhal_tile_description_t> tiles;

        ConsoleLogger log_hhal;

        static mango_addr_t *mem;
        static sem_t *sem_id;
        static int f_mem;
        static std::map<mango_cluster_id_t, std::vector<mango_addr_t>> event_register_off;
        static void init_semaphore(void);

        GNManagerExitCode get_string_arguments(int kernel_id, Arguments &args, std::string &str_args);

        inline GNManagerExitCode set_tlb_entry(mango_cluster_id_t cluster, mango_unit_id_t unit,
                               hhal_tlb_entry_type_t type, mango_addr_t virtual_addr,
			                   mango_size_t size, mango_addr_t phy_addr, mango_mem_id_t memory) { return GNManagerExitCode::OK; }

        mango_addr_t get_first_virtual_address(mango_unit_type_t unit_type, hhal_tlb_entry_type_t entry_type) const;
        inline mango_addr_t get_virtual_address_alignment(hhal_tlb_entry_type_t) const { return 0x0; }

        mango_size_t get_memory_size(mango_cluster_id_t cluster, mango_mem_id_t memory) const;

        /*! \brief Stub of a Memory Manager
        * Currently, it only manages in a very rough way the virtual addresses
        */
        class MM {

        public:
            MM() noexcept;

            ~MM();


            mango_exit_code_t set_vaddr_kernels(GNManager &manager, std::vector<gn_kernel> &kernels) noexcept;
                
            mango_exit_code_t set_vaddr_buffers(GNManager &manager, std::vector<gn_buffer> &buffers) noexcept;

            mango_exit_code_t set_vaddr_events(GNManager &manager, std::vector<gn_event> &events) noexcept;


        private:

            std::map<mango_id_t, int> entries;
            std::map<mango_id_t, mango_addr_t> virtual_address_pool;    /** This is used to keep track of
                                                                            used virtual addresses of buffers.
                                                                            It maps kernel id to next free
                                                                            virtual address. */

            void set_tlb_kb(GNManager &manager, mango_id_t unit, mango_id_t mem_bank, mango_addr_t starting_addr,
                    mango_size_t size, mango_addr_t phy_addr, int entry,
                    uint32_t cluster_id) const noexcept;

            virtual void set_buff_tlb(GNManager &manager, gn_kernel &k, gn_buffer &b) noexcept;
            virtual void set_event_tlb(GNManager &manager, gn_kernel &k, gn_event &e) noexcept;

        };
};

}

#endif
