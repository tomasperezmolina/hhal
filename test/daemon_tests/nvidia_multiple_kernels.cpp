#include <fstream>
#include <iostream>
#include <vector>
#include <assert.h>

#include "hhal.h"
#include "hhal_client.h"

#include "mango_arguments.h"
#include "event_utils.h"
#include "nvidia_dummy_rm.h"

#define KERNEL_1_PATH "cuda_kernels/saxpy_1"
#define KERNEL_2_PATH "cuda_kernels/saxpy_2"

#define KERNEL_1_ID 1
#define KERNEL_2_ID 2
#define BUFFER_X1_ID 0
#define BUFFER_O1_ID 1
#define BUFFER_X2_ID 2
#define BUFFER_Y2_ID 3
#define BUFFER_O2_ID 4

using namespace hhal;

void saxpy_1(float a, float *x, float *o, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        o[i] = a * x[i];
    }
}

void saxpy_2(float *x, float *y, float *o, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        o[i] = x[i] + y[i];
    }
}

void saxpy(float a, float *x, float *y, float *o, float n) {
    for (size_t i = 0; i < n; ++i) {
        o[i] = a * x[i] + y[i];
    }
}

int main(void) {
    hhal_daemon::HHALClient hhal(DAEMON_PATH);

    std::ifstream kernel_1_fd(KERNEL_1_PATH, std::ifstream::in | std::ifstream::ate);
    assert(kernel_1_fd.good() && "Kernel file does not exist");
    size_t kernel_1_size = (size_t) kernel_1_fd.tellg() + 1;

    std::ifstream kernel_2_fd(KERNEL_2_PATH, std::ifstream::in | std::ifstream::ate);
    assert(kernel_2_fd.good() && "Kernel file does not exist");
    size_t kernel_2_size = (size_t) kernel_2_fd.tellg() + 1;


    // Setup input and output buffers
    size_t n = 100;
    size_t buffer_size = n * sizeof(float);
    float a = 2.5f;
    float *x = new float[n], *o_1 = new float[n], *y = new float[n], *o_2 = new float[n];

    for (size_t i = 0; i < n; ++i) {
        x[i] = static_cast<float>(i);
        y[i] = static_cast<float>(i * 2);
    }


    mango_kernel kernel_1 = { KERNEL_1_ID, kernel_1_size };
    nvidia_rm::registered_kernel r_kernel_1 = nvidia_rm::register_kernel(kernel_1);
    mango_kernel kernel_2 = { KERNEL_2_ID, kernel_2_size };
    nvidia_rm::registered_kernel r_kernel_2 = nvidia_rm::register_kernel(kernel_2);

    std::vector<nvidia_rm::registered_kernel> kernels = {r_kernel_1, r_kernel_2};

    mango_event kernel_1_termination_event = {r_kernel_1.kernel_termination_event, {kernel_1.id}, {kernel_1.id}};
    mango_event kernel_2_termination_event = {r_kernel_2.kernel_termination_event, {kernel_2.id}, {kernel_2.id}};
    std::vector<mango_event> events = {kernel_1_termination_event, kernel_2_termination_event};

    std::vector<mango_buffer> buffers = {
        {BUFFER_X1_ID, buffer_size, {}, {KERNEL_1_ID}},
        {BUFFER_O1_ID, buffer_size, {KERNEL_1_ID}, {}},
        {BUFFER_X2_ID, buffer_size, {}, {KERNEL_2_ID}},
        {BUFFER_Y2_ID, buffer_size, {}, {KERNEL_2_ID}},
        {BUFFER_O2_ID, buffer_size, {KERNEL_2_ID}, {}},
    };

    nvidia_rm::resource_allocation(hhal, kernels, buffers, events);

    const std::map<hhal::Unit, std::string> kernel_1_images = {{hhal::Unit::NVIDIA, KERNEL_1_PATH}};
    const std::map<hhal::Unit, std::string> kernel_2_images = {{hhal::Unit::NVIDIA, KERNEL_2_PATH}};

    hhal.kernel_write(KERNEL_1_ID, kernel_1_images);
    hhal.kernel_write(KERNEL_2_ID, kernel_2_images);

    hhal.write_to_memory(BUFFER_X1_ID, x, buffer_size);
    printf("Sample host: Resource allocation done\n");

    float n_float = (float) n;

    Arguments arguments_1;
    scalar_arg scalar_arg11 = {hhal::ScalarType::FLOAT, sizeof(float)} ;
    scalar_arg11.afloat = a;
    arguments_1.add_scalar(scalar_arg11);

    arguments_1.add_buffer({BUFFER_X1_ID});
    arguments_1.add_buffer({BUFFER_O1_ID});

    scalar_arg scalar_arg12 = {hhal::ScalarType::FLOAT, sizeof(float)} ;
    scalar_arg12.afloat = n_float;
    arguments_1.add_scalar(scalar_arg12);

    Arguments arguments_2;
    arguments_2.add_buffer({BUFFER_X2_ID});
    arguments_2.add_buffer({BUFFER_Y2_ID});
    arguments_2.add_buffer({BUFFER_O2_ID});

    scalar_arg scalar_arg21 = {hhal::ScalarType::FLOAT, sizeof(float)} ;
    scalar_arg21.afloat = n_float;
    arguments_2.add_scalar(scalar_arg21);


    // Launch kernel 1
    hhal.kernel_start(KERNEL_1_ID, arguments_1);

    printf("Sample host: Waiting for kernel 1 termination...\n");

    events::wait(hhal, kernel_1_termination_event.id, 1);

    hhal.read_from_memory(BUFFER_O1_ID, o_1, buffer_size);

    // Check kernel 1 results
    float *expected_1 = new float[n];
    saxpy_1(a, x, expected_1, n);

    bool correct_1 = true;
    for (int i = 0; i < n; ++i) {
        if (o_1[i] != expected_1[i]) {
            printf("Sample host: Incorrect value at %d: got %.2f vs %.2f\n", i, o_1[i], expected_1[i]);
            printf("Sample host: Stopping...\n");
            correct_1 = false;
            break;
        }
    }
    if(correct_1) {
        printf("Sample host: First stage of SAXPY correctly performed\n");
    } else {
        exit(1);
    }

    // Prepare data for kernel 2
    hhal.write_to_memory(BUFFER_X2_ID, o_1, buffer_size);
    hhal.write_to_memory(BUFFER_Y2_ID, y, buffer_size);

    // Launch kernel 2

    hhal.kernel_start(KERNEL_2_ID, arguments_2);

    printf("Sample host: Waiting for kernel 2 termination...\n");

    events::wait(hhal, kernel_2_termination_event.id, 1);
    hhal.read_from_memory(BUFFER_O2_ID, o_2, buffer_size);

    // Check kernel 2 results
    float *expected_2 = new float[n];
    saxpy_2(o_1, y, expected_2, n);

    bool correct_2 = true;
    for (int i = 0; i < n; ++i) {
        if (o_2[i] != expected_2[i]) {
            printf("Sample host: Incorrect value at %d: got %.2f vs %.2f\n", i, o_2[i], expected_2[i]);
            printf("Sample host: Stopping...\n");
            correct_2 = false;
            break;
        }
    }
    if(correct_1) {
        printf("Sample host: Second stage of SAXPY correctly performed\n");
    } else {
        exit(1);
    }



    float *expected_3 = new float[n];

    /* check results */
    saxpy(a, x, y, expected_3, n);

    bool correct_3 = true;
    for (int i = 0; i < n; ++i) {
        if (o_2[i] != expected_3[i]) {
            printf("Sample host: SAXPY: Incorrect value at %d: got %.2f vs %.2f\n", i, o_2[i], expected_2[i]);
            printf("Sample host: Stopping...\n");
            correct_3 = false;
            break;
        }
    }
    if(correct_3) {
        printf("Sample host: SAXPY correctly performed\n");
    } else {
        exit(1);
    }

    nvidia_rm::resource_deallocation(hhal, kernels, buffers, events);

    delete[] x;
    delete[] y;
    delete[] o_1;
    delete[] o_2;
    delete[] expected_1;
    delete[] expected_2;
    delete[] expected_3;

    return 0;
}
