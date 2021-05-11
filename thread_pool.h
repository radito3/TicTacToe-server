#ifndef TICTACTOE_SERVER_THREAD_POOL_H
#define TICTACTOE_SERVER_THREAD_POOL_H

#include <chrono>
#include <functional>
#include <thread>

//thread pool should have a rw_lock (a shared_mutex with a shared_lock for read and unique_lock for write)
class ThreadPool {
public:
    struct Config {
        unsigned task_queue_size;
        unsigned max_num_threads;
        std::chrono::milliseconds thread_idle_timeout;
        unsigned initial_thread_num;

        Config() : task_queue_size(10), max_num_threads(15),
                    thread_idle_timeout(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::minutes(10))),
                    initial_thread_num(5) {}
    };

private:
    //fields...
    unsigned num_active_threads = 0;

public:
    explicit ThreadPool(ThreadPool::Config config = {}) {} //TODO implement

    ~ThreadPool() {
        //close active connections
        //reject pending connections
        //send shutdown signal
        //wait for threads destruction
    }

    bool submit_job(std::function<void()>&& job) {
        //FIXME temporary impl
        std::thread(std::forward<std::function<void()>>(job)).detach();
        //if accepted
        return true;
        //if rejected
        return false;
    }

    //public methods...

private:
    //private methods...

};

#endif //TICTACTOE_SERVER_THREAD_POOL_H
