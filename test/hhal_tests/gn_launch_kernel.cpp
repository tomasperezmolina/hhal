#include <vector>
#include <map>
#include <stdio.h>
#include <fstream>
#include <assert.h>

#include "hhal.h"

#include "arguments.h"

#include "mango_arguments.h"
#include "event_utils.h"
#include "gn_dummy_rm.h"


using namespace hhal;

#define KERNEL_PATH "gn_kernels/matrix_multiplication/matrix_multiplication_dev"
#define KID 1
#define B1 1
#define B2 2
#define B3 3

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

int main(void) {
    HHAL hhal;

    std::ifstream kernel_fd(KERNEL_PATH, std::ifstream::in | std::ifstream::ate);
    assert(kernel_fd.good() && "Kernel file does not exist");
    size_t kernel_size = (size_t) kernel_fd.tellg() + 1;

    int rows = 5;
    int columns = 5;

    size_t buffer_dim = rows * columns;
    size_t buffer_size = buffer_dim * sizeof(int); 

    /* matrix allocation */
    int *A = new int[buffer_dim], 
        *B = new int[buffer_dim], 
        *C = new int[buffer_dim], 
        *D = new int[buffer_dim];

    /* input matrices initialization */
    init_matrix(A, rows, columns);
    init_matrix(B, rows, columns);

    mango_kernel kernel = { KID, kernel_size };
    gn_rm::registered_kernel r_kernel = gn_rm::register_kernel(kernel);

    std::vector<mango_buffer> buffers = {
        {B1, buffer_size, {}, {KID}},
        {B2, buffer_size, {}, {KID}},
        {B3, buffer_size, {KID}, {}},
    };
    std::vector<gn_rm::registered_buffer> r_buffers;
    for(auto &b: buffers) {
        r_buffers.push_back(gn_rm::register_buffer(b));
    }

    mango_event buffer_event = {r_buffers[2].event}; // buffer 3 event
    mango_event kernel_termination_event = {r_kernel.kernel_termination_event};

    std::vector<mango_event> events;
    events.push_back({r_kernel.kernel_termination_event, {r_kernel.k.id}, {r_kernel.k.id}});
    for(int e_id: r_kernel.task_events) {
        events.push_back({e_id, {r_kernel.k.id}, {r_kernel.k.id}});
    }
    for(auto &b: r_buffers) {
        events.push_back({b.event, b.b.kernels_in, b.b.kernels_out});
    }

    /* resource allocation */
    resource_allocation(hhal, hhal.gn_manager, {r_kernel}, r_buffers, events);

    const std::map<hhal::Unit, std::string> kernel_images = {{hhal::Unit::GN, KERNEL_PATH}};
    hhal.kernel_write(kernel.id, kernel_images);
    printf("resource allocation done\n");

    /* Execution preparation */

    Arguments args;
    args.add_buffer({buffers[0].id});
    args.add_buffer({buffers[1].id});
    args.add_buffer({buffers[2].id});
    args.add_scalar({&rows, sizeof(rows), ScalarType::INT});
    args.add_scalar({&columns, sizeof(columns), ScalarType::INT});
    args.add_event({buffer_event.id});

    /* Data transfer host->device */
    hhal.write_to_memory(B1, A, buffer_size);
    hhal.write_to_memory(B2, B, buffer_size);

    /* spawn kernel */

    // Gotta write 0 to the event before starting the kernel
    events::write(hhal, kernel_termination_event.id, 0);
    hhal.kernel_start(KID, args);

    /* reading results */
    printf("Waiting for buffer event\n");
    events::wait(hhal, buffer_event.id, 1);
    hhal.read_from_memory(B3, C, buffer_size);

    /* wait for kernel completion */
    printf("Waiting for kernel termination event\n");
    events::wait(hhal, kernel_termination_event.id, 1);

    /* shut down the mango infrastructure */
    gn_rm::resource_deallocation(hhal, hhal.gn_manager, {kernel}, buffers, events);

    /* check results */
    kernel_function(A, B, D, rows, columns);

    int out = 0;
    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < columns; j++) {
            if(D[i*columns+j]!=C[i*columns+j]) {
                printf("Incorrect value at %d, %d: %d vs %d\n", i, j, D[i*columns+j], C[i*columns+j]);
                out++;
            }
        }
    }

    if (out) {
        printf("Detected %d errors in the computation\n", out);
    } else {
        printf("Matrix multiplication correctly performed\n");
    }

    delete[] A;
    delete[] B;
    delete[] C;
    delete[] D;

    return out;
}
