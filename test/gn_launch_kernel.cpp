#include <vector>
#include <map>
#include <stdio.h>

#include "hhal.h"
#include "event_utils.h"

#include <stdlib.h>

#define UNUSED(x) ((void)x)

struct kernel {
    int id;
    std::string image_path;
    size_t image_size;
};

struct buffer {
    int id;
    size_t size;
    std::vector<int> kernels_in;
    std::vector<int> kernels_out;
};

struct event {
    int id;
    std::vector<int> kernels_in;
    std::vector<int> kernels_out;
};

enum class ArgType {
    BUFFER,
    EVENT,
    SCALAR
};

struct Arg {
    ArgType type;
    int data;
};

using namespace hhal;

#define KERNEL_PATH "test/kernel/matrix_multiplication_dev"
#define KID 1
#define B1 1
#define B2 2
#define B3 3

int event_id_gen = 0;

int kernel_unit_id;
std::map<int, mango::mango_addr_t> event_addresses;

typedef struct registered_kernel_t {
    struct kernel k;
    int kernel_termination_event;
    std::vector<int> task_events;
} registered_kernel;

typedef struct registered_buffer_t {
    struct buffer b;
    int event;
} registered_buffer;

void prepare_events_registers(HHAL &hhal, std::vector<struct event> events, std::vector<registered_buffer> buffers);

void resource_allocation(HHAL &hhal, registered_kernel kernel, std::vector<registered_buffer> buffers, std::vector<struct event> events) {
	printf("[DummyRM] resource_allocation\n");

	static int u_pid = 0;
	static int kernel_address = 0xC0FFEE;
	static int event_address  = 0xDECADE;
	static int buffer_address = 0xBEEF;

    mango_cluster_id_t default_cluster_id = 0;
    mango_mem_id_t default_memory = 0;
    mango_unit_id_t default_unit = 0;
    mango_size_t num_tiles = 1;

    std::vector<mango_unit_type_t> types;
    std::vector<mango_unit_id_t> tiles_dst (num_tiles);

    auto unit_type = UnitType::GN;
    types.push_back(unit_type);
    printf("[DummyRM] resource_allocation: kernel %d type %d\n", kernel.k.id, unit_type);

    auto status = hhal.gn_manager.find_units_set(default_cluster_id, types, tiles_dst);
    if (status == GNManagerExitCode::ERROR){
        for (unsigned int i = 0; i< num_tiles; i++){
            tiles_dst[i]= u_pid++;
        }
    } else if (status != GNManagerExitCode::OK){
        printf("[DummyRM] resource_allocation: tiles mapping not found\n");
        return;
    }

    printf("[DummyRM] resource_allocation: %d tiles found\n", num_tiles);

    status = hhal.gn_manager.reserve_units_set(default_cluster_id, tiles_dst);
    if (status != GNManagerExitCode::OK){
        printf("[DummyRM] resource_allocation: tiles reservation failed\n");
        return;
    }

    mango_unit_id_t unit = 0;

    gn_kernel kernel_info;
    kernel_info.id = kernel.k.id;
    kernel_info.cluster_id = default_cluster_id;
    kernel_info.mem_tile = default_memory;
    kernel_info.physical_addr = kernel_address++;
    kernel_info.size = kernel.k.image_size;
    kernel_info.unit_id = tiles_dst[unit];
    kernel_info.task_events = kernel.task_events;
    kernel_info.termination_event = kernel.kernel_termination_event;

    // Hack for deallocation
    kernel_unit_id = kernel_info.unit_id;

    hhal.assign_kernel(hhal::Unit::GN, (hhal_kernel *) &kernel_info);

    printf("[DummyRM] resource_allocation: %d tiles reserved\n", num_tiles);

	for(auto &et : events) {
        gn_event info;
        info.id = et.id;
        info.physical_addr = event_address++;
        info.cluster_id = default_cluster_id;
        info.kernels_in = et.kernels_in;
        info.kernels_out = et.kernels_out;

        mango_addr_t phy_addr = event_address++;
        auto status = hhal.gn_manager.get_synch_register_addr(default_cluster_id, &phy_addr, 1);
        if (status != GNManagerExitCode::OK){
            printf("[DummyRM] resource_allocation: not enough registers\n");
            hhal.gn_manager.release_units_set(default_cluster_id, tiles_dst);
            return;
        }
        printf("[DummyRM] resource_allocation: event=%d, phy_addr=0x%x\n", et.id, phy_addr);
        info.physical_addr = phy_addr;
        // hack for deallocation
        event_addresses[info.id] = info.physical_addr;
        hhal.assign_event(hhal::Unit::GN, (hhal_event *) &info);
	}

	for(auto &bt : buffers) {
        gn_buffer info;
        info.id = bt.b.id;
        info.cluster_id = default_cluster_id;
        info.size = bt.b.size;
        info.event = bt.event;
        info.kernels_in = bt.b.kernels_in;
        info.kernels_out = bt.b.kernels_out;
        mango_mem_id_t memory = default_memory;
        mango_addr_t phy_addr = buffer_address++;


        hhal.gn_manager.find_memory(default_cluster_id, kernel_info.unit_id, info.size, &memory, &phy_addr);
        printf("[DummyRM] resource_allocation: buffer=%d, memory=%d, phy_addr=0x%x\n", info.id, memory, phy_addr);
        info.mem_tile = memory;
        info.physical_addr = phy_addr;
        hhal.assign_buffer(hhal::Unit::GN, (hhal_buffer *) &info);
        hhal.allocate_memory(info.id);
	}

    hhal.gn_manager.do_memory_management();
    prepare_events_registers(hhal, events, buffers);
}

void resource_deallocation(HHAL &hhal, struct kernel kernel, std::vector<struct buffer> buffers, std::vector<struct event> events) {
	printf("[DummyRM] resource_deallocation\n");
    mango_cluster_id_t default_cluster_id = 0;
    mango_size_t num_tiles = 1;

    std::vector<mango_unit_type_t> types;
    std::vector<mango_unit_id_t> tiles_dst;

    auto unit_type = UnitType::GN;
    types.push_back(unit_type);
    tiles_dst.push_back(kernel_unit_id);

    auto status = hhal.gn_manager.release_units_set(default_cluster_id, tiles_dst);
    if (status != GNManagerExitCode::OK){
        printf("[DummyRM] resource_deallocation: tiles release failed\n");
        return;
    }
    printf("[DummyRM] resource_deallocation: %d tiles released\n", num_tiles);

    for(auto &et : events) {
        hhal.gn_manager.release_synch_register_addr(default_cluster_id, event_addresses[et.id]);
        printf("[DummyRM] resource_deallocation: event=%d released\n", et.id);
    }

    for(auto &bt : buffers) {
        auto status = hhal.release_memory(bt.id);
        if (status != HHALExitCode::OK){
            printf("[DummyRM] resource_deallocation: release memory buffer %d failed\n", bt.id);
        }
        printf("[DummyRM] resource_deallocation: memory buffer %d released\n", bt.id);
    }

}

void init_matrix(int *matrix, int rows, int cols)
{
  for (int r=0;r<rows;r++) {
    for (int c=0;c<cols;c++) {
      matrix[r*cols+c] = random() % 100;
    }
  }
}

/* kernel function, reported here to allow checking the results 
 * obtained in the offloaded version 
 */
void kernel_function(int *A, int *B, int *C, int rows, int cols) {
  for (int r=0;r<rows;r++) {
    for (int c=0;c<cols;c++) {
      int v = 0;
      for (int p=0;p<rows;p++) {
        v = v + A[r * cols + p] * B[p * cols + c];
      }
      C[r * cols + c] = v;
    }
  }
	return;
}


void prepare_events_registers(HHAL &hhal, std::vector<struct event> events, std::vector<registered_buffer> buffers) {
    printf("Preparing event registers\n");
    // This function will initialize the event values to zero

	for(auto &et : events) {
        // It follows a strange pattern:
        // - We read the value (this should change to zero the register)
        events::read(hhal, et.id);
        // - We re-read the value and now must be zero
        int value = events::read(hhal, et.id);
        assert( 0 == value );
        UNUSED(value);
    }

	for(auto &bt : buffers) {
        auto et_id = bt.event;

        // assert(et->get_phy_addr() != 0); needed?

        events::read(hhal, et_id);	// Read the event BEFORE writing it to allow the initialization
                // in case of write-accumulare register
        int value = events::read(hhal, et_id);
        assert( 0 == value );
        UNUSED(value);

        const int WRITE = 2;
        events::write(hhal, et_id, WRITE);
    }
}

std::string get_arguments(HHAL &hhal, struct kernel kernel, std::vector<struct Arg> &args) {
	std::stringstream ss;

	//get full memory size
    uint32_t num_clusters;
    unsigned long long mem_size = 0;
    num_clusters = hhal.gn_manager.num_clusters;

    for (uint32_t cluster_id = 0; cluster_id< num_clusters; cluster_id++){
        hhal_tile_description_t htd = hhal.gn_manager.get_tiles(cluster_id);
        for (int i = 0; i < htd.total_tiles; i++) {
            mem_size += hhal.gn_manager.get_memory_size(cluster_id, i);
        }
    }
    ss << kernel.image_path;
    ss << " 0x" << std::hex << mem_size;

    auto &tlb = hhal.gn_manager.tlbs[kernel.id];

	for (const auto &arg : args) {
        switch (arg.type) {
            case ArgType::BUFFER:
                ss << " 0x" << std::hex << tlb.get_virt_addr_buffer(arg.data);
                break;
            case ArgType::EVENT:
                ss << " 0x" << std::hex << tlb.get_virt_addr_event(arg.data);
                break;
            case ArgType::SCALAR:
                ss << " " << arg.data;
                break;
            default:
                printf("Unknown argument\n");
                break;
        }
	}

	return ss.str();

}

registered_kernel register_kernel(struct kernel kernel) {
    return {
        kernel,
        event_id_gen++,
        // 3 "Kernel tasks events" are added for some reason, 
        // there is a TODO in libmango because it is not clear what this is doing or if its needed.
        {event_id_gen++, event_id_gen++, event_id_gen++},
    };
}


registered_buffer register_buffer(struct buffer buffer) {
    return {
        buffer,
        event_id_gen++,
    };
};

std::vector<Arg> arguments_setup(HHAL &hhal, registered_kernel kernel, std::vector<Arg> base_args) {
    std::vector<Arg> args;
    auto event = kernel.kernel_termination_event;
	auto tlb = hhal.gn_manager.tlbs[kernel.k.id];

	args.push_back({ArgType::EVENT, event});

	for(const auto ev : kernel.task_events) {
		args.push_back({ArgType::EVENT, ev});
	}

    args.insert(args.end(), base_args.begin(), base_args.end());
    return args;
}

int main(void) {
    HHAL hhal;

    std::ifstream kernel_fd(KERNEL_PATH, std::ifstream::in | std::ifstream::ate);
    size_t kernel_size = (size_t) kernel_fd.tellg() + 1;

    int *A;
    int *B;
    int *C;
    int *D;
    int rows;
    int columns;
    int out=0;

    rows = 5;
    columns = 5;

    size_t buffer_dim = rows*columns;
    size_t buffer_size = buffer_dim*sizeof(int); 

    /* matrix allocation */
    A = new int[buffer_dim];
    B = new int[buffer_dim];
    C = new int[buffer_dim];
    D = new int[buffer_dim];

    /* input matrices initialization */
    init_matrix(A, rows, columns);
    init_matrix(B, rows, columns);

    // /* initialization of the mango context */
    // mango_init("matrix_multiplication", "test_manga");

    struct kernel kernel = { KID, KERNEL_PATH, kernel_size };
    registered_kernel r_kernel = register_kernel(kernel);

    // kernelfunction *k = mango_kernelfunction_init();
    // #ifdef GNEMU
    // mango_load_kernel(kernel_file, k, GN, BINARY);

    // mango_kernel_t k1 = mango_register_kernel(KID, k, 2, 1, B1, B2, B3);  

    // /* Registration of buffers */
    // mango_buffer_t b1 = mango_register_memory(B1, rows*columns*sizeof(int), BUFFER, 0, 1, k1);
    // mango_buffer_t b2 = mango_register_memory(B2, rows*columns*sizeof(int), BUFFER, 0, 1, k1);
    // mango_buffer_t b3 = mango_register_memory(B3, rows*columns*sizeof(int), BUFFER, 1, 0, k1);

    std::vector<struct buffer> buffers = {
        {B1, buffer_size, {}, {KID}},
        {B2, buffer_size, {}, {KID}},
        {B3, buffer_size, {KID}, {}},
    };
    std::vector<registered_buffer> r_buffers;
    for(auto &b: buffers) {
        r_buffers.push_back(register_buffer(b));
    }

    /* Registration of task graph */
    // mango_task_graph_t *tg = mango_task_graph_create(1, 3, 0, k1, b1, b2, b3);

    struct event buffer_event = {r_buffers[2].event}; // buffer 3 event
    struct event kernel_termination_event = {r_kernel.kernel_termination_event};

    std::vector<struct event> events;
    events.push_back({r_kernel.kernel_termination_event, {r_kernel.k.id}, {r_kernel.k.id}});
    for(int e_id: r_kernel.task_events) {
        events.push_back({e_id, {r_kernel.k.id}, {r_kernel.k.id}});
    }
    for(auto &b: r_buffers) {
        events.push_back({b.event, b.b.kernels_in, b.b.kernels_out});
    }

    /* resource allocation */
    // mango_resource_allocation(tg);
    resource_allocation(hhal, r_kernel, r_buffers, events);
    printf("resource allocation done\n");

    /* Execution preparation */

    struct Arg arg1 = {ArgType::BUFFER, buffers[0].id};
    struct Arg arg2 = {ArgType::BUFFER, buffers[1].id};
    struct Arg arg3 = {ArgType::BUFFER, buffers[2].id};
    struct Arg arg4 = {ArgType::SCALAR, rows};
    struct Arg arg5 = {ArgType::SCALAR, columns};
    struct Arg arg6 = {ArgType::EVENT,  buffer_event.id};

    std::vector<Arg> args = arguments_setup(hhal, r_kernel, {arg1, arg2, arg3, arg4, arg5, arg6});

    /* Data transfer host->device */
    hhal.write_to_memory(B1, A, buffer_size);
    hhal.write_to_memory(B2, B, buffer_size);
    // mango_write(A, b1, DIRECT, 0);
    // mango_write(B, b2, DIRECT, 0);

    /* spawn kernel */
    // mango_event_t ev = mango_start_kernel(k1, args, 0);

    std::string arguments = get_arguments(hhal, kernel, args);
    printf("Kernel argument string:\n%s\n", arguments.c_str());
    // Gotta write 0 to the event before starting the kernel
    events::write(hhal, kernel_termination_event.id, 0);
    hhal.kernel_start_string_args(KID, arguments);

    /* reading results */
    // mango_wait(e);
    events::wait(hhal, buffer_event.id, 1);
    // mango_read(C, b3, DIRECT, 0);
    hhal.read_from_memory(B3, C, buffer_size);

    /* wait for kernel completion */
    // mango_wait(ev);
    events::wait(hhal, kernel_termination_event.id, 1);


    /* shut down the mango infrastructure */
    // mango_resource_deallocation(tg);
    // mango_task_graph_destroy_all(tg);
    // mango_release();
    resource_deallocation(hhal, kernel, buffers, events);


    /* check results */
    kernel_function(A, B, D, rows, columns);

    for(int i=0; i<rows; i++)
        for(int j=0; j<columns; j++)
            if(D[i*columns+j]!=C[i*columns+j]) {
                printf("Incorrect value at %d, %d: %d vs %d\n", i, j, D[i*columns+j], C[i*columns+j]);
                out++;
            }

    if (out) {
        printf("Detected %d errors in the computation\n", out);
        exit(out);
    } else {
        printf("Matrix multiplication correctly performed\n");
        exit(0);
    }
}
