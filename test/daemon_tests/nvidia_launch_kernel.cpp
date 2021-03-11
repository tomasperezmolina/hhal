#include <fstream>
#include <iostream>
#include <vector>
#include <assert.h>

#include "hhal.h"
#include "hhal_client.h"

#include "mango_arguments.h"
#include "event_utils.h"
#include "nvidia_dummy_rm.h"

#define DAEMON_PATH "/tmp/server-test"
#define KERNEL_PATH "cuda_kernels/saxpy"

#define KERNEL_ID 1
#define BUFFER_X_ID 0
#define BUFFER_Y_ID 1
#define BUFFER_O_ID 2

using namespace hhal;

void saxpy(float a, float *x, float *y, float *o, float n) {
    for (size_t i = 0; i < n; ++i) {
        o[i] = a * x[i] + y[i];
    }
}

int main(void) {
    hhal_daemon::HHALClient hhal(DAEMON_PATH);

    std::ifstream kernel_fd(KERNEL_PATH, std::ifstream::in | std::ifstream::ate);
    assert(kernel_fd.good() && "Kernel file does not exist");
    size_t kernel_size = (size_t) kernel_fd.tellg() + 1;

    mango_kernel kernel = { KERNEL_ID, kernel_size };
    nvidia_rm::registered_kernel r_kernel = nvidia_rm::register_kernel(kernel);

    mango_event kernel_termination_event = {r_kernel.kernel_termination_event};
    std::vector<mango_event> events = {kernel_termination_event};

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
        {BUFFER_X_ID, buffer_size, {}, {KERNEL_ID}},
        {BUFFER_Y_ID, buffer_size, {}, {KERNEL_ID}},
        {BUFFER_O_ID, buffer_size, {KERNEL_ID}, {}},
    };

    nvidia_rm::resource_allocation(hhal, {r_kernel}, buffers, events);

    const std::map<hhal::Unit, std::string> kernel_images = {{hhal::Unit::NVIDIA, KERNEL_PATH}};

    hhal.kernel_write(KERNEL_ID, kernel_images);

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
    printf("Before kernel finishes!\n");

    events::wait(hhal, kernel_termination_event.id, 1);

    hhal.read_from_memory(BUFFER_O_ID, o, buffer_size);

    for (size_t i = 0; i < 10; ++i) { // first 10 results only
        std::cout << a << " * " << x[i] << " + " << y[i] << " = " << o[i] << '\n';
    }

    float *expected = new float[n];
    saxpy(a, x, y, expected, n);

    bool correct = true;
    for (int i = 0; i < n; ++i) {
        if (o[i] != expected[i]) {
            printf("Sample host: Incorrect value at %d: got %.2f vs %.2f\n", i, o[i], expected[i]);
            std::cout << "Sample host: Stopping...\n" << std::endl;
            correct = false;
            break;
        }
    }
    if(correct) {
        std::cout << "Sample host: SAXPY correctly performed" << std::endl;
    }

    nvidia_rm::resource_deallocation(hhal, {r_kernel}, buffers, events);

    delete[] x;
    delete[] y;
    delete[] o;
    delete[] expected;

    return 0;
}
