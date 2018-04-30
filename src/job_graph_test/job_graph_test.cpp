#ifndef JOB_GRAPH_H
#define JOB_GRAPH_H

#include "job_manager.h"

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

#define WORKER_MAIN_THREAD 1
#define WORKER_DEFAULT 2

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
    }, 0, WORKER_MAIN_THREAD);
    
    Job* job_drawStage = frameGraph.CreateJob([](Job& j)
    {
        std::cout << "DrawStage" << std::endl;
    }, job_frameEnd, WORKER_MAIN_THREAD);

    Job* job_frameStart = frameGraph.CreateJob([](Job& j)
    {
        std::cout << "frameStart" << std::endl;
    }, 0, WORKER_MAIN_THREAD);
    
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
            std::cout << "RootJob" << std::endl;
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

        Job required_job([](Job& job){
            std::cout << "Required job done" << std::endl;
        });

        std::cout << "nCores: " << std::thread::hardware_concurrency() << std::endl;
        JobManager jobManager(std::thread::hardware_concurrency() - 1, WORKER_DEFAULT);
        std::cout << "threadId: " << std::this_thread::get_id() << std::endl;
        //jobManager.Submit(&job).Submit(&job_end).Submit(&some_random_job);
        std::vector<Job*> jobs(500);
        for(unsigned i = 0; i < jobs.size(); ++i)
        {
            jobs[i] = new Job([](Job& j)
            { 
                std::cout << "test" << std::endl;
            }, &job_end, &required_job, WORKER_MAIN_THREAD);
        }
        for(unsigned i = 0; i < jobs.size(); ++i)
        {
            jobManager.Submit(jobs[i]);
        }
        jobManager.Submit(&job_end);
        jobManager.Submit(&required_job);
        JobWorker worker(&jobManager, WORKER_MAIN_THREAD);
        worker.Wait(&job_end);
    }
}