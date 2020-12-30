#include <fstream>
#include <iostream>
#include <vector>

#include "hhal.h"

#include "kernel_arguments.h"
#include "cuda_argument_parser.h"

#define KERNEL_PATH "saxpy"
#define KERNEL_NAME "saxpy"

#define KERNEL_ID 1
#define BUFFER_X_ID 0
#define BUFFER_Y_ID 1
#define BUFFER_O_ID 2

using namespace hhal;
using namespace cuda_manager;

struct kernel {
    int id;
    size_t image_size;
};

struct buffer {
    int id;
    size_t size;
};

struct kernel_args {
    char *function_name;
    //int threads;
    //int blocks;
    int arg_count;
    char *arg_array;
};

// This should be handled in the RM side, it lives here for now as there are not two libraries at the moment.
// In the future there would be a library for RM which handles this,
// and a library for kernel execution which handles whats in the main function.
void resource_management(HHAL &hhal, struct kernel kernel, std::vector<struct buffer> &buffers) {
    nvidia_kernel k1 = { kernel.id, 0, kernel.id, kernel.image_size };
    hhal.assign_kernel(hhal::Unit::NVIDIA, (hhal_kernel *) &k1);
    hhal.allocate_kernel(KERNEL_ID);

    for (auto &buf: buffers) {
        nvidia_buffer buffer = { buf.id, 0, buf.id, buf.size };
        hhal.assign_buffer(hhal::Unit::NVIDIA, (hhal_buffer *) &buffer);
        hhal.allocate_memory(buffer.id);
    }
}

int main(void) {
    HHAL hhal;

    std::ifstream kernel_fd(KERNEL_PATH, std::ifstream::in | std::ifstream::ate);
    size_t kernel_size = (size_t) kernel_fd.tellg() + 1;

    struct kernel kernel = { KERNEL_ID, kernel_size };

    // Setup input and output buffers
    size_t n = 100;
    size_t buffer_size = n * sizeof(float);
    float a = 2.5f;
    float *x = new float[n], *y = new float[n], *o = new float[n];

    for (size_t i = 0; i < n; ++i) {
        x[i] = static_cast<float>(i);
        y[i] = static_cast<float>(i * 2);
    }

    std::vector<struct buffer> buffers = {
        {BUFFER_X_ID, buffer_size},
        {BUFFER_Y_ID, buffer_size},
        {BUFFER_O_ID, buffer_size},
    };

    resource_management(hhal, kernel, buffers);

    hhal.kernel_write(KERNEL_ID, KERNEL_PATH);

    hhal.write_to_memory(BUFFER_X_ID, x, buffer_size);
    hhal.write_to_memory(BUFFER_Y_ID, y, buffer_size);



    // Set up arguments
    int arg_count = 5;
    char *args = (char *) malloc(sizeof(ValueArg) * 2 + sizeof(BufferArg) * 3);
    char *current_arg = args;

    ValueArg *arg_a = (ValueArg *) current_arg;
    *arg_a = {VALUE, a};
    current_arg += sizeof(ValueArg);

    BufferArg *arg_x = (BufferArg *) current_arg;
    *arg_x = {BUFFER, BUFFER_X_ID, true};
    current_arg += sizeof(BufferArg);

    BufferArg *arg_y = (BufferArg *) current_arg;
    *arg_y = {BUFFER, BUFFER_Y_ID, true};
    current_arg += sizeof(BufferArg);

    BufferArg *arg_o = (BufferArg *) current_arg;
    *arg_o = {BUFFER, BUFFER_O_ID, false};
    current_arg += sizeof(BufferArg);

    ValueArg *arg_n = (ValueArg *) current_arg;
    *arg_n = {VALUE, (float)n};
    current_arg += sizeof(ValueArg);

    /* String args
    ValueArg  arg_a = {VALUE, a};
    BufferArg arg_x = {BUFFER, BUFFER_X_ID, true};
    BufferArg arg_y = {BUFFER, BUFFER_Y_ID, true};
    BufferArg arg_o = {BUFFER, BUFFER_O_ID, false};
    ValueArg  arg_n = {VALUE, (float)n};

    std::vector<void *> args { &arg_a, &arg_x, &arg_y, &arg_o, &arg_n };
    
    // Arguments to a string
    std::string arguments = args_to_string(KERNEL_NAME, KERNEL_ID, args);

    hhal.kernel_start_string_args(KERNEL_ID, arguments);
    */

    kernel_args kernel_arguments = { (char *) KERNEL_NAME, arg_count, args };

    // Launch kernel
    hhal.kernel_start(KERNEL_ID, &kernel_arguments);

    hhal.read_from_memory(BUFFER_O_ID, o, buffer_size);

    for (size_t i = 0; i < 10; ++i) { // first 10 results only
        std::cout << a << " * " << x[i] << " + " << y[i] << " = " << o[i] << '\n';
    }

    hhal.release_kernel(KERNEL_ID);
    hhal.release_memory(BUFFER_X_ID);
    hhal.release_memory(BUFFER_Y_ID);
    hhal.release_memory(BUFFER_O_ID);

    free(args);
    delete[] x;
    delete[] y;
    delete[] o;
}
