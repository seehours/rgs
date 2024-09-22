#pragma once
#include "RGS/Config.h"

#include <sstream>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <string>
#include <thread>
#include <mutex>
#include <iostream>
#include <queue>
#include <condition_variable>
#include <future>

namespace RGS {

    class ThreadPool {
    public:
        ThreadPool(size_t threadCount) 
        {
            for (size_t i = 0; i < threadCount; ++i)
            {
                m_Workers.emplace_back([this]()
                {
                    for (;;) 
                    {
                        std::function<void()> task;

                        {
                            std::unique_lock<std::mutex> lock(queueMutex);
                            condition.wait(lock, [this] { return !m_Tasks.empty() || terminate; });
                            if (terminate && m_Tasks.empty())
                                return;
                            task = std::move(m_Tasks.front());
                            m_Tasks.pop();
                        }

                        task();
                    }
                });
            }
        }

        ~ThreadPool() 
        {
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                terminate = true;
            }
            condition.notify_all();
            for (std::thread& worker : m_Workers)
                worker.join();
        }

        template<typename F, typename... Args>
        auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> 
        {
            using return_type = typename std::result_of<F(Args...)>::type;

            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );

            std::future<return_type> res = task->get_future();
            {
                std::unique_lock<std::mutex> lock(queueMutex);

                if (terminate)
                    throw std::runtime_error("enqueue on stopped ThreadPool");

                m_Tasks.emplace([task]() { (*task)(); });
            }
            condition.notify_one();
            return res;
        }

    private:
        std::vector<std::thread> m_Workers;
        std::queue<std::function<void()>> m_Tasks;

        std::mutex queueMutex;
        std::condition_variable condition;
        bool terminate = false;
    };


    using FloatingPointMicroseconds = std::chrono::duration<double, std::micro>;

    struct ProfileResult {
        std::string Name;
        FloatingPointMicroseconds Start;
        std::chrono::microseconds ElapsedTime;
       // std::thread::id ThreadID;
    };

    struct InstrumentationSession {
        std::string Name;
    };

    class Instrumentor
    {
    public:
        Instrumentor(const Instrumentor&) = delete;
        Instrumentor(Instrumentor&&) = delete;

        void BeginSession(const std::string& name, const std::string& filepath = "results.json")
        {
            std::lock_guard lock(m_Mutex);
            if (m_CurrentSession)
            {
                InternalEndSession();
            }
            m_OutputStream.open(filepath);

            if (m_OutputStream.is_open()) 
            {
                m_CurrentSession = new InstrumentationSession({ name });
                WriteHeader();
            }
            else 
            {
                std::cout << "Instrumentor could not open results file.";
            }
        }

        void EndSession() 
        {
            std::lock_guard lock(m_Mutex);
            InternalEndSession();
        }

        void WriteProfileImpl(const ProfileResult& result) 
        {
            std::stringstream json;

            json << std::setprecision(3) << std::fixed;
            json << ",{";
            json << "\"cat\":\"function\",";
            json << "\"dur\":" << (result.ElapsedTime.count()) << ',';
            json << "\"name\":\"" << result.Name << "\",";
            json << "\"ph\":\"X\",";
            json << "\"pid\":0,";
            json << "\"tid\": 0,";  // << result.ThreadID << ",";
            json << "\"ts\":" << result.Start.count();
            json << "}";

            std::lock_guard lock(m_Mutex);
            if (m_CurrentSession) 
            {
                m_OutputStream << json.str();
                m_OutputStream.flush();
            }
        }

        void WriteProfile(const ProfileResult& result)
        {
            m_ThreadPool.enqueue(std::bind(&Instrumentor::WriteProfileImpl, this, result));
        }

        static Instrumentor& Instance() 
        {
            static Instrumentor instance;
            return instance;
        }

    private:
        Instrumentor()
            : m_CurrentSession(nullptr), m_ThreadPool(std::thread::hardware_concurrency()) {}

        ~Instrumentor() 
        {
            EndSession();
        }

        void WriteHeader()
        {
            m_OutputStream << "{\"otherData\": {},\"traceEvents\":[{}";
            m_OutputStream.flush();
        }

        void WriteFooter() 
        {
            m_OutputStream << "]}";
            m_OutputStream.flush();
        }

        void InternalEndSession() 
        {
            if (m_CurrentSession) 
            {
                WriteFooter();
                m_OutputStream.close();
                delete m_CurrentSession;
                m_CurrentSession = nullptr;
            }
        }

    private:
        std::mutex m_Mutex;
        InstrumentationSession* m_CurrentSession;
        std::ofstream m_OutputStream;
        ThreadPool m_ThreadPool;  // ThreadPool member for handling tasks
    };

    class InstrumentationTimer
    {
    public:
        InstrumentationTimer(const char* name)
            : m_Name(name) 
        {
            m_StartTimepoint = std::chrono::steady_clock::now();
        }

        ~InstrumentationTimer() 
        {
            auto endTimepoint = std::chrono::steady_clock::now();
            auto highResStart = FloatingPointMicroseconds{ m_StartTimepoint.time_since_epoch() };
            auto elapsedTime = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch() -
                std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch();

            Instrumentor::Instance().WriteProfile({ m_Name, highResStart, elapsedTime});
        }

    private:
        const char* m_Name;
        std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
    };

    namespace InstrumentorUtils {

        template <size_t N>
        struct ChangeResult
        {
            char Data[N];
        };

        template <size_t N, size_t K>
        constexpr auto CleanupOutputString(const char(&expr)[N], const char(&remove)[K])
        {
            ChangeResult<N> result = {};

            size_t srcIndex = 0;
            size_t dstIndex = 0;
            while (srcIndex < N)
            {
                size_t matchIndex = 0;
                while (matchIndex < K - 1 && srcIndex + matchIndex < N - 1 && expr[srcIndex + matchIndex] == remove[matchIndex])
                    matchIndex++;
                if (matchIndex == K - 1)
                    srcIndex += matchIndex;
                result.Data[dstIndex++] = expr[srcIndex] == '"' ? '\'' : expr[srcIndex];
                srcIndex++;
            }

            return result;
        }
    }
}

#if RGS_PROFILE
// Resolve which function signature macro will be used. Note that this only
// is resolved when the (pre)compiler starts, so the syntax highlighting
// could mark the wrong one in your editor!

#if defined(__FUNCSIG__) || (_MSC_VER)
#define RGS_FUNC_SIG __FUNCSIG__
#else
#define RGS_FUNC_SIG "RGS_FUNC_SIG unknown!"
#endif

#define RGS_PROFILE_BEGIN_SESSION(name, filepath) ::RGS::Instrumentor::Instance().BeginSession(name, filepath)
#define RGS_PROFILE_END_SESSION() ::RGS::Instrumentor::Instance().EndSession()
#define RGS_PROFILE_SCOPE_LINE2(name, line) constexpr auto fixedName##line = ::RGS::InstrumentorUtils::CleanupOutputString(name, "__cdecl ");\
                                                   ::RGS::InstrumentationTimer timer##line(fixedName##line.Data)
#define RGS_PROFILE_SCOPE_LINE(name, line) RGS_PROFILE_SCOPE_LINE2(name, line)
#define RGS_PROFILE_SCOPE(name) RGS_PROFILE_SCOPE_LINE(name, __LINE__)
#define RGS_PROFILE_FUNCTION() RGS_PROFILE_SCOPE(RGS_FUNC_SIG)
#else
#define RGS_PROFILE_BEGIN_SESSION(name, filepath)
#define RGS_PROFILE_END_SESSION()
#define RGS_PROFILE_SCOPE(name)
#define RGS_PROFILE_FUNCTION()
#endif