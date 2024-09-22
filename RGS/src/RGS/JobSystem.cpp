#include "JobSystem.h"
#include "RGS/Config.h"

#include <algorithm>
#include <atomic>
#include <thread>
#include <condition_variable>

namespace RGS::JobSystem {

    static uint32_t s_NumThreads = 0;
    static ThreadSafeRingBuffer<std::function<void()>, 256> s_JobPool;    // a thread safe queue to put pending jobs onto the end (with a capacity of 256 jobs). A worker thread can grab a job from the beginning
    static std::condition_variable s_WakeCondition;    // used in conjunction with the wakeMutex below. Worker threads just sleep when there is no job, and the main thread can wake them up
    static std::mutex s_WakeMutex;    // used in conjunction with the wakeCondition above
    static uint64_t s_CurrentLabel = 0;
    static std::atomic<uint64_t> s_FinishedLabel;

    void Init()
    {
        // Initialize the worker execution state to 0:
        s_FinishedLabel.store(0);

        if constexpr (Config::LimitToSingleThread)
        {
            s_NumThreads = 1u;
        }
        else
        {
            // Retrieve the number of hardware threads in this system:
            // Calculate the actual number of worker threads we want:
            auto numCores = std::thread::hardware_concurrency();
            s_NumThreads = std::max(1u, numCores);
        }

        // Create all our worker threads while immediately starting them:
        for (uint32_t threadID = 0; threadID < s_NumThreads; ++threadID)
        {
            std::thread worker([] {

                std::function<void()> job; // the current job for the thread, it's empty at start.

                // This is the infinite loop that a worker thread will do 
                while (true)
                {
                    if (s_JobPool.pop_front(job)) // try to grab a job from the jobPool queue
                    {
                        // It found a job, execute it:
                        job(); // execute job
                        s_FinishedLabel.fetch_add(1); // update worker label state
                    }
                    else
                    {
                        // no job, put thread to sleep
                        std::unique_lock<std::mutex> lock(s_WakeMutex);
                        s_WakeCondition.wait(lock);
                    }
                }

                });

            // *****Here we could do platform specific thread setup...

            worker.detach(); // forget about this thread, let it do it's job in the infinite loop that we created above
        }
    }

    // This little helper function will not let the system to be deadlocked while the main thread is waiting for something
    static inline void poll()
    {
        s_WakeCondition.notify_one(); // wake one worker thread
        std::this_thread::yield(); // allow this thread to be rescheduled
    }

    void Execute(const std::function<void()>& job)
    {
        s_CurrentLabel += 1;

        while (!s_JobPool.push_back(job)) { poll(); }

        s_WakeCondition.notify_one();
    }

    bool IsBusy()
    {
        return s_FinishedLabel.load() < s_CurrentLabel;
    }

    void Wait()
    {
        while (IsBusy()) { poll(); }
    }

    void Dispatch(uint32_t jobCount,
                  uint32_t groupSize,
                  const std::function<void(JobDispatchArgs)>& job)
    {
        if (jobCount == 0 || groupSize == 0)
        {
            return;
        }

        // Calculate the amount of job groups to dispatch (overestimate, or "ceil"):
        const uint32_t groupCount = (jobCount + groupSize - 1) / groupSize;

        // The main thread label state is updated:
        s_CurrentLabel += groupCount;

        for (uint32_t groupIndex = 0; groupIndex < groupCount; ++groupIndex)
        {
            // For each group, generate one real job:
            auto jobGroup = [jobCount, groupSize, job, groupIndex]()
            {
                // Calculate the current group's offset into the jobs:
                const uint32_t groupJobOffset = groupIndex * groupSize;
                const uint32_t groupJobEnd = std::min(groupJobOffset + groupSize, jobCount);

                JobDispatchArgs args;
                args.GroupIndex = groupIndex;

                // Inside the group, loop through all job indices and execute job for each index:
                for (uint32_t i = groupJobOffset; i < groupJobEnd; ++i)
                {
                    args.JobIndex = i;
                    job(args);
                }
            };

            // Try to push a new job until it is pushed successfully:
            while (!s_JobPool.push_back(jobGroup)) { poll(); }

            s_WakeCondition.notify_one(); // wake one thread
        }
    }

}


