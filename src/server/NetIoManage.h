
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "NetIoWorker.h"

class NetIoManage
{
private:
    /* data */
    std::vector<NetIoWorker> m_netIOWorkers;
    std::vector<std::thread> m_netIOWorkerThs;
    int m_nextWorkerIndex;
    std::thread m_selfThread;

    std::vector<int> m_needHandleFds;
    std::mutex m_needHandleFdsMutex;
    std::condition_variable m_needHandleFdsConVar;
    void AddNeedHandleFds(int fd);
    void RemoveNeedHandleFds(int fd);
    bool TryAddListenFd(int fd);
public:
    NetIoManage(int workerNumber);
    ~NetIoManage();
    void Init();
    void Run();
    void AddListenFd(int fd);
    void JoinThreads();
};

