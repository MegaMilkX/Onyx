#ifndef JOB_WORKER_H
#define JOB_WORKER_H

#include "job.h"
#include <util/scoped_timer.h>
#include <lib/concurrentqueue.h>

class critical_section
{
public:
    critical_section() { InitializeCriticalSection(&m_cs); }
    ~critical_section() { DeleteCriticalSection(&m_cs); }

    void lock() { EnterCriticalSection(&m_cs); }
    void unlock() { LeaveCriticalSection(&m_cs); }

private:
    CRITICAL_SECTION m_cs;
};

class JobManager;
class JobWorker
{
public:
    JobWorker() : _running(false), _manager(0) {}
    JobWorker(JobManager* man)
    : _running(false), _manager(man)
    {

    }
    ~JobWorker()
    {

    }

    void Submit(Job* job) {

    }

    void Run() {
        _running = true;
        while(_running)
            RunOne();
    }
    void Stop() {
        _running = false;
    }

    void Wait(Job* waitFor) {
        while(!waitFor->Finished())
            RunOne();
    }
private:
    void RunOne()
    {
        Job* j = GetJob();
        if(j != 0)
        {
            j->Run();
        }
    }
    Job* GetJob();

    moodycamel::ConcurrentQueue<Job*> _queue;
    JobManager* _manager;
    std::atomic<bool> _running;
};

#endif
