#ifndef TICTACTOE_SERVER_THREAD_POOL_H
#define TICTACTOE_SERVER_THREAD_POOL_H

#include <functional>
#include <thread>
#include <vector>

//thread pool should have a rw_lock (a shared_mutex with a shared_lock for read and unique_lock for write)
class ThreadPool {
public:
    class RejectedJobPolicy {
    public:
        virtual ~RejectedJobPolicy() = default;

        virtual void handle_rejected_job(const std::function<void()>&) = 0;
    };
    //TODO implement default rejection policy

    struct Config {
        unsigned task_queue_size;
        unsigned max_num_threads;
        unsigned thread_idle_timeout_millis;
        unsigned initial_thread_num;
        //TODO add default rejection policy

        Config() : task_queue_size(10), max_num_threads(15), thread_idle_timeout_millis(10 * 60 * 1000),
                   initial_thread_num(5) {}
    };

private:
    //fields...
    unsigned num_active_threads = 0;
//    RejectedJobPolicy* rejected_job_policy;
    //FIXME temporary impl
    std::vector<std::thread> threads;

public:
    explicit ThreadPool(ThreadPool::Config config = {}) {}

    ~ThreadPool() {
//        delete rejected_job_policy;
        //FIXME temporary impl
        for (auto& th : threads) {
            th.join();
        }
        //close active connections
        //reject pending connections
        //send shutdown signal
        //wait for threads destruction
    }

//    void set_rejected_job_policy(RejectedJobPolicy* policy) {
//        delete rejected_job_policy;
//        rejected_job_policy = policy;
//    }

    void submit_job(std::function<void()>&& job) {
        //FIXME temporary impl
        threads.emplace_back(std::forward<std::function<void()>>(job));
        //if rejected
//        rejected_job_policy->handle_rejected_job(job);
    }

    //public methods...

private:
    //private methods...

};

#endif //TICTACTOE_SERVER_THREAD_POOL_H
