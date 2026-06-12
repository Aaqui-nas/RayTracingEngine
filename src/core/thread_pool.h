#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>
#include <atomic>

namespace rt {
    class ThreadPool {
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;
        std::mutex queue_mutex;
        std::condition_variable cv;
        bool stopped = false;
        std::atomic<int> active_tasks{0};

    public:
        explicit ThreadPool(std::size_t num_threads) {
            workers.reserve(num_threads);
            for (size_t i = 0; i < num_threads; ++i)
            workers.emplace_back([this]{
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        cv.wait(lock, [this]{ return stopped || !tasks.empty(); });
                        if (stopped && tasks.empty()) return;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    ++active_tasks;
                    task();
                    --active_tasks;
                    cv.notify_all();
                }
            });
        }
        ~ThreadPool() {
            {
                std::lock_guard lock(queue_mutex);
                stopped = true;
            }
            cv.notify_all();
            for (auto& worker : workers) {
                worker.join();
            }
        }

        template<typename F, typename... Args>
        auto submit(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
            using RetType = std::invoke_result_t<F, Args...>;
            auto task = std::make_shared<std::packaged_task<RetType()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...));
            auto fut = task->get_future();
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                tasks.push([task]() { (*task)(); });
            }
            cv.notify_one();
            return fut;
        }

        void wait_all() {
            std::unique_lock<std::mutex> lock(queue_mutex);
            cv.wait(lock, [this]{ return tasks.empty() && active_tasks == 0;});
        }

    };
}
