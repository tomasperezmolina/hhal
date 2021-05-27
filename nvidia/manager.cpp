#include <fstream>
#include <algorithm>
#include <thread>
#include <functional>

#ifdef PROFILING_MODE
#include "profiling.h"
#endif

#include "nvidia/manager.h"
#include "kernel_arguments.h"

namespace hhal {

    NvidiaManagerExitCode NvidiaManager::assign_kernel(nvidia_kernel *info) {
        printf("NvidiaManager: Assigning kernel %d, mem_id=%d\n", info->id, info->mem_id);
        kernel_info[info->id] = *info;
        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::assign_buffer(nvidia_buffer *info) {
        printf("NvidiaManager: Assigning buffer %d\n", info->id);
        buffer_info[info->id] = *info;
        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::assign_event(nvidia_event *info) {
        printf("NvidiaManager: Assigning event %d\n", info->id);
        event_info[info->id] = *info;
        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::deassign_kernel(int kernel_id) {
        printf("NvidiaManager: Deassigning kernel %d\n", kernel_id);
        kernel_info.erase(kernel_id);
        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::deassign_buffer(int buffer_id) {
        printf("NvidiaManager: Deassigning buffer %d\n", buffer_id);
        buffer_info.erase(buffer_id);
        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::deassign_event(int event_id) {
        printf("NvidiaManager: Deassigning event %d\n", event_id);
        event_info.erase(event_id);
        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::kernel_write(int kernel_id, std::string image_path) {
        nvidia_kernel &info = kernel_info[kernel_id];

        std::ifstream input_file(image_path, std::ifstream::in | std::ifstream::ate);

        if (!input_file.is_open()) {
            return NvidiaManagerExitCode::ERROR;
        }

        size_t input_size = (size_t) input_file.tellg();
        size_t buffer_size = input_size + 1;
        char *ptx = new char[buffer_size];

        input_file.seekg(0, std::ifstream::beg);
        input_file.read(ptx, input_size);
        input_file.close();
        ptx[input_size] = '\0';

        CudaApiExitCode all_err = cuda_api.allocate_kernel(info.mem_id, buffer_size);

        if (all_err != OK) {
            delete [] ptx;
            return NvidiaManagerExitCode::ERROR;
        }

        CudaApiExitCode err = cuda_api.write_kernel(info.mem_id, ptx, buffer_size);

        delete[] ptx;

        if (err != OK) {
            return NvidiaManagerExitCode::ERROR;
        }

        // For now we assume that the function name is equal to the filename
        std::string function_name = image_path;

        // Remove directory if present.
        const size_t last_slash_idx = function_name.find_last_of("/");
        if (std::string::npos != last_slash_idx) {
            function_name.erase(0, last_slash_idx + 1);
        }

        // Remove extension if present.
        const size_t period_idx = function_name.rfind('.');
        if (std::string::npos != period_idx) {
            function_name.erase(period_idx);
        }
        kernel_function_names[info.id] = function_name;

        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::kernel_start(int kernel_id, const Arguments &arguments) {
        auto &args = arguments.get_args();
        int arg_count = args.size();
        size_t arg_array_size = 0;
        size_t arg_scalar_size = 0;
        for(auto &arg: args) {
            switch (arg.type) {
                case ArgumentType::BUFFER:
                    arg_array_size += sizeof(cuda_manager::BufferArg);
                    break;
                case ArgumentType::EVENT:
                    printf("NvidiaManager: Event arguments not supported yet\n");
                    return NvidiaManagerExitCode::ERROR;
                    break;
                case ArgumentType::SCALAR:
                    arg_array_size += sizeof(cuda_manager::ScalarArg);
                    arg_scalar_size += arg.scalar.size;
                    break;
                default:
                    printf("NvidiaManager: Unknown argument\n");
                    return NvidiaManagerExitCode::ERROR; 
            }
        }

        char *scalar_allocations = (char*) malloc(arg_scalar_size);
        char *current_allocation = scalar_allocations;
        char *arg_array = (char*) malloc(arg_array_size);
        char *current_arg = arg_array;

        for(auto &arg: args) {
            switch (arg.type) {
                case ArgumentType::BUFFER: {
                    auto &b_info = buffer_info[arg.buffer.id];
                    // If the buffer has the kernel as an output (the kernel will READ from the buffer)
                    // then the buffer is an input to that kernel.
                    auto &kernels_out = b_info.kernels_out;
                    bool is_in = std::find(kernels_out.begin(), kernels_out.end(), kernel_id) != kernels_out.end();
                    auto *arg_x = (cuda_manager::BufferArg *) current_arg;
                    *arg_x = {cuda_manager::BUFFER, b_info.id, is_in};
                    current_arg += sizeof(cuda_manager::BufferArg);
                    break;
                } 
                case ArgumentType::SCALAR: {
                    hhal::scalar_arg scalar = arg.scalar;
                    auto *arg_a = (cuda_manager::ScalarArg *) current_arg;
                    void *scalar_ptr = get_scalar_ptr(&scalar);
                    memcpy(current_allocation, scalar_ptr, scalar.size);

                    *arg_a = {cuda_manager::SCALAR, (void*)current_allocation};

                    current_allocation += scalar.size;
                    current_arg += sizeof(cuda_manager::ScalarArg);
                    arg_array_size += sizeof(cuda_manager::ScalarArg);
                    break;
                }
                default:
                    printf("NvidiaManager: This should not happen\n");
                    return NvidiaManagerExitCode::ERROR;
            }
        }

        thread_pool.push_task(std::bind(&NvidiaManager::launch_kernel, this, kernel_id, arg_array, arg_count, scalar_allocations));

        return NvidiaManagerExitCode::OK;
    }

    void NvidiaManager::launch_kernel(int kernel_id, char *arg_array, int arg_count, char* scalar_allocations) {
        // Should we add mutexes for the kernel and event maps? They should not be modified after being assigned anyway.
        nvidia_kernel &info = kernel_info[kernel_id];

        CudaResourceArgs r_args = {info.gpu_id, {info.grid_dim_x, info.grid_dim_y, info.grid_dim_z}, {info.block_dim_x, info.block_dim_y, info.block_dim_z}};

        auto &termination_event = event_info[info.termination_event];

#ifdef PROFILING_MODE
        auto ref = profiling::Profiler::get_instance().start_kernel_execution(kernel_id);
#endif
        CudaApiExitCode err = cuda_api.launch_kernel(kernel_id, kernel_function_names[kernel_id].c_str(), r_args, arg_array, arg_count);
#ifdef PROFILING_MODE
        ref->finish();
#endif

        free(arg_array);
        free(scalar_allocations);

        if (err != OK) {
            printf("[Error] NvidiaManager: Error launching kernel\n");
        }

        write_sync_register(termination_event.id, 1);
    }

    NvidiaManagerExitCode NvidiaManager::allocate_memory(int buffer_id) {
        nvidia_buffer &info = buffer_info[buffer_id];

        CudaApiExitCode err = cuda_api.allocate_memory(info.mem_id, info.size);

        if (err != OK) {
            return NvidiaManagerExitCode::ERROR; 
        }

        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::allocate_kernel(int kernel_id) {
        // Allocating at kernel write time because we no longer have kernel size info here
        /*
        nvidia_kernel &info = kernel_info[kernel_id];

        CudaApiExitCode err = cuda_api.allocate_kernel(info.mem_id, info.size);

        if (err != OK) {
            return NvidiaManagerExitCode::ERROR;
        }
        */
        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::release_memory(int buffer_id) {
        nvidia_buffer &info = buffer_info[buffer_id];

        CudaApiExitCode err = cuda_api.deallocate_memory(info.mem_id);

        if (err != OK) {
            return NvidiaManagerExitCode::ERROR;
        }

        return NvidiaManagerExitCode::OK;
    }
    
    NvidiaManagerExitCode NvidiaManager::release_kernel(int kernel_id) {
        nvidia_kernel &info = kernel_info[kernel_id];

        CudaApiExitCode err = cuda_api.deallocate_kernel(info.mem_id);

        if (err != OK) {
            return NvidiaManagerExitCode::ERROR;
        }

        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::write_to_memory(int buffer_id, const void *source, size_t size) {
        nvidia_buffer &info = buffer_info[buffer_id];

        CudaApiExitCode err = cuda_api.write_memory(info.mem_id, source, size);

        if (err != OK) {
            return NvidiaManagerExitCode::ERROR;
        }

        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::read_from_memory(int buffer_id, void *dest, size_t size) {
        nvidia_buffer &info = buffer_info[buffer_id];

        CudaApiExitCode err = cuda_api.read_memory(info.mem_id, dest, size);

        if (err != OK) {
            return NvidiaManagerExitCode::ERROR;
        }

        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::write_sync_register(int event_id, uint32_t data) {
        auto ec = registry.write_event(event_id, data);
        if (ec != EventRegistryExitCode::OK) {
            return NvidiaManagerExitCode::ERROR;
        }
        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::read_sync_register(int event_id, uint32_t *data) {
        auto ec = registry.read_event(event_id, data);
        if (ec != EventRegistryExitCode::OK) {
            return NvidiaManagerExitCode::ERROR;
        }
        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::allocate_event(int event_id) {
        auto ec = registry.add_event(event_id);
        if (ec != EventRegistryExitCode::OK) {
            return NvidiaManagerExitCode::ERROR;
        }
        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::release_event(int event_id) {
        auto ec = registry.remove_event(event_id);
        if (ec != EventRegistryExitCode::OK) {
            return NvidiaManagerExitCode::ERROR;
        }
        return NvidiaManagerExitCode::OK;
    }
}
