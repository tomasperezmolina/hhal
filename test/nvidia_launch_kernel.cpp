#include <fstream>
#include <iostream>
#include <vector>

#include "hhal.h"

#include "kernel_arguments.h"
#include "cuda_argument_parser.h"

#include "test/mango_arguments.h"

#define KERNEL_PATH "saxpy"

#define KERNEL_ID 1
#define BUFFER_X_ID 0
#define BUFFER_Y_ID 1
#define BUFFER_O_ID 2

using namespace hhal;
using namespace cuda_manager;

// This should be handled in the RM side, it lives here for now as there are not two libraries at the moment.
// In the future there would be a library for RM which handles this,
// and a library for kernel execution which handles whats in the main function.
void resource_management(HHAL &hhal, mango_kernel kernel, std::vector<mango_buffer> &buffers) {
    nvidia_kernel k1 = { kernel.id, 0, kernel.id, kernel.image_size };
    hhal.assign_kernel(hhal::Unit::NVIDIA, (hhal_kernel *) &k1);
    hhal.allocate_kernel(KERNEL_ID);

    for (auto &buf: buffers) {
        nvidia_buffer buffer = { buf.id, 0, buf.id, buf.size, buf.kernels_in, buf.kernels_out };
        hhal.assign_buffer(hhal::Unit::NVIDIA, (hhal_buffer *) &buffer);
        hhal.allocate_memory(buffer.id);
    }
}

int main(void) {
    HHAL hhal;

    std::ifstream kernel_fd(KERNEL_PATH, std::ifstream::in | std::ifstream::ate);
    size_t kernel_size = (size_t) kernel_fd.tellg() + 1;

    mango_kernel kernel = { KERNEL_ID, kernel_size };

    // Setup input and output buffers
    size_t n = 100;
    size_t buffer_size = n * sizeof(float);
    float a = 2.5f;
    float *x = new float[n], *y = new float[n], *o = new float[n];

    for (size_t i = 0; i < n; ++i) {
        x[i] = static_cast<float>(i);
        y[i] = static_cast<float>(i * 2);
    }

    std::vector<mango_buffer> buffers = {
        {BUFFER_X_ID, buffer_size, {KERNEL_ID}, {}},
        {BUFFER_Y_ID, buffer_size, {KERNEL_ID}, {}},
        {BUFFER_O_ID, buffer_size, {}, {KERNEL_ID}},
    };

    resource_management(hhal, kernel, buffers);

    hhal.kernel_write(KERNEL_ID, KERNEL_PATH);

    hhal.write_to_memory(BUFFER_X_ID, x, buffer_size);
    hhal.write_to_memory(BUFFER_Y_ID, y, buffer_size);

    float n_float = (float) n;

    Arguments arguments;
    arguments.add_scalar({&a, sizeof(a), ScalarType::FLOAT});
    arguments.add_buffer({BUFFER_X_ID});
    arguments.add_buffer({BUFFER_Y_ID});
    arguments.add_buffer({BUFFER_O_ID});
    arguments.add_scalar({&n_float, sizeof(float), ScalarType::FLOAT});

    // Launch kernel
    hhal.kernel_start(KERNEL_ID, arguments);

    hhal.read_from_memory(BUFFER_O_ID, o, buffer_size);

    for (size_t i = 0; i < 10; ++i) { // first 10 results only
        std::cout << a << " * " << x[i] << " + " << y[i] << " = " << o[i] << '\n';
    }

    hhal.release_kernel(KERNEL_ID);
    hhal.release_memory(BUFFER_X_ID);
    hhal.release_memory(BUFFER_Y_ID);
    hhal.release_memory(BUFFER_O_ID);

    delete[] x;
    delete[] y;
    delete[] o;
}
