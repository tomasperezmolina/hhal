#include <vector>
#include <map>
#include <stdio.h>

#include "hhal.h"

#include <stdlib.h>

struct kernel {
    int id;
    size_t image_size;
};

struct buffer {
    int id;
    size_t size;
};

struct event {
    int id;
};

using namespace hhal;

#define KERNEL_PATH "/opt/mango/usr/local/share/matrix_multiplication/matrix_multiplication_dev"
#define KID 1
#define B1 1
#define B2 2
#define B3 3

int kernel_unit_id;
std::map<int, mango::mango_addr_t> event_addresses;

void resource_allocation(HHAL &hhal, struct kernel kernel, std::vector<struct buffer> buffers, std::vector<struct event> events) {
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

    auto unit_type =  UnitType::GN;
    types.push_back(unit_type);
    printf("[DummyRM] resource_allocation: kernel %d type %d\n", kernel.id, unit_type);


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
    kernel_info.id = kernel.id;
    kernel_info.cluster_id = default_cluster_id;
    kernel_info.mem_tile = default_memory;
    kernel_info.physical_addr = kernel_address++;
    kernel_info.size = kernel.image_size;
    kernel_info.unit_id = tiles_dst[unit];
    kernel_info.virtual_addr = 0;
    kernel_info.tlb = 0;

    // Hack for deallocation
    kernel_unit_id = kernel_info.unit_id;

    hhal.assign_kernel_to_gn(kernel_info);

    printf("[DummyRM] resource_allocation: %d tiles reserved\n", num_tiles);

	for(auto &et : events) {
        gn_event info;
        info.physical_addr = event_address++;
        info.cluster_id = default_cluster_id;

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
        hhal.assign_event_to_gn(info);
	}

	for(auto &bt : buffers) {
        gn_buffer info;
        info.id = bt.id;
        info.cluster_id = default_cluster_id;
        info.size = bt.size;
        mango_mem_id_t memory = default_memory;
        mango_addr_t phy_addr = buffer_address++;


        hhal.gn_manager.find_memory(default_cluster_id, kernel_info.unit_id, info.size, &memory, &phy_addr);
        printf("[DummyRM] resource_allocation: buffer=%d, memory=%d, phy_addr=0x%x\n", info.id, memory, phy_addr);
        info.mem_tile = memory;
        info.physical_addr = phy_addr;
        hhal.assign_buffer_to_gn(info);
        hhal.allocate_memory(info.id);
	}

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

uint32_t read(HHAL &hhal, struct event event) {
    uint8_t value;
    if (hhal.read_sync_register(event.id, &value) != HHALExitCode::OK) {
        printf("Read synch register failed!\n");
    }
    return value;
}


uint32_t lock(HHAL &hhal, struct event event) {
	uint32_t value;
	do {
		value = read(hhal, event);
	} while (value == 0);
	return value;
}


void wait(HHAL &hhal, struct event event, uint32_t state) {

	uint32_t value;
	printf("Wait state %d: id %d", state, event.id);
	do {
		value = lock(hhal, event);

		if (value != state) {
			printf("Expected %d, instead it's %d\n", state, value);

            if (hhal.write_sync_register(event.id, value) != HHALExitCode::OK) {
                printf("Writing to sync register failed.\n");
            }
			// std::this_thread::yield();
		}

	} while (value != state);
}

std::string get_arguments(HHAL &hhal) {
	std::stringstream ss;

	//get full memory size
    uint32_t num_clusters;
    unsigned long long mem_size = 0;
    num_clusters = hhal.gn_manager.num_clusters;

    for (uint32_t cluster_id = 0; cluster_id< num_clusters; cluster_id++){
        hhal_tile_description_t htd = HHAL::get().get_tiles(cluster_id);
        for (int i = 0; i < htd.total_tiles; i++) {
            mem_size += HHAL::get().get_memory_size(cluster_id, i);
        }
    }
    ss << kernel->get_kernel()->get_kernel_version(arch_type);
    ss << " 0x" << std::hex << mem_size;

	for (const auto arg : args) {

		if (p_instanceof<BufferArg>(arg) || p_instanceof<EventArg>(arg)) {
			ss << " 0x" << std::hex << arg->get_value();
		}
		else if ( p_instanceof<ScalarArg<mango_size_t>>(arg) 
			 || p_instanceof<ScalarArg<char>>(arg) 
			 || p_instanceof<ScalarArg<unsigned char>>(arg) 
			 || p_instanceof<ScalarArg<short>>(arg) 
			 || p_instanceof<ScalarArg<unsigned short>>(arg) 
			 || p_instanceof<ScalarArg<int>>(arg) 
			 || p_instanceof<ScalarArg<unsigned int>>(arg) 
			 || p_instanceof<ScalarArg<float>>(arg)
 		) {
			ss << " " << arg->get_value();
		}
		else {
			mango_log->Warn("Unrecognized class in argument vectors.");
		}
	}

	return ss.str();

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

    char kernel_file[] = "/opt/mango/usr/local/share/matrix_multiplication/matrix_multiplication_dev";

    struct kernel kernel = { KID, kernel_size };

    // kernelfunction *k = mango_kernelfunction_init();
    // #ifdef GNEMU
    // mango_load_kernel(kernel_file, k, GN, BINARY);

    // mango_kernel_t k1 = mango_register_kernel(KID, k, 2, 1, B1, B2, B3);  

    // /* Registration of buffers */
    // mango_buffer_t b1 = mango_register_memory(B1, rows*columns*sizeof(int), BUFFER, 0, 1, k1);
    // mango_buffer_t b2 = mango_register_memory(B2, rows*columns*sizeof(int), BUFFER, 0, 1, k1);
    // mango_buffer_t b3 = mango_register_memory(B3, rows*columns*sizeof(int), BUFFER, 1, 0, k1);

    std::vector<struct buffer> buffers = {
        {B1, buffer_size},
        {B2, buffer_size},
        {B3, buffer_size},
    };

    int event_id_gen = 0;
    std::vector<struct event> events = {
        {event_id_gen++},
        {event_id_gen++},
        {event_id_gen++}
    };

    /* Registration of task graph */
    // mango_task_graph_t *tg = mango_task_graph_create(1, 3, 0, k1, b1, b2, b3);

    struct event buffer_event = events[2]; // buffer 3 event
    struct event kernel_termination_event; // ????
    kernel_termination_event.id = event_id_gen++;
    events.push_back(kernel_termination_event);

    // 3 "Kernel tasks events" are added for some reason, 
    // there is a TODO in libmango because it is not clear what this is doing or if its needed.
    for(int i = 0; i < 3; i++) {
        events.push_back({event_id_gen++});
    }

    /* resource allocation */
    // mango_resource_allocation(tg);
    resource_allocation(hhal, kernel, buffers, events);

    /* Execution preparation */
    mango_arg_t *arg1 = mango_arg( k1, &b1, sizeof(uint64_t), BUFFER );
    mango_arg_t *arg2 = mango_arg( k1, &b2, sizeof(uint64_t), BUFFER );
    mango_arg_t *arg3 = mango_arg( k1, &b3, sizeof(uint64_t), BUFFER );
    mango_arg_t *arg4 = mango_arg( k1, &rows, sizeof(uint32_t), SCALAR );
    mango_arg_t *arg5 = mango_arg( k1, &columns, sizeof(uint32_t), SCALAR );
    mango_event_t e = mango_get_buffer_event(b3);
    mango_arg_t *arg6 = mango_arg( k1, &e, sizeof(uint64_t), EVENT );

    mango_args_t *args = mango_set_args(k1, 6, arg1, arg2, arg3, arg4, arg5, arg6);


    /* Data transfer host->device */
    hhal.write_to_memory(B1, A, buffer_size);
    hhal.write_to_memory(B2, B, buffer_size);
    // mango_write(A, b1, DIRECT, 0);
    // mango_write(B, b2, DIRECT, 0);

    /* spawn kernel */
    // mango_event_t ev = mango_start_kernel(k1, args, 0);

    std::string arguments = "???";
    hhal.kernel_start(KID, arguments);

    /* reading results */
    // mango_wait(e);
    wait(hhal, buffer_event, 1);
    // mango_read(C, b3, DIRECT, 0);
    hhal.read_from_memory(B3, C, buffer_size);

    /* wait for kernel completion */
    // mango_wait(ev);
    wait(hhal, kernel_termination_event, 1);


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