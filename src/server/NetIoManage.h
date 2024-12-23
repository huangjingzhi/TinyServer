
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "NetIoWorker.h"
#include "Communicator.h"

#ifndef NETIOMANAGE_H
#define NETIOMANAGE_H

class NetIoManage
{
private:
    /* data */
    std::vector<NetIoWorker> m_netIOWorkers;
    std::vector<std::thread> m_netIOWorkerThs;
    int m_nextWorkerIndex;
    std::thread m_selfThread;

    std::vector<Communicator *> m_needHandleFds;
    std::mutex m_needHandleFdsMutex;
    std::condition_variable m_needHandleFdsConVar;
    void AddNeedHandleFds(Communicator *communicator);
    void RemoveNeedHandleFds(Communicator *communicator);
    bool TryAddListenFd(Communicator *communicator);
public:
    NetIoManage(int workerNumber, int workerMaxFd);
    ~NetIoManage();
    void Init();
    void Run();
    void AddListenFd(Communicator *communicator);
    void JoinThreads();
};

#endif