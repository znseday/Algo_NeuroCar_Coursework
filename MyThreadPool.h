#ifndef MYTHREADPOOL_H
#define MYTHREADPOOL_H

#include <queue>
#include <thread>
#include <condition_variable>
#include <functional>
#include <atomic>

using Task_t = std::tuple<std::function<void(size_t, size_t)>, size_t, size_t>;


class MyThreadPool
{
private:

    bool IsInited = false;

    std::vector<std::thread*> Threads;

    std::condition_variable cv;
    std::mutex cvMutex, toWaitMutex, consoleMutex;

    std::atomic_bool ItsTimeToRelease = false;

    std::condition_variable cvWaitAll;

    std::atomic_int CountTasksToDo = 0;

    //void MainLoop(std::queue<Task_t> &_tasks);

    std::queue<Task_t> Tasks;

public:

    void SetCountTasksToDo(int _count) {CountTasksToDo = _count;}

    //std::mutex & cvMutexAccess() {return cvMutex;}

    template<typename ...Args>
    void EmplaceTask(Args&& ...args)
    {
        std::lock_guard<std::mutex> lk(cvMutex);
        Tasks.emplace(std::forward<Args>(args)...);
    }

    friend void MainLoop(MyThreadPool &_threadPool); // Можно ли как-то сделать, чтобы MainLoop был мембером?

    MyThreadPool() = default;

    void Init(size_t _threadCount);
    void FinishAndRelease();

    void NotifyToDoAllTasks();
    void WaitForAllTasksCompleted();

    size_t GetThreadCount() const {return Threads.size();}

    ~MyThreadPool();
};

#endif // MYTHREADPOOL_H
