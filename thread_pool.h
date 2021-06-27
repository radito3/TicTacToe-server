#ifndef TICTACTOE_SERVER_THREAD_POOL_H
#define TICTACTOE_SERVER_THREAD_POOL_H

#include <functional>
#include <thread>
#include <utility>
#include <vector>
#include <mutex>
#include <queue>
#include <future>
#include <condition_variable>
#include <iostream>

//thread pool should have a rw_lock (a shared_mutex with a shared_lock for read and unique_lock for write)
class ThreadPool {
public:
    class RejectedJobPolicy {
    public:
        virtual ~RejectedJobPolicy() = default;

        virtual void handle_rejected_job(const std::function<void()>&) = 0;
    };

    struct Config {
        unsigned task_queue_size;
        unsigned max_threads_num;
        unsigned thread_idle_timeout_millis;
        unsigned initial_thread_num;

        Config() : task_queue_size(10), max_threads_num(15), thread_idle_timeout_millis(10 * 60 * 1000),
                   initial_thread_num(5) {}
    };

private:
    RejectedJobPolicy* rejected_job_policy;
    unsigned num_active_threads;
    std::vector<std::thread> worker_threads;
    bool stopping = false;
    std::condition_variable empty_cond;
    std::mutex event_mutex;
    std::queue <std::function<void()>> task_queue;
    unsigned task_queue_size;

public:
    explicit ThreadPool(ThreadPool::Config config = {}) : task_queue_size(config.task_queue_size), num_active_threads(config.initial_thread_num) { 
        initialize(num_active_threads);
    }

    ~ThreadPool() {
        stopping = true;
            
        while(!task_queue.empty()) {
            auto &task = task_queue.front();
            rejected_job_policy->handle_rejected_job(task);
            task_queue.pop();
        }

        for(int i = 0 ; i < num_active_threads ; i++){
            task_queue.emplace([]() {});
            empty_cond.notify_all();
        }

        for (auto& th : worker_threads) {
            th.join();
        }

        delete rejected_job_policy;
    }

    void set_rejected_job_policy(RejectedJobPolicy* policy) {
        delete rejected_job_policy;
        rejected_job_policy = policy;
    }

    void submit_job(std::function<void()>&& job) {
        if(task_queue.size() < task_queue_size) {
            std::unique_lock<std::mutex> lock{event_mutex};
            task_queue.emplace(std::forward<std::function<void()>>(job));
            empty_cond.notify_one();
        } else {
            rejected_job_policy->handle_rejected_job(job);
        }
    }

private:

    void initialize(unsigned number_of_threads) {
        for (unsigned i = 0 ; i < number_of_threads ; i++) {
            worker_threads.emplace_back(&ThreadPool::thread_work, this);
        }
    }

    void thread_work() {
        while (!stopping) {
            std::unique_lock<std::mutex> lock{event_mutex};
            empty_cond.wait(lock, [&]() { return !task_queue.empty(); });
            auto &job = task_queue.front();

            try {
                job();
            } catch (const std::runtime_error& e) {
                std::cerr << e.what() << std::endl;
                rejected_job_policy->handle_rejected_job(job);
            }
            
            task_queue.pop();
        }
    }
};

#endif //TICTACTOE_SERVER_THREAD_POOL_H
