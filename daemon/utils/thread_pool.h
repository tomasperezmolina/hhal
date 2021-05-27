#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <atomic>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

#include "nvidia/synchronized_queue.h"

class ThreadPool
{

private:
    std::atomic<bool> done; // thread pool status
    unsigned int thread_count; // thread pool size
    SynchronizedQueue<std::function<void()>> work_queue;
    std::vector<std::thread> threads; 
    void worker_thread();

public:
    ThreadPool(int nr_threads = 0);

    virtual ~ThreadPool();

    void push_task(std::function<void ()> func) {
        work_queue.put(func);
    }

};
#endif //THREADPOOL_H