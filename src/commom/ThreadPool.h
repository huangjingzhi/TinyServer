#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <queue>
#include <type_traits>
#include <utility>
#include <vector>

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

class ThreadsGuard
{
public:
    ThreadsGuard(std::vector<std::thread>& v)
        : m_threads(v)
    {
        
    }

    ~ThreadsGuard()
    {
        for (size_t i = 0; i != m_threads.size(); ++i)
        {
            if (m_threads[i].joinable())
            {
                m_threads[i].join();
            }
        }
    }
private:
    ThreadsGuard(ThreadsGuard&& tg) = delete;
    ThreadsGuard& operator = (ThreadsGuard&& tg) = delete;

    ThreadsGuard(const ThreadsGuard&) = delete;
    ThreadsGuard& operator = (const ThreadsGuard&) = delete;
private:
    std::vector<std::thread>& m_threads;
};


class ThreadPool
{
public:
    typedef std::function<void()> task_type;

public:
    explicit ThreadPool(int n = 0);

    ~ThreadPool()
    {
        stop();
        m_condition.notify_all();
    }

    void stop()
    {
        m_stop.store(true, std::memory_order_release);
    }

    template<class Function, class... Args>
    std::future<typename std::result_of<Function(Args...)>::type> add(Function&&, Args&&...);

private:
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator = (ThreadPool&&) = delete;
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator = (const ThreadPool&) = delete;

private:
    std::atomic<bool> m_stop;
    std::mutex m_mutex;
    std::condition_variable m_condition;

    std::queue<task_type> m_tasks;
    std::vector<std::thread> m_threads;
    ThreadsGuard m_tg;
};


inline ThreadPool::ThreadPool(int n)
    : m_stop(false)
    , m_tg(m_threads)
{
    int nthreads = n;
    if (nthreads <= 0)
    {
        nthreads = std::thread::hardware_concurrency();
        nthreads = (nthreads == 0 ? 2 : nthreads);
    }

    for (int i = 0; i != nthreads; ++i)
    {
        m_threads.push_back(std::thread([this]{
            while (!m_stop.load(std::memory_order_acquire))
            {
                task_type task;
                {
                    std::unique_lock<std::mutex> ulk(this->m_mutex);
                    this->m_condition.wait(ulk, [this]{ return m_stop.load(std::memory_order_acquire) || !this->m_tasks.empty(); });
                    if (m_stop.load(std::memory_order_acquire))
                        return;
                    task = std::move(this->m_tasks.front());
                    this->m_tasks.pop();
                }
                task();
            }
        }));
    }
}

template<class Function, class... Args>
std::future<typename std::result_of<Function(Args...)>::type>
    ThreadPool::add(Function&& fcn, Args&&... args)
{
    typedef typename std::result_of<Function(Args...)>::type return_type;
    typedef std::packaged_task<return_type()> task;

    auto t = std::make_shared<task>(std::bind(std::forward<Function>(fcn), std::forward<Args>(args)...));
    auto ret = t->get_future();
    {
        std::lock_guard<std::mutex> lg(m_mutex);
        if (m_stop.load(std::memory_order_acquire))
            throw std::runtime_error("thread pool has stopped");
        m_tasks.emplace([t]{(*t)(); });
    }
    m_condition.notify_one();
    return ret;
}

#endif