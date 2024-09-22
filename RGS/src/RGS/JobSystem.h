#pragma once
#include <functional>
#include <mutex>

namespace RGS::JobSystem {

    struct JobDispatchArgs
    {
        // The index of the current job being executed.
        uint32_t JobIndex;
        // The index of the group this job belongs to.
        uint32_t GroupIndex;
    };

    // Initializes internal resources such as worker threads. 
    // This function should be called once at the start of the application.
    void Init();

    // Adds a job to the job queue for asynchronous execution. 
    // Any available idle thread will pick up and execute this job.
    void Execute(const std::function<void()>& job);

    /**
    * @brief Divides jobs into groups and dispatches them across threads for asynchronous execution.
    * @param jobCount    The total number of jobs to be processed.
    * @param groupSize   The number of jobs each thread will process in a group.
    * @param job         A function that takes JobDispatchArgs and defines the job to be executed.
    */
    void Dispatch(uint32_t jobCount, uint32_t groupSize, const std::function<void(JobDispatchArgs)>& job);

    // Checks whether any threads are currently busy processing jobs.
    bool IsBusy();

    // Blocks the calling thread until all worker threads have completed their jobs and become idle.
    void Wait();

    template<typename T, size_t capacity>
    class ThreadSafeRingBuffer
    {
    public:
        // Pushes an item to the end of the buffer if there is free space.
        // Returns true if the item was successfully added.
        // Returns false if the buffer is full and there isn't enough space.
        inline bool push_back(const T& item)
        {
            bool result = false;
            lock.lock();
            size_t next = (head + 1) % capacity;
            if (next != tail)
            {
                data[head] = item;
                head = next;
                result = true;
            }
            lock.unlock();
            return result;
        }

        // Retrieves and removes an item from the front of the buffer if available.
        // Returns true if an item was successfully retrieved.
        // Returns false if the buffer is empty and there are no items to retrieve.
        inline bool pop_front(T& item)
        {
            bool result = false;
            lock.lock();
            if (tail != head)
            {
                item = data[tail];
                tail = (tail + 1) % capacity;
                result = true;
            }
            lock.unlock();
            return result;
        }

    private:
        // Array to store the items in the buffer.
        T data[capacity];
        // Index pointing to the head of the buffer (where new items are added).
        size_t head = 0;
        // Index pointing to the tail of the buffer (where items are retrieved).
        size_t tail = 0;
        // Mutex for ensuring thread-safe access to the buffer.
        std::mutex lock;

    };

}
