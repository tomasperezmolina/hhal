#include "gn.h"
#include <cassert>

#define UNUSED(x) ((void)x)

namespace mango {

mango_addr_t *GNLibHHAL::mem;
sem_t *GNLibHHAL::sem_id;
int GNLibHHAL::f_mem;
std::map<mango_cluster_id_t, std::vector<mango_addr_t>> GNLibHHAL::event_register_off;

GNLibHHAL::GNLibHHAL() noexcept {
    this->max_buffers = this->max_kernels = 2048;
    this->initialized = false;
}

void GNLibHHAL::init_semaphore(void) {

    mode_t old_mask = umask(0);

    GNLibHHAL::sem_id = sem_open(MANGO_SEMAPHORE, O_CREAT, 0666, 1);
    if (sem_id == SEM_FAILED) {
        throw std::runtime_error("GNLibHHAL: Unable to init semaphore.");
    }
    umask(old_mask);

}

void GNLibHHAL::initialize() {
    assert(initialized == false);

    uint32_t num_clusters;
    HNemu::instance()->get_num_clusters(&num_clusters);

    init_semaphore();
    if (sem_id == NULL) {
        return;
    }

    mode_t old_mask = umask(0);
    f_mem = open(MANGO_DEVICE_MEMORY,O_RDWR | O_CREAT, 0666);
    if (f_mem==-1) {
        log_hhal.Error("Unable to read device memory " MANGO_DEVICE_MEMORY);
        return;
    }

    umask(old_mask);

    unsigned long long mem_size = 0;
    for (uint32_t cluster_id = 0; cluster_id < num_clusters; cluster_id++){
        unsigned long long mem_size_cluster = 0;
        HNemu::instance()->get_memory_size(cluster_id, &mem_size_cluster);
        mem_size += mem_size_cluster;
    }

    assert(0 == ftruncate(f_mem, mem_size));

    mem = (mango_addr_t*) mmap (NULL, mem_size,
                            PROT_READ|PROT_WRITE, MAP_SHARED, f_mem,0);

    if (mem==MAP_FAILED) {
        log_hhal.Error("Memory not present in hn emulation configuration ");
        return;
    }
    for (mango_cluster_id_t cluster_id = 0; cluster_id< num_clusters; cluster_id++){
        hhal_tile_description_t cluster_info;
        auto info = HNemu::instance()->get_info(cluster_id);
        cluster_info.total_tiles = info->num_tiles;
        cluster_info.tiles_x = info->num_cols;
        cluster_info.tiles_y = info->num_rows;
        tiles.insert(std::make_pair(cluster_id, cluster_info));
    }
    this->num_clusters = num_clusters;

    // sync registers offset
    for (mango_cluster_id_t cluster_id = 0; cluster_id< num_clusters; cluster_id++) {
        std::vector <mango_addr_t> regs(MANGO_REG_SIZE, MANGO_REG_UNUSED);
        event_register_off.emplace(std::make_pair(cluster_id, regs));
    }

    // Reserve space for sync registers
    // registers for all clusters are reserved on cluster 0 close to tile 0
    mango_cluster_id_t default_cluster_id = 0;
    mango_size_t size = num_clusters * MANGO_REG_SIZE * sizeof(mango_addr_t);
    mango_mem_id_t memory = 0;
    mango_addr_t phy_addr = 0;
    uint32_t phy_addr_l = 0;
    HNemu::instance()->find_memory(default_cluster_id, 0, size, &memory, &phy_addr_l);
    phy_addr=phy_addr_l;
    int status = HNemu::instance()->allocate_memory(default_cluster_id, memory, phy_addr, size);
    if (status!=HN_SUCCEEDED){
        log_hhal.Error("[GNLibHHAL] memory allocation for sync registers failed: cluster=%d, memory=%d, phy_addr=0x%x, size=%d",
                       default_cluster_id, memory, phy_addr, size);
    } else {
        log_hhal.Debug("[GNLibHHAL] memory for sync registers allocated: cluster=%d, memory=%d, phy_addr=0x%x, size=%d",
                       default_cluster_id, memory, phy_addr, size);
    }



    initialized = true;
}

void GNLibHHAL::finalize() {
    assert(initialized == true);

    sem_close(sem_id);
    close(f_mem);

    initialized = false;
}

void GNLibHHAL::kernel_write(mango_cluster_id_t cluster, std::string image_path, mango_mem_id_t memory, mango_addr_t kernel_addr) {
    assert(initialized == true);
    assert(image_path.size() > 0);

    log_hhal.Debug("[GNLibHHAL] kernel_write: cluster=%d,  image_path=%s, memory=%d, address=0x%x",
            cluster, image_path.c_str(), memory,kernel_addr);
}

void GNLibHHAL::kernel_start(mango_cluster_id_t cluster, mango_unit_id_t unit, mango_addr_t kernel_addr, std::string argument_string) {
    assert(initialized == true);
    assert(argument_string.size() > 0);

    pid_t pid;
    pid = fork();
    if (!pid){ /* child, executor */
        if(argument_string[0] != '/') {
            argument_string.insert(0, "./");
        }
        printf("system(%s);\n", argument_string.c_str());
        auto ret = system(argument_string.c_str());
        UNUSED(ret);
        char n[11];
        std::string gn_return(MANGO_ROOT);  // MANGO_ROOT compile-time defined
        gn_return += GN_RETURN_DIR;
        execl(gn_return.c_str(), n, NULL);
        printf("***\n!!!\nEXECL ERROR\n!!!\n***\n");
        exit(0);
    }
    log_hhal.Debug("[GNLibHHAL] kernel_start: cluster=%d,  unit=%d, address=0x%x, argument_string=%s",
            cluster, unit, kernel_addr,argument_string.c_str());
}

void GNLibHHAL::write_to_memory (mango_cluster_id_t cluster, mango_mem_id_t memory, mango_addr_t dest_address, const void* source_buffer, mango_size_t size) {
    assert(initialized == true);
    assert(source_buffer != NULL);
    assert(size > 0);

    memcpy(mem + (dest_address/sizeof(mango_addr_t)), static_cast<char*>(const_cast<void*>(source_buffer)), size);
    log_hhal.Debug("[GNLibHHAL] write_to_memory: cluster=%d,  memory=%d, dest_address=0x%x, size=%d",
                   cluster, memory, dest_address, size);
}

void GNLibHHAL::read_from_memory(mango_cluster_id_t cluster, mango_mem_id_t memory, mango_addr_t source_address, void* dest_buffer, mango_size_t size) {
    assert(initialized == true);
    assert(dest_buffer != NULL);
    assert(size > 0);

    memcpy(static_cast<char*>(dest_buffer), mem + (source_address/sizeof(mango_addr_t)), size);

    log_hhal.Debug("[GNLibHHAL] read_from_memory: cluster=%d,  memory=%d, source_address=0x%x, size=%d",
                   cluster, memory, source_address, size);
}


mango_exit_code_t GNLibHHAL::get_synch_register_addr (mango_cluster_id_t cluster, mango_addr_t *reg_address, bool isINCRWRITE_REG) {

    mango_addr_t reg = MANGO_REG_SIZE+1;
    if ( isINCRWRITE_REG ) {
        for (int i = 1; (i < MANGO_REG_SIZE) && (reg > MANGO_REG_SIZE); i += 2) {
            if (event_register_off[cluster][i] == MANGO_REG_UNUSED) {
                reg = i;
                event_register_off[cluster][i] = MANGO_REG_INUSE;
            }
        }
    } else {
        for (int i = 0; (i< MANGO_REG_SIZE) && (reg > MANGO_REG_SIZE); i+=2 ) {
            if (event_register_off[cluster][i] == MANGO_REG_UNUSED) {
                reg = i;
                event_register_off[cluster][i] = MANGO_REG_INUSE;
            }
        }
    }

    if (reg <= MANGO_REG_SIZE ){
        *reg_address = reg*4 + cluster*MANGO_REG_SIZE*4 ;
        return ExitCode::SUCCESS;
    } else {
        return ExitCode::ERR_OTHER;
    }
}

void GNLibHHAL::release_synch_register_addr (mango_cluster_id_t cluster, mango_addr_t reg_address) {
    reg_address -= cluster*MANGO_REG_SIZE*4;
    reg_address /= sizeof(mango_addr_t);;
    event_register_off[cluster][reg_address] = MANGO_REG_UNUSED;
}

void GNLibHHAL::write_sync_register(mango_cluster_id_t cluster, mango_addr_t reg_address, uint8_t data) {
    assert(initialized == true);
    reg_address -= cluster*MANGO_REG_SIZE*4;
    reg_address /= sizeof(mango_addr_t);

    sem_wait(sem_id);

    if (reg_address % 8 != 0) {
        mem[reg_address] += data;
    } else {
        mem[reg_address] = data;
    }

    sem_post(sem_id);
    log_hhal.Debug("[GNLibHHAL] write_sync_register: cluster=%d,  reg_address=0x%x, data=%d",
                   cluster, reg_address, data);
    }

uint8_t GNLibHHAL::read_sync_register(mango_cluster_id_t cluster, mango_addr_t reg_address) {
    assert(initialized == true);

    reg_address -= cluster*MANGO_REG_SIZE*4;
    reg_address /= sizeof(mango_addr_t);

    sem_wait(sem_id);

    uint8_t data = mem[reg_address];
    mem[reg_address]=0;

    sem_post(sem_id);

    log_hhal.Debug("[GNLibHHAL] read_sync_register: cluster=%d, reg_address=0x%x, data=%d",
                   cluster, reg_address, data);

    return data;
}

mango_size_t GNLibHHAL::get_memory_size(mango_cluster_id_t cluster, mango_mem_id_t memory) const {
    assert(initialized == true);

    mango_size_t mem_size;
    int err = HNemu::instance()->get_memory_size (cluster, memory, &mem_size);

    if (err != 0) {
//        log_hhal.Error("[GNLibHHAL] Cannot get memory size: cluster=%d, memory=%d", cluster, memory);
    }

    return mem_size;
}

void GNLibHHAL::set_tlb_entry(mango_cluster_id_t cluster, mango_unit_id_t unit,
                               hhal_tlb_entry_type_t type, mango_addr_t virtual_addr,
			                   mango_size_t size, mango_addr_t phy_addr, mango_mem_id_t memory) {


}

mango_addr_t GNLibHHAL::get_first_virtual_address(mango_unit_type_t unit_type, hhal_tlb_entry_type_t entry_type) const {
    if(entry_type == hhal_tlb_entry_type_t::NORMAL_DATA)
        return num_clusters * MANGO_REG_SIZE * sizeof(mango_addr_t);
    if(entry_type == hhal_tlb_entry_type_t::EXECUTABLE_CODE)
        return 0x0;
    if(entry_type == hhal_tlb_entry_type_t::SYNCHRONIZATION_REGS)
        return 0x0;
    assert(false);
}


mango_exit_code_t GNLibHHAL::find_memory(mango_cluster_id_t cluster, mango_unit_id_t unit,
                             mango_size_t size, mango_mem_id_t *memory, mango_addr_t *phy_addr){
    uint32_t phy_addr_l = 0; //HNemu
    int status = HNemu::instance()->find_memory(cluster, unit, size, memory, &phy_addr_l);
    if (status!=HN_SUCCEEDED){
        log_hhal.Error("[GNLibHHAL] memory not found: cluster=%d, unit=%d, size=%d",
                       cluster, unit, size);
        return ExitCode::ERR_OUT_OF_MEMORY;
    } else {
        *phy_addr = phy_addr_l;
        log_hhal.Debug("[GNLibHHAL] free memory found: cluster=%d, unit=%d, size=%d, "
                       "memory-%d, phy_addr=0x%x",
                       cluster, unit, size, *memory, *phy_addr);
        return ExitCode::SUCCESS;
    }
}

mango_exit_code_t GNLibHHAL::allocate_memory(mango_cluster_id_t cluster, mango_mem_id_t memory,
                                 mango_addr_t phy_addr, mango_size_t size){
    int status = HNemu::instance()->allocate_memory(cluster, memory, phy_addr, size);
    if (status!=HN_SUCCEEDED){
        log_hhal.Error("[GNLibHHAL] memory allocation failed: cluster=%d, memory=%d, phy_addr=0x%x, size=%d",
                       cluster, memory, phy_addr, size);
        return ExitCode::ERR_OUT_OF_MEMORY;
    } else {
        log_hhal.Debug("[GNLibHHAL] memory allocated: cluster=%d, memory=%d, phy_addr=0x%x, size=%d",
                       cluster, memory, phy_addr, size);
        return ExitCode::SUCCESS;
    }

}
mango_exit_code_t GNLibHHAL::release_memory(mango_cluster_id_t cluster, mango_mem_id_t memory,
                                mango_addr_t phy_addr, mango_size_t size){
    int status = HNemu::instance()->release_memory(cluster, memory, phy_addr, size);
    if (status!=HN_SUCCEEDED){
        log_hhal.Error("[GNLibHHAL] memory free failed: cluster=%d, memory=%d, phy_addr=0x%x, size=%d",
                       cluster, memory, phy_addr, size);
        return ExitCode::ERR_OTHER;
    } else {
        log_hhal.Debug("[GNLibHHAL] memory released: cluster=%d, memory=%d, phy_addr=0x%x, size=%d",
                       cluster, memory, phy_addr, size);
        return ExitCode::SUCCESS;
    }
}


inline uint32_t get_hn_tile_family(mango_unit_type_t unit_type) {
    switch (unit_type) {
        case mango_unit_type_t::PEAK:
            return HN_TILE_FAMILY_PEAK;
        case mango_unit_type_t::NUP:
            return HN_TILE_FAMILY_NUPLUS;
        case mango_unit_type_t::DCT:
             return HN_TILE_FAMILY_DCT;
        case mango_unit_type_t::GN:
            return HN_TILE_FAMILY_GN;
        default:
            return HN_TILE_MODEL_NONE;
        }
    }

mango_exit_code_t GNLibHHAL::find_units_set(mango_cluster_id_t cluster, std::vector<mango_unit_type_t> &types, std::vector<mango_unit_id_t> &tiles_dst){
    auto num_tiles = types.size();
    uint32_t* types_hn = new uint32_t[num_tiles];
    for (unsigned int i = 0; i< num_tiles; i++){
        types_hn[i] = get_hn_tile_family(types[i]);
        log_hhal.Debug("[GNLibHHAL] find_units_set: type %d converted to %d ",
                       types[i], types_hn[i] );
    }
    uint32_t *tiles = nullptr;
    auto status =  HNemu::instance()->find_units_set(cluster, num_tiles, types_hn, &tiles);

    if (status != HN_SUCCEEDED ) {
        delete [] types_hn;
        return ExitCode::ERR_OTHER;
    }
    
    for (uint32_t i = 0; i< num_tiles; i++){
        tiles_dst[i] = tiles[i];
    }
    free (tiles);
    delete [] types_hn;
    return ExitCode::SUCCESS;
}

mango_exit_code_t GNLibHHAL::reserve_units_set(mango_cluster_id_t cluster, const std::vector<mango_unit_id_t> &tiles){
    auto status = HNemu::instance()->reserve_units_set(cluster, tiles.size(), tiles.data());
    if (status!=HN_SUCCEEDED)
        return ExitCode::ERR_OTHER;
    else
        return ExitCode::SUCCESS;

}

mango_exit_code_t GNLibHHAL::release_units_set(mango_cluster_id_t cluster, const std::vector<mango_unit_id_t> &tiles){
    auto status = HNemu::instance()->release_units_set(cluster, tiles.size(), tiles.data());
    if (status!=HN_SUCCEEDED)
        return ExitCode::ERR_OTHER;
    else
        return ExitCode::SUCCESS;
}

} // namespace mango
