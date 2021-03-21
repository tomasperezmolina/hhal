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
#include "nvidia_dummy_rm.h"


using namespace hhal;

#define KERNEL_1_PATH "gn_kernels/saxpy1/gn_saxpy_1"
#define KERNEL_2_PATH "cuda_kernels/saxpy_2"
#define KID_1 1
#define KID_2 2
#define BX_1_ID 1
#define BO1_ID 2
#define BX_2_ID 3
#define BY_ID 4
#define BO2_ID 5

void saxpy_1(int a, float *x, float *o, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        o[i] = a * x[i];
    }
}

void saxpy_2(float *x, float *y, float *o, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        o[i] = x[i] + y[i];
    }
}

void saxpy(int a, float *x, float *y, float *o, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        o[i] = a * x[i] + y[i];
    }
}

int main(void) {
    HHAL hhal;

    std::ifstream kernel_1_fd(KERNEL_1_PATH, std::ifstream::in | std::ifstream::ate);
    assert(kernel_1_fd.good() && "Kernel file 1 does not exist");
    size_t kernel_1_size = (size_t) kernel_1_fd.tellg() + 1;

    std::ifstream kernel_2_fd(KERNEL_2_PATH, std::ifstream::in | std::ifstream::ate);
    assert(kernel_2_fd.good() && "Kernel file 2 does not exist");
    size_t kernel_2_size = (size_t) kernel_2_fd.tellg() + 1;

    // Setup input and output buffers
    size_t n = 100;
    size_t buffer_size = n * sizeof(float);
    int a = 3;
    float *x = new float[n], *o_1 = new float[n], *y = new float[n], *o_2 = new float[n];

    for (size_t i = 0; i < n; ++i) {
        x[i] = static_cast<float>(i);
        y[i] = static_cast<float>(i * 2);
    }

    mango_kernel kernel_1 = { KID_1, kernel_1_size };
    gn_rm::registered_kernel r_kernel_1 = gn_rm::register_kernel(kernel_1);
    mango_kernel kernel_2 = { KID_2, kernel_2_size };
    nvidia_rm::registered_kernel r_kernel_2 = nvidia_rm::register_kernel(kernel_2);

    std::vector<mango_buffer> gn_buffers = {
        {BX_1_ID, buffer_size, {}, {KID_1}},
        {BO1_ID, buffer_size, {KID_1}, {}},
    };

    std::vector<mango_buffer> nvidia_buffers = {
        {BX_2_ID, buffer_size, {}, {KID_2}},
        {BY_ID, buffer_size, {}, {KID_2}},
        {BO2_ID, buffer_size, {KID_2}, {}},
    };

    std::vector<gn_rm::registered_buffer> r_gn_buffers;
    for(auto &b: gn_buffers) {
        r_gn_buffers.push_back(gn_rm::register_buffer(b));
    }

    mango_event kernel_1_termination_event = {r_kernel_1.kernel_termination_event};
    mango_event kernel_2_termination_event = {r_kernel_2.kernel_termination_event};

    std::vector<mango_event> gn_events;
    gn_events.push_back({r_kernel_1.kernel_termination_event, {r_kernel_1.k.id}, {r_kernel_1.k.id}});
    for(auto &b: r_gn_buffers) {
        gn_events.push_back({b.event, b.b.kernels_in, b.b.kernels_out});
    }

    std::vector<mango_event> nvidia_events = {kernel_2_termination_event};;

    /* resource allocation */
    gn_rm::resource_allocation(hhal, {r_kernel_1}, r_gn_buffers, gn_events);
    nvidia_rm::resource_allocation(hhal, {r_kernel_2}, nvidia_buffers, nvidia_events);

    const std::map<hhal::Unit, std::string> kernel_1_images = {{hhal::Unit::GN, KERNEL_1_PATH}};
    const std::map<hhal::Unit, std::string> kernel_2_images = {{hhal::Unit::NVIDIA, KERNEL_2_PATH}};
    printf("resource allocation done\n");

    hhal.kernel_write(kernel_1.id, kernel_1_images);
    hhal.kernel_write(kernel_2.id, kernel_2_images);

    /* Execution preparation */

    Arguments args_k_1;
    scalar_arg scalar_arg11 = {hhal::ScalarType::INT, sizeof(int32_t)};
    scalar_arg11.aint32 = a;
    args_k_1.add_scalar(scalar_arg11);
    args_k_1.add_buffer({BX_1_ID});
    args_k_1.add_buffer({BO1_ID});
    scalar_arg scalar_arg12 = {hhal::ScalarType::INT, sizeof(int32_t)};
    scalar_arg12.aint32 = n;
    args_k_1.add_scalar(scalar_arg12);

    float n_float = (float) n;

    Arguments args_k_2;
    args_k_2.add_buffer({BX_2_ID});
    args_k_2.add_buffer({BY_ID});
    args_k_2.add_buffer({BO2_ID});
    scalar_arg scalar_arg21 = {hhal::ScalarType::FLOAT, sizeof(float)};
    scalar_arg21.afloat = n_float;
    args_k_2.add_scalar(scalar_arg21);

    /* Data transfer host->device */
    hhal.write_to_memory(BX_1_ID, x, buffer_size);

    /* spawn kernel */

    // Gotta write 0 to the event before starting the kernel
    events::write(hhal, kernel_1_termination_event.id, 0);
    hhal.kernel_start(KID_1, args_k_1);

    /* wait for kernel completion */
    printf("Waiting for kernel termination event\n");
    events::wait(hhal, kernel_1_termination_event.id, 1);
    hhal.read_from_memory(BO1_ID, o_1, buffer_size);

    float *expected_1 = new float[n];

    /* check results */
    saxpy_1(a, x, expected_1, n);

    bool correct_1 = true;
    for (int i = 0; i < n; ++i) {
        if (o_1[i] != expected_1[i]) {
            printf("Sample host: SAXPY 1: Incorrect value at %d: got %.2f vs %.2f\n", i, o_1[i], expected_1[i]);
            printf("Sample host: Stopping...\n");
            correct_1 = false;
            break;
        }
    }
    if(correct_1) {
        printf("Sample host: first stage of SAXPY correctly performed\n");
    }

    /* Data transfer host->device */
    hhal.write_to_memory(BX_2_ID, o_1, buffer_size);
    hhal.write_to_memory(BY_ID, y, buffer_size);

    /* spawn kernel */

    // Gotta write 0 to the event before starting the kernel
    events::write(hhal, kernel_2_termination_event.id, 0);
    hhal.kernel_start(KID_2, args_k_2);

    /* wait for kernel completion */
    printf("Waiting for kernel termination event\n");
    events::wait(hhal, kernel_2_termination_event.id, 1);
    hhal.read_from_memory(BO2_ID, o_2, buffer_size);

    float *expected_2 = new float[n];

    /* check results */
    saxpy_2(o_1, y, expected_2, n);

    bool correct_2 = true;
    for (int i = 0; i < n; ++i) {
        if (o_2[i] != expected_2[i]) {
            printf("Sample host: SAXPY 2: Incorrect value at %d: got %.2f vs %.2f\n", i, o_2[i], expected_2[i]);
            printf("Sample host: Stopping...\n");
            correct_2 = false;
            break;
        }
    }
    if(correct_2) {
        printf("Sample host: second stage of SAXPY correctly performed\n");
    }

    gn_rm::resource_deallocation(hhal, {kernel_1}, gn_buffers, gn_events);
    nvidia_rm::resource_deallocation(hhal, {r_kernel_2}, nvidia_buffers, nvidia_events);

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
    }

    delete[] x;
    delete[] y;
    delete[] o_1;
    delete[] o_2;
    delete[] expected_1;
    delete[] expected_2;
    delete[] expected_3;

    return 0;
}
