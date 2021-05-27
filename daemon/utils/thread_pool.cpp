#include "nvidia/thread_pool.h"

void doNothing() {}

ThreadPool::ThreadPool(int nr_threads): done(false) {
    if (nr_threads <= 0)
        thread_count = std::thread::hardware_concurrency();
    else
        thread_count = nr_threads;

    for (int i=0; i < thread_count; ++i)
        threads.push_back(std::thread(&ThreadPool::worker_thread, this));
}

ThreadPool::~ThreadPool() {
    done = true;
    for (unsigned int i = 0; i < thread_count; i++) push_task(&doNothing);
    for (auto & th: threads)
        if (th.joinable())
            th.join();
}

void ThreadPool::worker_thread() {
    while (!done) {
        work_queue.get()();
    }
}