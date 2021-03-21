#ifndef GN_MANAGER_H
#define GN_MANAGER_H

#include <map>
#include <string>
#include <cstdint>
#include <semaphore.h>

#include "arguments.h"

#include "gn/types.h"

typedef struct hhal_tile_description {
    int total_tiles;
    int tiles_x;
    int tiles_y;
} hhal_tile_description_t;

namespace hhal {

// Replacement for mango_addr_t, need to figure out whether to make it 32 or 64.
// Probably it does depend on the architecture GN compiles for as we need to make sure any possible buffer value is aligned.
typedef uint32_t addr_t; 

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
        GNManagerExitCode kernel_start(int kernel_id, const Arguments &arguments);

        GNManagerExitCode allocate_kernel(int kernel_id);
        GNManagerExitCode release_kernel(int kernel_id);
        GNManagerExitCode allocate_memory(int buffer_id);
        GNManagerExitCode release_memory(int buffer_id);
        GNManagerExitCode allocate_event(int event_id);
        GNManagerExitCode release_event(int event_id);

        GNManagerExitCode write_to_memory(int buffer_id, const void *source, size_t size);
        GNManagerExitCode read_from_memory(int buffer_id, void *dest, size_t size);
        GNManagerExitCode write_sync_register(int event_id, uint32_t data);
        GNManagerExitCode read_sync_register(int event_id, uint32_t *data);

    private:
        int num_clusters;
        bool initialized = false;
        int max_buffers = 2048;
        int max_kernels = 2048;

        std::map<int, gn_kernel> kernel_info;
        std::map<int, gn_event> event_info;
        std::map<int, gn_buffer> buffer_info;

        std::map<int, std::string> kernel_images;
        
        std::map<uint32_t, hhal_tile_description_t> tiles;

        static addr_t *mem;
        static sem_t *sem_id;
        static int f_mem;
        static std::map<uint32_t, std::vector<addr_t>> event_register_off;
        static void init_semaphore(void);

        GNManagerExitCode get_string_arguments(int kernel_id, Arguments &args, std::string &str_args);
        GNManagerExitCode kernel_start_string_args(int kernel_id, std::string arguments);
        GNManagerExitCode find_memory(uint32_t cluster, uint32_t unit, uint32_t size, uint32_t *memory, addr_t *phy_addr);
        GNManagerExitCode find_units_set(uint32_t cluster, uint32_t num_tiles, std::vector<uint32_t> &tiles_dst);
        GNManagerExitCode reserve_units_set(uint32_t cluster, const std::vector<uint32_t> &tiles);
        GNManagerExitCode release_units_set(uint32_t cluster, const std::vector<uint32_t> &tiles);

        GNManagerExitCode get_synch_register_addr(uint32_t cluster, addr_t *reg_address, bool isINCRWRITE_REG);
        void release_synch_register_addr(uint32_t cluster, addr_t reg_address);

        uint32_t get_memory_size(uint32_t cluster, uint32_t memory) const;
};

}

#endif
