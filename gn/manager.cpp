#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sstream>

#include "gn/manager.h"
#include "gn/hnemu/hnemu.h"
#include "gn/hnemu/hn_include/hn_errcode.h"

#define MANGO_ROOT "/opt/mango"

#define MANGO_SEMAPHORE       "mango_sem"
#define MANGO_DEVICE_MEMORY   "/tmp/device_memory.dat"
#define MANGO_REG_SIZE (128)
#define MANGO_REG_UNUSED 0
#define MANGO_REG_INUSE 1

#define GN_DEFAULT_CLUSTER 0

// Size used in gn hhal
// #define ADDR_SIZE sizeof(addr_t) 

// Size used in libgn
#define ADDR_SIZE 4L

#define NUM_VNS HN_MAX_VNS

#define UNUSED(x) ((void)x)

namespace hhal {

ConsoleLogger log_hhal;

addr_t *GNManager::mem;
sem_t *GNManager::sem_id;
int GNManager::f_mem;
std::map<uint32_t, std::vector<addr_t>> GNManager::event_register_off;

GNManagerExitCode GNManager::initialize() {

    assert(initialized == false);

    uint32_t num_clusters;
    HNemu::instance()->get_num_clusters(&num_clusters);
    log_hhal.Info("GNManager: Num clusters: %d", num_clusters);

    init_semaphore();
    if (sem_id == NULL || sem_id == SEM_FAILED) {
        return GNManagerExitCode::ERROR;
    }

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

    mem = (addr_t*) mmap (NULL, mem_size,
                            PROT_READ|PROT_WRITE, MAP_SHARED, f_mem,0);

    if (mem==MAP_FAILED) {
        log_hhal.Error("Memory not present in hn emulation configuration ");
        return GNManagerExitCode::ERROR;
    }
    for (uint32_t cluster_id = 0; cluster_id< num_clusters; cluster_id++){
        hhal_tile_description_t cluster_info;
        auto info = HNemu::instance()->get_info(cluster_id);
        cluster_info.total_tiles = info->num_tiles;
        cluster_info.tiles_x = info->num_cols;
        cluster_info.tiles_y = info->num_rows;
        tiles.insert(std::make_pair(cluster_id, cluster_info));
    }
    this->num_clusters = num_clusters;

    // sync registers offset
    for (uint32_t cluster_id = 0; cluster_id < num_clusters; cluster_id++) {
        std::vector <addr_t> regs(MANGO_REG_SIZE, MANGO_REG_UNUSED);
        event_register_off.emplace(std::make_pair(cluster_id, regs));
    }

    // Reserve space for sync registers
    // registers for all clusters are reserved on cluster 0 close to tile 0
    uint32_t default_cluster_id = 0;
    uint32_t size = num_clusters * MANGO_REG_SIZE * ADDR_SIZE;
    uint32_t memory = 0;
    addr_t phy_addr = 0;
    uint32_t phy_addr_l = 0;
    int status;
    status = HNemu::instance()->find_memory(default_cluster_id, 0, size, &memory, &phy_addr_l);
    if (status!=HN_SUCCEEDED){
        log_hhal.Error("GNManager: cannot find memory for sync registers: cluster=%d, size=%d",
                    default_cluster_id, size);
        return GNManagerExitCode::ERROR;
    }
    phy_addr=phy_addr_l;
    status = HNemu::instance()->allocate_memory(default_cluster_id, memory, phy_addr, size);
    
    if (status!=HN_SUCCEEDED){
        log_hhal.Error("GNManager: memory allocation for sync registers failed: cluster=%d, memory=%d, phy_addr=0x%x, size=%d",
                    default_cluster_id, memory, phy_addr, size);
        return GNManagerExitCode::ERROR;
    } else {
        log_hhal.Debug("GNManager: memory for sync registers allocated: cluster=%d, memory=%d, phy_addr=0x%x, size=%d",
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
    log_hhal.Debug("GNManager: Assigning kernel %d", info->id);
    kernel_info[info->id] = *info;
    return GNManagerExitCode::OK;
}

GNManagerExitCode GNManager::assign_buffer(gn_buffer *info) {
    log_hhal.Debug("GNManager: Assigning buffer %d, size=%zu", info->id, info->size);
    buffer_info[info->id] = *info;

    return GNManagerExitCode::OK;
}

GNManagerExitCode GNManager::assign_event(gn_event *info) {
    log_hhal.Debug("GNManager: Assigning event %d", info->id);
    event_info[info->id] = *info;

    return GNManagerExitCode::OK;
}

GNManagerExitCode GNManager::kernel_write(int kernel_id, std::string image_path) {
    assert(initialized == true);
    assert(image_path.size() > 0);
    gn_kernel &info = kernel_info[kernel_id];
    kernel_images[info.id] = image_path;

    log_hhal.Debug("GNManager: kernel_write: cluster=%d,  image_path=%s",
            allocated_kernel_info[kernel_id].cluster_id, image_path.c_str());
    return GNManagerExitCode::OK;
}

GNManagerExitCode GNManager::kernel_start(int kernel_id, const Arguments &arguments) {
    gn_kernel &info = kernel_info[kernel_id];

    Arguments full_args;
    auto event = info.termination_event;

    full_args.add_event({event});

    // Adding termination event 3 extra times.
    // GN requires 3 task events for multithreading (currently unsupported)
    for(int i=0;i<3;++i) {
        full_args.add_event({event});
    }

    full_args.add_arguments(arguments);

    std::string str_args;
    GNManagerExitCode ec;
    ec = get_string_arguments(kernel_id, full_args, str_args);
    if (ec != GNManagerExitCode::OK) {
        return GNManagerExitCode::ERROR;
    }
    log_hhal.Info("GNManager: Kernel argument string:\n%s", str_args.c_str());
    ec = kernel_start_string_args(kernel_id, str_args);
    if (ec != GNManagerExitCode::OK) {
        return GNManagerExitCode::ERROR;
    }
    return GNManagerExitCode::OK;
}

GNManagerExitCode GNManager::kernel_start_string_args(int kernel_id, std::string arguments) {
    assert(initialized == true);
    assert(arguments.size() > 0);
    auto &info = allocated_kernel_info[kernel_id];

    pid_t pid;
    pid = fork();
    if (!pid){ /* child, executor */
        if(arguments[0] != '/') {
            arguments.insert(0, "./");
        }
        printf("system(%s);\n", arguments.c_str());
        auto ret = system(arguments.c_str());
        UNUSED(ret);
        exit(0);
    }
    log_hhal.Debug("GNManager: kernel_start: cluster=%d,  unit=%d, argument_string=%s",
            info.cluster_id, info.unit_id, arguments.c_str());
    return GNManagerExitCode::OK;
}

GNManagerExitCode GNManager::write_to_memory(int buffer_id, const void *source, size_t size) {
    assert(initialized == true);
    assert(source != NULL);
    assert(size > 0);
    auto &info = allocated_buffer_info[buffer_id];

    size_t offset = info.physical_addr / ADDR_SIZE;

    memcpy(mem + offset, static_cast<char*>(const_cast<void*>(source)), size);
    log_hhal.Debug("GNManager: write_to_memory: cluster=%d,  memory=%d, dest_address=0x%x, size=%d",
                   info.cluster_id, info.mem_tile, info.physical_addr, size);
    log_hhal.Debug("GNManager: write_to_memory: real addr=0x%x", offset);
    return GNManagerExitCode::OK;
}

GNManagerExitCode GNManager::read_from_memory(int buffer_id, void *dest, size_t size) {
    assert(initialized == true);
    assert(dest != NULL);
    assert(size > 0);
    auto &info = allocated_buffer_info[buffer_id];

    size_t offset = info.physical_addr / ADDR_SIZE;

    memcpy(static_cast<char*>(dest), mem + offset, size);
    log_hhal.Debug("GNManager: read_from_memory: cluster=%d,  memory=%d, source_address=0x%x, size=%d",
                   info.cluster_id, info.mem_tile, info.physical_addr, size);
    log_hhal.Debug("GNManager: write_to_memory: real addr=0x%x", offset);
    return GNManagerExitCode::OK;
}

GNManagerExitCode GNManager::write_sync_register(int event_id, uint32_t data) {
    assert(initialized == true);
    auto &info = allocated_event_info[event_id];
    int reg_address = info.physical_addr;
    reg_address -= info.cluster_id * MANGO_REG_SIZE * 4;
    reg_address /= ADDR_SIZE;

    sem_wait(sem_id);

    if (reg_address % 8 != 0) {
        mem[reg_address] += data;
    } else {
        mem[reg_address] = data;
    }

    sem_post(sem_id);
    log_hhal.Trace("GNManager: write_sync_register: cluster=%d, phy_addr=%p, reg_address=0x%x, data=%d",
                   info.cluster_id, info.physical_addr, reg_address, data);
    return GNManagerExitCode::OK;
}

GNManagerExitCode GNManager::read_sync_register(int event_id, uint32_t *data) {
    assert(initialized == true);
    auto &info = allocated_event_info[event_id];
    int reg_address = info.physical_addr;
    reg_address -= info.cluster_id * MANGO_REG_SIZE * 4;
    reg_address /= ADDR_SIZE;

    sem_wait(sem_id);

    uint32_t result = mem[reg_address];
    mem[reg_address] = 0;

    sem_post(sem_id);

    log_hhal.Trace("GNManager: read_sync_register: cluster=%d, phy_addr=%p, reg_address=0x%x, data=%d",
                   info.cluster_id, info.physical_addr, reg_address, result);

    *data = result;
    return GNManagerExitCode::OK;
}

GNManagerExitCode GNManager::allocate_kernel(int kernel_id){
    std::vector<uint32_t> tiles_dst(1);
    auto status = find_units_set(GN_DEFAULT_CLUSTER, 1, tiles_dst);
    if (status != GNManagerExitCode::OK){
        log_hhal.Error("GNManager: allocate_kernel: tile mapping not found.");
        return GNManagerExitCode::ERROR;
    }

    log_hhal.Info("GNManager: allocate_kernel: resource_allocation: tile found");

    status = reserve_units_set(GN_DEFAULT_CLUSTER, tiles_dst);
    if (status != GNManagerExitCode::OK) {
        log_hhal.Error("GNManager: allocate_kernel: tile reservation failed");
        return GNManagerExitCode::ERROR;
    }

    allocated_kernel_info[kernel_id] = {GN_DEFAULT_CLUSTER, tiles_dst[0]};
}

GNManagerExitCode GNManager::release_kernel(int kernel_id){
    std::vector<uint32_t> tiles_dst = { allocated_kernel_info[kernel_id].unit_id };

    auto status = release_units_set(GN_DEFAULT_CLUSTER, tiles_dst);
    if (status != GNManagerExitCode::OK){
        log_hhal.Error("GNManager: release_kernel: tile release failed");
        return GNManagerExitCode::ERROR;
    }
    allocated_kernel_info.erase(kernel_id);
    return GNManagerExitCode::OK;
}

GNManagerExitCode GNManager::allocate_memory(int buffer_id){
    gn_buffer &info = buffer_info[buffer_id];

    allocated_buffer alloc_info;
    alloc_info.cluster_id = GN_DEFAULT_CLUSTER;
    
    uint32_t mem_tile;
    addr_t phy_addr;
    uint32_t default_unit;

    if (info.kernels_in.size() != 0) {
        default_unit = allocated_kernel_info[info.kernels_in.back()].unit_id;
    } else {
        default_unit = allocated_kernel_info[info.kernels_out.back()].unit_id;
    }

    log_hhal.Debug("GNManager: allocate_memory: Finding memory for cluster=%d, unit=%d, size=%zu", alloc_info.cluster_id, default_unit, info.size);
    auto status = find_memory(alloc_info.cluster_id, default_unit, info.size, &mem_tile, &phy_addr);
    if (status != GNManagerExitCode::OK){
        log_hhal.Error("GNManager: allocate_memory: cannot find memory for buffer %d", info.id);
        return GNManagerExitCode::ERROR;
    }
    log_hhal.Debug("GNManager: allocate_memory: buffer=%d, memory=%d, phy_addr=0x%x", info.id, mem_tile, phy_addr);
    alloc_info.mem_tile = mem_tile;
    alloc_info.physical_addr = phy_addr;
    allocated_buffer_info[info.id] = alloc_info;

    int hn_status = HNemu::instance()->allocate_memory(alloc_info.cluster_id, alloc_info.mem_tile, alloc_info.physical_addr, info.size);
    
    if (hn_status != HN_SUCCEEDED){
        log_hhal.Error("GNManager: memory allocation failed: cluster=%d, memory=%d, phy_addr=0x%x, size=%d",
                       alloc_info.cluster_id, alloc_info.mem_tile, alloc_info.physical_addr, info.size);
        return GNManagerExitCode::ERROR;
    } else {
        log_hhal.Debug("GNManager: memory allocated: cluster=%d, memory=%d, phy_addr=0x%x, size=%d",
                       alloc_info.cluster_id, alloc_info.mem_tile, alloc_info.physical_addr, info.size);

        // Prepare event register
        GNManagerExitCode ec;
        uint32_t value;
        auto et_id = info.event;
        auto &ev_info = allocated_event_info[et_id];
        log_hhal.Debug("GNManager: allocate_memory: setting buffer event to WRITE, event %d, phy_addr 0x%x", et_id, ev_info.physical_addr);
        ec = read_sync_register(et_id, &value); 
        if (ec != GNManagerExitCode::OK) return ec;
        assert( 0 == value );

        const int WRITE = 2;
        ec = write_sync_register(et_id, WRITE);
        if (ec != GNManagerExitCode::OK) return ec;
    
        return GNManagerExitCode::OK;
    }

}

GNManagerExitCode GNManager::release_memory(int buffer_id){
    auto &info = allocated_buffer_info[buffer_id];
    size_t buf_size = buffer_info[buffer_id].size;
    int status = HNemu::instance()->release_memory(info.cluster_id, info.mem_tile, info.physical_addr, buf_size);
    if (status != HN_SUCCEEDED){
        log_hhal.Error("GNManager: memory free failed: cluster=%d, memory=%d, phy_addr=0x%x, size=%d",
                       info.cluster_id, info.mem_tile, info.physical_addr, buf_size);
        return GNManagerExitCode::ERROR;
    } else {
        log_hhal.Debug("GNManager: memory released: cluster=%d, memory=%d, phy_addr=0x%x, size=%d",
                       info.cluster_id, info.mem_tile, info.physical_addr, buf_size);
        return GNManagerExitCode::OK;
    }
}

GNManagerExitCode GNManager::allocate_event(int event_id){
    addr_t phy_addr;
    auto status = get_synch_register_addr(GN_DEFAULT_CLUSTER, &phy_addr, 1);
    if (status != GNManagerExitCode::OK) {
        log_hhal.Debug("GNManager: allocate_event: event %d allocation failed", event_id);
        return GNManagerExitCode::ERROR;
    }
    allocated_event_info[event_id] = {GN_DEFAULT_CLUSTER, phy_addr};

    log_hhal.Debug("GNManager: allocate_event: event=%d, phy_addr=0x%x", event_id, allocated_event_info[event_id].physical_addr);

    // Prepare event register
    GNManagerExitCode ec;
    uint32_t value;
    ec = read_sync_register(event_id, &value);
    if (ec != GNManagerExitCode::OK) return ec;
    // - We re-read the value and now must be zero
    ec = read_sync_register(event_id, &value);
    if (ec != GNManagerExitCode::OK) return ec;
    assert( 0 == value );

    return GNManagerExitCode::OK;
}

GNManagerExitCode GNManager::release_event(int event_id){
    release_synch_register_addr(GN_DEFAULT_CLUSTER, allocated_event_info[event_id].physical_addr);
    log_hhal.Debug("GNManager: release_event: event=%d released", event_id);
    allocated_event_info.erase(event_id);
    return GNManagerExitCode::OK;
}

void GNManager::init_semaphore(void) {

    mode_t old_mask = umask(0);

    GNManager::sem_id = sem_open(MANGO_SEMAPHORE, O_CREAT, 0666, 1);
    if (sem_id == SEM_FAILED) {
        printf("GNLibHHAL: Unable to init semaphore.\n");
    }
    umask(old_mask);

}

GNManagerExitCode GNManager::get_synch_register_addr(uint32_t cluster, addr_t *reg_address, bool isINCRWRITE_REG) {

    addr_t reg = MANGO_REG_SIZE+1;
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

void GNManager::release_synch_register_addr(uint32_t cluster, addr_t reg_address) {
    reg_address -= cluster * MANGO_REG_SIZE * 4;
    reg_address /= ADDR_SIZE;
    event_register_off[cluster][reg_address] = MANGO_REG_UNUSED;
}

uint32_t GNManager::get_memory_size(uint32_t cluster, uint32_t memory) const {
    assert(initialized == true);

    uint32_t mem_size;
    int err = HNemu::instance()->get_memory_size(cluster, memory, &mem_size);

    if (err != HN_SUCCEEDED) {
        log_hhal.Error("GNManager: Cannot get memory size: cluster=%d, memory=%d", cluster, memory);
    }

    return mem_size;
}

GNManagerExitCode GNManager::find_memory(uint32_t cluster, uint32_t unit,
                             uint32_t size, uint32_t *memory, addr_t *phy_addr){
    log_hhal.Debug("GNManager: find_memory: cluster=%d, unit=%d, size=%zu", cluster, unit, size);
    uint32_t phy_addr_l = 0; //HNemu
    int status = HNemu::instance()->find_memory(cluster, unit, size, memory, &phy_addr_l);
    if (status!=HN_SUCCEEDED){
        log_hhal.Error("GNManager: memory not found: cluster=%d, unit=%d, size=%d",
                       cluster, unit, size);
        return GNManagerExitCode::ERROR;
    } else {
        *phy_addr = phy_addr_l;
        log_hhal.Debug("GNManager: free memory found: cluster=%d, unit=%d, size=%d, "
                       "memory=%d, phy_addr=0x%x",
                       cluster, unit, size, *memory, *phy_addr);
        return GNManagerExitCode::OK;
    }
}

GNManagerExitCode GNManager::find_units_set(uint32_t cluster, uint32_t num_tiles, std::vector<uint32_t> &tiles_dst){
    uint32_t* types_hn = new uint32_t[num_tiles];
    for (unsigned int i = 0; i< num_tiles; i++){
        types_hn[i] = HN_TILE_FAMILY_GN;
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

GNManagerExitCode GNManager::reserve_units_set(uint32_t cluster, const std::vector<uint32_t> &tiles){
    auto status = HNemu::instance()->reserve_units_set(cluster, tiles.size(), tiles.data());
    if (status!=HN_SUCCEEDED)
        return GNManagerExitCode::ERROR;
    else
        return GNManagerExitCode::OK;

}

GNManagerExitCode GNManager::release_units_set(uint32_t cluster, const std::vector<uint32_t> &tiles){
    auto status = HNemu::instance()->release_units_set(cluster, tiles.size(), tiles.data());
    if (status!=HN_SUCCEEDED)
        return GNManagerExitCode::ERROR;
    else
        return GNManagerExitCode::OK;
}

GNManagerExitCode GNManager::get_string_arguments(int kernel_id, Arguments &args, std::string &str_args) {
	std::stringstream ss;

	//get full memory size
    unsigned long long mem_size = 0;

    for (uint32_t cluster_id = 0; cluster_id < num_clusters; cluster_id++){
        hhal_tile_description_t htd = tiles.at(cluster_id);
        for (int i = 0; i < htd.total_tiles; i++) {
            mem_size += get_memory_size(cluster_id, i);
        }
    }

    gn_kernel &info = kernel_info[kernel_id];

    if (kernel_images.find(info.id) == kernel_images.end()) {
        log_hhal.Error("GNManager: No kernel path");
        return GNManagerExitCode::ERROR;
    }

    ss << kernel_images[info.id];
    ss << " 0x" << std::hex << mem_size;

	for (const auto &arg : args.get_args()) {
        switch (arg.type) {
            case ArgumentType::BUFFER:
                ss << " 0x" << std::hex << allocated_buffer_info[arg.buffer.id].physical_addr;
                break;
            case ArgumentType::EVENT:
                ss << " 0x" << std::hex << allocated_event_info[arg.event.id].physical_addr;
                break;
            case ArgumentType::SCALAR:
                if (arg.scalar.type == ScalarType::INT) {
                    switch (arg.scalar.size)
                    {
                    case sizeof(int8_t):
                        ss << " " << arg.scalar.aint8;
                        break;
                    case sizeof(int16_t):
                        ss << " " << arg.scalar.aint16;
                        break;
                    case sizeof(int32_t):
                        ss << " " << arg.scalar.aint32;
                        break;
                    case sizeof(int64_t):
                        ss << " " << arg.scalar.aint64;
                        break;
                    default:
                        log_hhal.Error("GNManager: Unknown scalar int size");
                        return GNManagerExitCode::ERROR;
                    }
                }
                else {
                    log_hhal.Error("GNManager: Float scalars not supported");
                    return GNManagerExitCode::ERROR;
                }
                break;
            default:
                log_hhal.Error("GNManager: Unknown argument");
                return GNManagerExitCode::ERROR;
        }
	}
    str_args = ss.str();

	return GNManagerExitCode::OK;
}

}
