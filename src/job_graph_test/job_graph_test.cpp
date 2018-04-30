#ifndef JOB_GRAPH_H
#define JOB_GRAPH_H

#include "job_worker.h"

class JobManager
{
public:
    JobManager(size_t nWorkers)
    {
        for(size_t i = 0; i < nWorkers; ++i)
        {
            _workers.emplace_back(
                new JobWorker(this)
            );
            _threads.emplace_back([this, i](){
                _workers[i]->Run();
            });
        }
    }
    ~JobManager()
    {
        for(auto& w : _workers)
        {
            w->Stop();
        }
        for(auto& t : _threads)
        {
            t.join();
        }
    }
    JobManager& Submit(Job* j)
    {
        _queue.enqueue(j);
        return *this;
    }
    void Wait(Job* j){}

    Job* GetJob(std::thread::id thread)
    {
        Job* j = 0;
        _queue.try_dequeue(j);
        if(!j) return 0;
        if(!j->ReadyToRun() ||
        (j->GetAffinity() != std::thread::id() && j->GetAffinity() != thread))
        {
            _queue.enqueue(j);
            return 0;
        }
        return j;
    }
private:
    moodycamel::ConcurrentQueue<Job*> _queue;
    std::vector<JobWorker*> _workers;
    std::vector<std::thread> _threads;
};

Job* JobWorker::GetJob()
{
    return _manager->GetJob(std::this_thread::get_id());
}

#include <set>

class FrameGraph
{
public:
    FrameGraph(size_t nWorkers){

    }

    Job* CreateJob(job_fn_t task, Job* parent = 0, int threadAffinity = 0){
        Job* j = new Job(task, parent);
        _jobs.insert(j);
        return j;
    }
    void DestroyJob(Job* j){
        _jobs.erase(j);
        delete j;
    }
private:
    std::set<Job*> _jobs;
};

#define THREAD_AFFINITY_MAIN 1

#endif

void test_a();
void test_b();

int main()
{
    test_a();

    std::getchar();
    return 0;
}

void test_b()
{
    FrameGraph frameGraph(std::thread::hardware_concurrency() - 1);

    Job* job_frameEnd = frameGraph.CreateJob([](Job& j)
    {
        std::cout << "frameEnd" << std::endl;
    }, 0, THREAD_AFFINITY_MAIN);
    
    Job* job_drawStage = frameGraph.CreateJob([](Job& j)
    {
        std::cout << "DrawStage" << std::endl;
    }, job_frameEnd, THREAD_AFFINITY_MAIN);

    Job* job_frameStart = frameGraph.CreateJob([](Job& j)
    {
        std::cout << "frameStart" << std::endl;
    }, 0, THREAD_AFFINITY_MAIN);
    
    frameGraph.CreateJob([](Job& j)
    {
        std::cout << "Process object" << std::endl;
    });    
}

void test_a()
{
    {
        ScopedTimer timer("JobsDone!");
        Job job_end([](Job& j){
            //std::cout << "RootJob" << std::endl;
        });

        /*
        std::string mystring = "Hello, World!";
        Job job([](Job& job){
            std::string* str = job.GetData<std::string*>();
            //std::cout << str->c_str() << std::endl;
        }, &job_end);
        job.SetData(&mystring);

        Job some_random_job([](Job& job){
            //std::cout << "SomeRandomJob" << std::endl;
        }, &job_end);
        */
        std::cout << "nCores: " << std::thread::hardware_concurrency() << std::endl;
        JobManager jobManager(std::thread::hardware_concurrency() - 1);
        std::cout << "threadId: " << std::this_thread::get_id() << std::endl;
        //jobManager.Submit(&job).Submit(&job_end).Submit(&some_random_job);
        jobManager.Submit(&job_end);
        std::vector<Job*> jobs(500);
        for(unsigned i = 0; i < jobs.size(); ++i)
        {
            jobs[i] = new Job([](Job& j)
            { 
                std::cout << std::this_thread::get_id() << std::endl; 
            }, &job_end, 0, std::this_thread::get_id());
        }
        for(unsigned i = 0; i < jobs.size(); ++i)
        {
            jobManager.Submit(jobs[i]);
        }
        JobWorker worker(&jobManager);
        worker.Wait(&job_end);
    }
}