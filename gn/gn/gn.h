#ifndef MOD_HHAL_GNLIB_H_
#define MOD_HHAL_GNLIB_H_


#include "hhal.h"
#include "hnemu/hnemu.h"
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

#define MANGO_SEMAPHORE       "mango_sem"
#define MANGO_DEVICE_MEMORY   "/tmp/device_memory.dat"
#define MANGO_REG_SIZE (128)
#define MANGO_REG_UNUSED 0
#define MANGO_REG_INUSE 1

#define NUM_VNS HN_MAX_VNS



#define GN_RETURN_DIR  "/bin/gn_return";

namespace mango {

class GNLibHHAL : public HHAL {

public:

    GNLibHHAL() noexcept;

    virtual void initialize() override;

    virtual void finalize() override;

    virtual void kernel_write(mango_cluster_id_t cluster, std::string image_path, mango_mem_id_t memory, mango_addr_t kernel_addr) override;

    virtual void kernel_start(mango_cluster_id_t cluster, mango_unit_id_t unit, mango_addr_t kernel_addr, std::string argument_string) override;

    virtual void write_to_memory (mango_cluster_id_t cluster, mango_mem_id_t memory, mango_addr_t dest_address, const void* source_buffer, mango_size_t size) override;

    virtual void read_from_memory(mango_cluster_id_t cluster, mango_mem_id_t memory, mango_addr_t source_address, void* dest_buffer, mango_size_t size) override;

    virtual void write_sync_register(mango_cluster_id_t cluster, mango_addr_t reg_address, uint8_t data) override;

    virtual uint8_t read_sync_register(mango_cluster_id_t cluster, mango_addr_t reg_address) override;

    virtual mango_size_t get_memory_size(mango_cluster_id_t cluster, mango_mem_id_t memory) const override;

    virtual void set_tlb_entry(mango_cluster_id_t cluster, mango_unit_id_t unit,
                               hhal_tlb_entry_type_t type, mango_addr_t virtual_addr,
			                   mango_size_t size, mango_addr_t phy_addr, mango_mem_id_t memory) override;

    virtual void clear_tlb() override {}

    virtual mango_addr_t get_first_virtual_address(mango_unit_type_t, hhal_tlb_entry_type_t) const override;
    virtual mango_addr_t get_virtual_address_alignment(hhal_tlb_entry_type_t) const override  { return 0x0; }

    virtual mango_exit_code_t get_synch_register_addr (mango_cluster_id_t cluster, mango_addr_t *phy_addr,
            bool isINCRWRITE_REG) override;
    virtual void release_synch_register_addr (mango_cluster_id_t cluster, mango_addr_t reg_address) override;
    virtual mango_exit_code_t find_memory(mango_cluster_id_t cluster, mango_unit_id_t unit,
                             mango_size_t size, mango_mem_id_t *memory, mango_addr_t *phy_addr) override;

    virtual mango_exit_code_t allocate_memory(mango_cluster_id_t cluster, mango_mem_id_t memory,
                                 mango_addr_t phy_addr, mango_size_t size) override;
    virtual mango_exit_code_t release_memory(mango_cluster_id_t cluster, mango_mem_id_t memory,
                                mango_addr_t phy_addr, mango_size_t size) override;
    mango_exit_code_t find_units_set(mango_cluster_id_t cluster, std::vector<mango_unit_type_t> &types,
                                     std::vector<mango_unit_id_t> &tiles_dst)override;
    mango_exit_code_t reserve_units_set(mango_cluster_id_t cluster, const std::vector<mango_unit_id_t> &tiles)override;
    mango_exit_code_t release_units_set(mango_cluster_id_t cluster, const std::vector<mango_unit_id_t> &tiles)override;

private:
    bool initialized;
    ConsoleLogger log_hhal;

    static mango_addr_t *mem;
    static sem_t *sem_id;
    static int f_mem;
    static std::map<mango_cluster_id_t, std::vector<mango_addr_t>> event_register_off;
    static void init_semaphore(void);

};

}


#endif
