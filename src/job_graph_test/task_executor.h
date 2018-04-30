#ifndef TASK_EXECUTOR_H
#define TASK_EXECUTOR_H

#include <vector>
#include <mutex>
#include <thread>
#include <queue>

typedef void(*task_fn_t)(void);

class TaskExecutor
{
public:
    TaskExecutor(size_t nThreads);
    ~TaskExecutor();
    void Enqueue(task_fn_t t);
private:
    std::vector<std::thread> _threads;
    std::queue<task_fn_t> _tasks;

    std::mutex _sync_queue;
    std::condition_variable _condition;
    bool _stop;
};

inline TaskExecutor::TaskExecutor(size_t nThreads)
: _stop(false)
{
    for(size_t i = 0; i < nThreads; ++i)
    {
        _threads.emplace_back([this]{
            for(;;)
            {
                task_fn_t task;
                {
                    std::unique_lock<std::mutex> lock(_sync_queue);
                    _condition.wait(lock, [this]{
                        return _stop || !_tasks.empty();
                    });
                    if(_stop && _tasks.empty())
                        return;
                    task = _tasks.front();
                    _tasks.pop();
                }
                task();
            }
        });
    }
}

inline TaskExecutor::~TaskExecutor()
{
    {
        std::unique_lock<std::mutex> lock(_sync_queue);
        _stop = true;
    }
    _condition.notify_all();
    for(std::thread& w : _threads)
    {
        w.join();
    }
}

void TaskExecutor::Enqueue(task_fn_t task)
{
    {
        std::unique_lock<std::mutex> lock(_sync_queue);
        if(_stop)
            return;
        _tasks.emplace(task);
    }
    _condition.notify_one();
}

#endif
