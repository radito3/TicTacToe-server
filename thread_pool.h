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
    unsigned num_active_threads = 0;
    //RejectedJobPolicy* rejected_job_policy;
    //FIXME temporary impl
    std::vector<std::thread> threads;
    //bool stoping = false;
    //std::condition_variable event_var;
    //std::mutex event_mutex;
    //std::queue <std::function<void()>>  task_queue;
    unsigned task_queue_size;

public:
    explicit ThreadPool(ThreadPool::Config config = {}) : task_queue_size(config.task_queue_size) {
        //start();
    } //TODO implement

    ~ThreadPool() {
        delete rejected_job_policy;
        //send_ stop signal to threads
        for (auto& th : worker_threads) {
            th.join();
        }
        //stop();

        //close active connections
        //reject pending connections
        //send shutdown signal
        //wait for threads destruction
    }

    void set_rejected_job_policy(RejectedJobPolicy* policy) {
        delete rejected_job_policy;
        rejected_job_policy = policy;
    }

>>>>>>> 14dde82 (Add protobuf definitions & work on impl)

    void submit_job(std::function<void()>&& job) {
        threads.emplace_back(std::forward<std::function<void()>>(job));
         /*if (task_queue::size() < task_queue_size) {

            auto wrapper = std::make_shared<std::packaged_task<decltype(job()) ()>>(std::move(job()));

            {
                std::unique_lock<std::mutex> lock{ event_mutex };
                task_queue.emplace([=] {
                    (*wrapper)();
                    })
            }

            event_var.notify_one();
            return wrapper->get_future();

        }
        else {
            rejected_job_policy->handle_rejected_job(job);
        }*/
    }

private:
    //private methods...

  

    /*void start(num_active_threads) {
        for (auto i = 0u ; i < num_active_threads; ++i) {
            threads.emplace_back([=] {
                while (true) {

                    std::function<void()> job;

                    {
                        std::unique_lock<std::mutex> lock{ event_mutex };
                        event_var.wait(lock, [=] {return stoping || !task_queue.empty(); });

                        if (stoping && task_queue.empty())
                            break;

                        job = std::move(task_queue.front());
                        task_queue.pop();

                    }

                    job();
                    
                }
            })
        }
    }
    void stop() noexcept
    {
        {
            std::unique_lock<std::mutex> lock{event_mutex};
            stoping = true;
        }
 
        event_var.notify_all();
 
        for (auto& th : threads) {
            th.join();
        }
    }*/
};

#endif //TICTACTOE_SERVER_THREAD_POOL_H