#ifndef TICTACTOE_SERVER_THREAD_POOL_H
#define TICTACTOE_SERVER_THREAD_POOL_H

#include <chrono>
#include <functional>
#include <thread>
#include <vector>

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
    //FIXME temporary impl
    std::vector<std::thread> threads;

public:
    explicit ThreadPool(ThreadPool::Config config = {}) {} //TODO implement

    ~ThreadPool() {
        //FIXME temporary impl
        for (auto& th : threads) {
            th.join();
        }
        //close active connections
        //reject pending connections
        //send shutdown signal
        //wait for threads destruction
    }

    bool submit_job(std::function<void()>&& job) {
        //FIXME temporary impl
        threads.emplace_back(std::forward<std::function<void()>>(job));
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
