#include "MyThreadPool.h"

#include <iostream>


void MainLoop(MyThreadPool &_threadPool)
{
    _threadPool.consoleMutex.lock();
    std::cout << "Thread " << std::this_thread::get_id() << " just started" << std::endl;
    _threadPool.consoleMutex.unlock();

    while (!_threadPool.ItsTimeToRelease)
    {
        std::unique_lock<std::mutex> lk(_threadPool.cvMutex);

        if (_threadPool.CountTasksToDo <= 0)
                _threadPool.cvWaitAll.notify_all();

        _threadPool.cv.wait(lk, [&_threadPool]() { return !_threadPool.Tasks.empty() ||
                                                           _threadPool.ItsTimeToRelease
                                                          /*|| _threadPool.CountTasksToDo > 0*/; });

        if (!_threadPool.Tasks.empty())
        {
            auto [f, s, e] = std::move(_threadPool.Tasks.front());
            _threadPool.Tasks.pop();

            lk.unlock();

            f(s, e);

            _threadPool.CountTasksToDo--;
//            if (_threadPool.CountTasksToDo <= 0)
//                    _threadPool.cv_wait_all.notify_all();
        }
        else
        {
//            if (_threadPool.CountTasksToDo <= 0)
//                    _threadPool.cv_wait_all.notify_all();
        }
    }

    _threadPool.consoleMutex.lock();
    std::cout << "Thread " << std::this_thread::get_id() << " just finished" << std::endl;
    _threadPool.consoleMutex.unlock();
}
//-------------------------------------------------------------
//-------------------------------------------------------------

void MyThreadPool::Init(size_t _threadCount)
{
    if (IsInited)
    {
        std::cout << "MyThreadPool is already inited " << std::endl;
        std::cout << "MyThreadPool is releasing..." << std::endl;
        FinishAndRelease();
        std::cout << "MyThreadPool released" << std::endl;
    }

    IsInited = true;
    ItsTimeToRelease = false;

    Threads.resize(_threadCount);

    std::cout << "Threads starting..." << std::endl;

    for (auto & t : Threads)
    {
        t = new std::thread(MainLoop, std::ref(*this));
    }
}
//-------------------------------------------------------------

void MyThreadPool::FinishAndRelease()
{
    if (!IsInited)
    {
        std::cout << "MyThreadPool is already released" << std::endl;
        return;
    }

    ItsTimeToRelease = true;
    cv.notify_all();

    for (auto & t : Threads)
    {
        t->join();
    }

    for (auto & t : Threads)
    {
        delete t;
    }

    Threads.clear();

    IsInited = false;
}
//-------------------------------------------------------------

void MyThreadPool::NotifyToDoAllTasks()
{
    if (!IsInited)
        throw std::runtime_error("MyThreadPool is not inited to to anything!");

    cv.notify_all();
}
//-------------------------------------------------------------

void MyThreadPool::WaitForAllTasksCompleted()
{
    std::unique_lock<std::mutex> lk(toWaitMutex); // Использовать cvMutex или новый мьютекс?

    cvWaitAll.wait(lk, [this]()
    {
        return CountTasksToDo <= 0;
    });
}
//-------------------------------------------------------------

MyThreadPool::~MyThreadPool()
{
    FinishAndRelease();
}
//-------------------------------------------------------------


