#include <fstream>

#include "nvidia/manager.h"

namespace hhal {

    NvidiaManagerExitCode NvidiaManager::assign_kernel(nvidia_kernel info) {
        kernel_info[info.id] = info;
        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::assign_buffer(nvidia_buffer info) {
        buffer_info[info.id] = info;
        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::assign_event(nvidia_event info) {
        event_info[info.id] = info;
        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::kernel_write(int kernel_id, std::string image_path) {
        nvidia_kernel &info = kernel_info[kernel_id];

        std::ifstream input_file(image_path, std::ifstream::in | std::ifstream::ate);

        if (!input_file.is_open()) {
            throw std::runtime_error("CudaHHAL: Unable to open kernel file.");
        }

        size_t input_size = (size_t) input_file.tellg();
        size_t buffer_size = input_size + 1;
        char *ptx = new char[buffer_size];

        input_file.seekg(0, std::ifstream::beg);
        input_file.read(ptx, input_size);
        input_file.close();
        ptx[input_size] = '\0';

        CudaApiExitCode err = cuda_api.write_kernel(info.mem_id, ptx, buffer_size);

        delete[] ptx;

        if (err != OK) {
            return NvidiaManagerExitCode::ERROR;
        }
        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::kernel_start(int kernel_id, std::string arguments) {
        CudaApiExitCode err = cuda_api.launch_kernel(arguments.c_str(), arguments.size());

        if (err != OK) {
            return NvidiaManagerExitCode::ERROR;
        }

        // TODO a separate thread should set this to indicate the kernel is done, also the arguments should specify the kernel.
        write_sync_register(0, 1);
        return NvidiaManagerExitCode::OK; 
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
        nvidia_kernel &info = kernel_info[kernel_id];

        CudaApiExitCode err = cuda_api.allocate_kernel(info.mem_id, info.size);

        if (err != OK) {
            return NvidiaManagerExitCode::ERROR;
        }

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
    
    NvidiaManagerExitCode NvidiaManager::release_kernel(int buffer_id) {
        nvidia_buffer &info = buffer_info[buffer_id];

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

    NvidiaManagerExitCode NvidiaManager::write_sync_register(int event_id, uint8_t data) {
        this->data = data;
        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::read_sync_register(int event_id, uint8_t *data) {
        uint8_t res = this->data;
        this->data = 0;
        *data = res;
        return NvidiaManagerExitCode::OK;
    }
}