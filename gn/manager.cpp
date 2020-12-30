#include "gn/manager.h"
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "gn/mm.h"

#define MANGO_ROOT "/opt/mango"

#define MANGO_SEMAPHORE       "mango_sem"
#define MANGO_DEVICE_MEMORY   "/tmp/device_memory.dat"
#define MANGO_REG_SIZE (128)
#define MANGO_REG_UNUSED 0
#define MANGO_REG_INUSE 1

#define NUM_VNS HN_MAX_VNS

#define GN_RETURN_DIR  "/bin/gn_return";

#define UNUSED(x) ((void)x)

namespace hhal {

mango_addr_t *GNManager::mem;
sem_t *GNManager::sem_id;
int GNManager::f_mem;
std::map<mango_cluster_id_t, std::vector<mango_addr_t>> GNManager::event_register_off;

GNManagerExitCode GNManager::initialize() {

    assert(initialized == false);

    uint32_t num_clusters;
    HNemu::instance()->get_num_clusters(&num_clusters);
    printf("Num clusters: %d\n", num_clusters);

    init_semaphore();
    if (sem_id == NULL) {
        return GNManagerExitCode::ERROR;
    }
    printf("Sem_id: %p\n", sem_id);

    mode_t old_mask = umask(0);
    f_mem = open(MANGO_DEVICE_MEMORY,O_RDWR | O_CREAT, 0666);
    if (f_mem==-1) {
        log_hhal.Error("Unable to read device memory " MANGO_DEVICE_MEMORY);
        return GNManagerExitCode::ERROR;
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
        return GNManagerExitCode::ERROR;
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
    for (mango_cluster_id_t cluster_id = 0; cluster_id < num_clusters; cluster_id++) {
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
        return GNManagerExitCode::ERROR;
    } else {
        log_hhal.Debug("[GNLibHHAL] memory for sync registers allocated: cluster=%d, memory=%d, phy_addr=0x%x, size=%d",
                    default_cluster_id, memory, phy_addr, size);
    }

    initialized = true;
    return GNManagerExitCode::OK;
}

GNManagerExitCode GNManager::finalize() {
    assert(initialized == true);

    sem_close(sem_id);
    close(f_mem);

    initialized = false;
    return GNManagerExitCode::OK;
}

GNManagerExitCode GNManager::assign_kernel(gn_kernel *info) {
    kernel_info[info->id] = *info;
    tlbs[info->id] = TLB();
    return GNManagerExitCode::OK;
}

GNManagerExitCode GNManager::assign_buffer(gn_buffer *info) {
    buffer_info[info->id] = *info;
    return GNManagerExitCode::OK;
}

GNManagerExitCode GNManager::assign_event(gn_event *info) {
    event_info[info->id] = *info;
    return GNManagerExitCode::OK;
}

GNManagerExitCode GNManager::kernel_write(int kernel_id, std::string image_path) {
    assert(initialized == true);
    assert(image_path.size() > 0);
    gn_kernel &info = kernel_info[kernel_id];

    log_hhal.Debug("[GNLibHHAL] kernel_write: cluster=%d,  image_path=%s, memory=%d, address=0x%x",
            info.cluster_id, image_path.c_str(), info.mem_tile, info.physical_addr);
    return GNManagerExitCode::OK;
}
GNManagerExitCode GNManager::kernel_start_string_args(int kernel_id, std::string arguments) {
    assert(initialized == true);
    assert(arguments.size() > 0);
    gn_kernel &info = kernel_info[kernel_id];

    pid_t pid;
    pid = fork();
    if (!pid){ /* child, executor */
        if(arguments[0] != '/') {
            arguments.insert(0, "./");
        }
        printf("system(%s);\n", arguments.c_str());
        auto ret = system(arguments.c_str());
        UNUSED(ret);
        char n[11];
        std::string gn_return(MANGO_ROOT);  // MANGO_ROOT compile-time defined
        gn_return += GN_RETURN_DIR;
        execl(gn_return.c_str(), n, NULL);
        printf("***\n!!!\nEXECL ERROR\n!!!\n***\n");
        exit(0);
    }
    log_hhal.Debug("[GNLibHHAL] kernel_start: cluster=%d,  unit=%d, address=0x%x, argument_string=%s",
            info.cluster_id, info.unit_id, info.physical_addr, arguments.c_str());
    return GNManagerExitCode::OK;
}

GNManagerExitCode GNManager::write_to_memory(int buffer_id, const void *source, size_t size) {
    assert(initialized == true);
    assert(source != NULL);
    assert(size > 0);
    gn_buffer &info = buffer_info[buffer_id];

    memcpy(mem + (info.physical_addr/sizeof(mango_addr_t)), static_cast<char*>(const_cast<void*>(source)), size);
    log_hhal.Debug("[GNLibHHAL] write_to_memory: cluster=%d,  memory=%d, dest_address=0x%x, size=%d",
                   info.cluster_id, info.mem_tile, info.physical_addr, size);
    return GNManagerExitCode::OK;
}

GNManagerExitCode GNManager::read_from_memory(int buffer_id, void *dest, size_t size) {
    assert(initialized == true);
    assert(dest != NULL);
    assert(size > 0);
    gn_buffer &info = buffer_info[buffer_id];

    memcpy(static_cast<char*>(dest), mem + (info.physical_addr/sizeof(mango_addr_t)), size);

    log_hhal.Debug("[GNLibHHAL] read_from_memory: cluster=%d,  memory=%d, source_address=0x%x, size=%d",
                   info.cluster_id, info.mem_tile, info.physical_addr, size);
    return GNManagerExitCode::OK;
}

GNManagerExitCode GNManager::write_sync_register(int event_id, uint8_t data) {
    assert(initialized == true);
    gn_event &info = event_info[event_id];
    int reg_address = info.physical_addr;
    reg_address -= info.cluster_id * MANGO_REG_SIZE * 4;
    reg_address /= sizeof(mango_addr_t);

    sem_wait(sem_id);

    if (reg_address % 8 != 0) {
        mem[reg_address] += data;
    } else {
        mem[reg_address] = data;
    }

    sem_post(sem_id);
    log_hhal.Debug("[GNLibHHAL] write_sync_register: cluster=%d,  reg_address=0x%x, data=%d",
                   info.cluster_id, reg_address, data);
    return GNManagerExitCode::OK;
}

GNManagerExitCode GNManager::read_sync_register(int event_id, uint8_t *data) {
    assert(initialized == true);
    gn_event &info = event_info[event_id];
    int reg_address = info.physical_addr;
    reg_address -= info.cluster_id * MANGO_REG_SIZE * 4;
    reg_address /= sizeof(mango_addr_t);

    if (sem_wait(sem_id) == -1) {
        printf("sem wait failed!\n");
    }
    else {
        printf("sem wait success\n");
    }

    uint8_t result = mem[reg_address];
    mem[reg_address]=0;

    sem_post(sem_id);

    log_hhal.Debug("[GNLibHHAL] read_sync_register: cluster=%d, reg_address=0x%x, data=%d",
                   info.cluster_id, reg_address, result);

    *data = result;
    return GNManagerExitCode::OK;
}

GNManagerExitCode GNManager::allocate_memory(int buffer_id){
    gn_buffer &info = buffer_info[buffer_id];
    int status = HNemu::instance()->allocate_memory(info.cluster_id, info.mem_tile, info.physical_addr, info.size);
    if (status!=HN_SUCCEEDED){
        log_hhal.Error("[GNLibHHAL] memory allocation failed: cluster=%d, memory=%d, phy_addr=0x%x, size=%d",
                       info.cluster_id, info.mem_tile, info.physical_addr, info.size);
        return GNManagerExitCode::ERROR;
    } else {
        log_hhal.Debug("[GNLibHHAL] memory allocated: cluster=%d, memory=%d, phy_addr=0x%x, size=%d",
                       info.cluster_id, info.mem_tile, info.physical_addr, info.size);
        return GNManagerExitCode::OK;
    }

}

GNManagerExitCode GNManager::release_memory(int buffer_id){
    gn_buffer &info = buffer_info[buffer_id];
    int status = HNemu::instance()->release_memory(info.cluster_id, info.mem_tile, info.physical_addr, info.size);
    if (status!=HN_SUCCEEDED){
        log_hhal.Error("[GNLibHHAL] memory free failed: cluster=%d, memory=%d, phy_addr=0x%x, size=%d",
                       info.cluster_id, info.mem_tile, info.physical_addr, info.size);
        return GNManagerExitCode::ERROR;
    } else {
        log_hhal.Debug("[GNLibHHAL] memory released: cluster=%d, memory=%d, phy_addr=0x%x, size=%d",
                       info.cluster_id, info.mem_tile, info.physical_addr, info.size);
        return GNManagerExitCode::OK;
    }
}

void GNManager::init_semaphore(void) {

    mode_t old_mask = umask(0);

    GNManager::sem_id = sem_open(MANGO_SEMAPHORE, O_CREAT, 0666, 1);
    if (sem_id == SEM_FAILED) {
        printf("GNLibHHAL: Unable to init semaphore.\n");
        throw std::runtime_error("GNLibHHAL: Unable to init semaphore.");
    }
    umask(old_mask);

}

GNManagerExitCode GNManager::get_synch_register_addr(mango_cluster_id_t cluster, mango_addr_t *reg_address, bool isINCRWRITE_REG) {

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
        return GNManagerExitCode::OK;
    } else {
        return GNManagerExitCode::ERROR;
    }
}

void GNManager::release_synch_register_addr(mango_cluster_id_t cluster, mango_addr_t reg_address) {
    reg_address -= cluster * MANGO_REG_SIZE * 4;
    reg_address /= sizeof(mango_addr_t);
    event_register_off[cluster][reg_address] = MANGO_REG_UNUSED;
}

mango_size_t GNManager::get_memory_size(mango_cluster_id_t cluster, mango_mem_id_t memory) const {
    assert(initialized == true);

    mango_size_t mem_size;
    int err = HNemu::instance()->get_memory_size (cluster, memory, &mem_size);

    if (err != 0) {
//        log_hhal.Error("[GNLibHHAL] Cannot get memory size: cluster=%d, memory=%d", cluster, memory);
    }

    return mem_size;
}

mango_addr_t GNManager::get_first_virtual_address(mango_unit_type_t unit_type, hhal_tlb_entry_type_t entry_type) const {
    if(entry_type == hhal_tlb_entry_type_t::NORMAL_DATA)
        return num_clusters * MANGO_REG_SIZE * sizeof(mango_addr_t);
    if(entry_type == hhal_tlb_entry_type_t::EXECUTABLE_CODE)
        return 0x0;
    if(entry_type == hhal_tlb_entry_type_t::SYNCHRONIZATION_REGS)
        return 0x0;
    assert(false);
}


GNManagerExitCode GNManager::find_memory(mango_cluster_id_t cluster, mango_unit_id_t unit,
                             mango_size_t size, mango_mem_id_t *memory, mango_addr_t *phy_addr){
    uint32_t phy_addr_l = 0; //HNemu
    int status = HNemu::instance()->find_memory(cluster, unit, size, memory, &phy_addr_l);
    if (status!=HN_SUCCEEDED){
        log_hhal.Error("[GNLibHHAL] memory not found: cluster=%d, unit=%d, size=%d",
                       cluster, unit, size);
        return GNManagerExitCode::ERROR;
    } else {
        *phy_addr = phy_addr_l;
        log_hhal.Debug("[GNLibHHAL] free memory found: cluster=%d, unit=%d, size=%d, "
                       "memory-%d, phy_addr=0x%x",
                       cluster, unit, size, *memory, *phy_addr);
        return GNManagerExitCode::OK;
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

GNManagerExitCode GNManager::find_units_set(mango_cluster_id_t cluster, std::vector<mango_unit_type_t> &types, std::vector<mango_unit_id_t> &tiles_dst){
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
        return GNManagerExitCode::ERROR;
    }
    
    for (uint32_t i = 0; i< num_tiles; i++){
        tiles_dst[i] = tiles[i];
    }
    free (tiles);
    delete [] types_hn;
    return GNManagerExitCode::OK;
}

GNManagerExitCode GNManager::reserve_units_set(mango_cluster_id_t cluster, const std::vector<mango_unit_id_t> &tiles){
    auto status = HNemu::instance()->reserve_units_set(cluster, tiles.size(), tiles.data());
    if (status!=HN_SUCCEEDED)
        return GNManagerExitCode::ERROR;
    else
        return GNManagerExitCode::OK;

}

GNManagerExitCode GNManager::release_units_set(mango_cluster_id_t cluster, const std::vector<mango_unit_id_t> &tiles){
    auto status = HNemu::instance()->release_units_set(cluster, tiles.size(), tiles.data());
    if (status!=HN_SUCCEEDED)
        return GNManagerExitCode::ERROR;
    else
        return GNManagerExitCode::OK;
}

GNManagerExitCode GNManager::do_memory_management() {
    MM mm;
    std::vector<gn_buffer>  buffers;
    std::vector<gn_event>   events;
    std::vector<gn_kernel>  kernels;

    for(auto &b_pair: buffer_info) {
        buffers.push_back(b_pair.second);
    }
    for(auto &e_pair: event_info) {
        events.push_back(e_pair.second);
    }
    for(auto &k_pair: kernel_info) {
        kernels.push_back(k_pair.second);
    }

    mm.set_vaddr_kernels(*this, kernels);
	mm.set_vaddr_buffers(*this, buffers);
	mm.set_vaddr_events(*this, events);
}

}
