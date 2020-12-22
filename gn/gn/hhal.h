#ifndef MOD_HHAL_H_
#define MOD_HHAL_H_

#include "mango_types.h"

#include <cassert>
#include <cstdint>
#include <map>
#include <vector>
#include <memory>
#include <string>

namespace mango {

typedef struct hhal_tile_description {
    int total_tiles;
    int tiles_x;
    int tiles_y;
} hhal_tile_description_t;

typedef struct hhal_tile_stats {
    // TODO
} hhal_stats_t;

typedef enum hhal_tlb_entry_type {
    NORMAL_DATA,
    OPERATING_SYSTEM,
    EXECUTABLE_CODE,
    SYNCHRONIZATION_REGS
} hhal_tlb_entry_type_t;

class HHAL {

public:

    virtual ~HHAL() noexcept { };

    virtual void initialize() = 0;

    virtual void finalize()   = 0;

    virtual void kernel_write(mango_cluster_id_t cluster, std::string image_path, mango_mem_id_t memory, mango_addr_t kernel_addr) = 0;

    virtual void kernel_start(mango_cluster_id_t cluster, mango_unit_id_t unit, mango_addr_t kernel_addr, std::string argument_string) = 0;

    virtual void write_to_memory (mango_cluster_id_t cluster, mango_mem_id_t memory, mango_addr_t dest_address, const void* source_buffer, mango_size_t size) = 0;

    virtual void read_from_memory(mango_cluster_id_t cluster, mango_mem_id_t memory, mango_addr_t source_address, void* dest_buffer, mango_size_t size) = 0;

    virtual void write_sync_register(mango_cluster_id_t cluster, mango_addr_t reg_address, uint8_t data) = 0;

    virtual uint8_t read_sync_register(mango_cluster_id_t cluster, mango_addr_t reg_address) = 0;

    virtual mango_size_t get_memory_size(mango_cluster_id_t cluster, mango_mem_id_t memory) const = 0;

    virtual void set_tlb_entry(mango_cluster_id_t cluster, mango_unit_id_t unit,
                               hhal_tlb_entry_type_t type, mango_addr_t virtual_addr,
			                   mango_size_t size, mango_addr_t phy_addr, mango_mem_id_t memory) = 0;

    virtual void clear_tlb() = 0;

	virtual mango_addr_t get_first_virtual_address(mango_unit_type_t, hhal_tlb_entry_type_t) const = 0;
	virtual mango_addr_t get_virtual_address_alignment(hhal_tlb_entry_type_t) const = 0;


    virtual mango_exit_code_t get_synch_register_addr (mango_cluster_id_t cluster, mango_addr_t *phy_addr,
            bool isINCRWRITE_REG){
        return ExitCode::ERR_FEATURE_NOT_IMPLEMENTED;
    }
    virtual void release_synch_register_addr (mango_cluster_id_t cluster, mango_addr_t reg_address) {};
    virtual mango_exit_code_t find_memory(mango_cluster_id_t cluster, mango_unit_id_t unit,
            mango_size_t size, mango_mem_id_t *memory, mango_addr_t *phy_addr){
        return ExitCode::ERR_FEATURE_NOT_IMPLEMENTED;
    }
    virtual mango_exit_code_t allocate_memory(mango_cluster_id_t cluster, mango_mem_id_t memory,
            mango_addr_t phy_addr, mango_size_t size){
        return ExitCode::ERR_FEATURE_NOT_IMPLEMENTED;
    }
    virtual mango_exit_code_t release_memory(mango_cluster_id_t cluster, mango_mem_id_t memory,
            mango_addr_t phy_addr, mango_size_t size){
        return ExitCode::ERR_FEATURE_NOT_IMPLEMENTED;
    }
    virtual mango_exit_code_t find_units_set(mango_cluster_id_t cluster, std::vector<mango_unit_type_t> &types,
            std::vector<mango_unit_id_t> &tiles_dst){
        return ExitCode::ERR_FEATURE_NOT_IMPLEMENTED;
    }
    virtual mango_exit_code_t reserve_units_set(mango_cluster_id_t cluster, const std::vector<mango_unit_id_t> &tiles){
        return ExitCode::ERR_FEATURE_NOT_IMPLEMENTED;
    }
    virtual mango_exit_code_t release_units_set(mango_cluster_id_t cluster, const std::vector<mango_unit_id_t> &tiles){
        return ExitCode::ERR_FEATURE_NOT_IMPLEMENTED;
    }

    int get_max_nr_buffers() const {
        return this->max_buffers;
    }
    int get_max_nr_kernels() const {
        return this->max_kernels;
    }

    int get_num_clusters() const noexcept { return this->num_clusters; }

    hhal_tile_description_t get_tiles(mango_cluster_id_t cluster_id) const { return this->tiles.at(cluster_id); }

    static HHAL& get(std::unique_ptr<HHAL> instance = nullptr) noexcept {
        static std::unique_ptr<HHAL> hhal = std::move(instance);
        assert(hhal != nullptr);
        return *hhal;
    }

protected:
    HHAL() noexcept { };
    int num_clusters;
    int max_buffers;
    int max_kernels;
    std::map<mango_cluster_id_t, hhal_tile_description_t> tiles;

private:
    HHAL(const HHAL &) = delete;

};

} // namespace mango


#endif // MOD_HHAL_H_
