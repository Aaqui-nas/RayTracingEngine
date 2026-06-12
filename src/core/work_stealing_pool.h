#pragma once

#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>

namespace rt {

class WorkStealingPool {
    struct Worker {
        std::deque<std::function<void()>> queue;
        std::mutex mutex;
    };

    std::vector<Worker> workers;
    std::vector<std::thread> threads;
    std::atomic<bool> stopped{false};
    std::atomic<int>  pending{0};
    std::mutex        cv_mutex;
    std::condition_variable cv;
    std::atomic<int>  next_worker{0};

    void worker_loop(int id) {
        while (!stopped) {
            std::function<void()> task;

            {
                std::lock_guard lock(workers[id].mutex);
                if (!workers[id].queue.empty()) {
                    task = std::move(workers[id].queue.back());
                    workers[id].queue.pop_back();
                }
            }

            if (!task) {
                for (int i = 1; i < (int)workers.size() && !task; i++) {
                    int target = (id + i) % (int)workers.size();
                    std::unique_lock lock(workers[target].mutex, std::try_to_lock);
                    if (lock && !workers[target].queue.empty()) {
                        task = std::move(workers[target].queue.front());
                        workers[target].queue.pop_front();
                    }
                }
            }

            if (task) {
                task();
                {
                    std::lock_guard lock(cv_mutex);
                    --pending;
                }
                cv.notify_all();
            } else {
                std::unique_lock lock(cv_mutex);
                cv.wait_for(lock, std::chrono::microseconds(50),
                    [this]{ return stopped.load() || pending.load() > 0; });
            }
        }
    }

public:
    explicit WorkStealingPool(std::size_t num_threads) : workers(num_threads) {
        threads.reserve(num_threads);
        for (int i = 0; i < (int)num_threads; i++)
            threads.emplace_back(&WorkStealingPool::worker_loop, this, i);
    }

    ~WorkStealingPool() {
        stopped = true;
        cv.notify_all();
        for (auto& t : threads) t.join();
    }

    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
        using RetType = std::invoke_result_t<F, Args...>;
        auto task = std::make_shared<std::packaged_task<RetType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        auto fut = task->get_future();

        int target = next_worker++ % (int)workers.size();
        {
            std::lock_guard lock(cv_mutex);
            ++pending;
        }
        {
            std::lock_guard lock(workers[target].mutex);
            workers[target].queue.push_back([task]{ (*task)(); });
        }
        cv.notify_one();
        return fut;
    }

    void wait_all() {
        std::unique_lock lock(cv_mutex);
        cv.wait(lock, [this]{ return pending.load() == 0; });
    }
};

}
