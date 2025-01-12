
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "NetIoWorker.h"
#include "Channel.h"

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

    std::vector<Channel *> m_needHandleFds;
    std::mutex m_needHandleFdsMutex;
    std::condition_variable m_needHandleFdsConVar;
    void AddNeedHandleFds(Channel *channel);
    void RemoveNeedHandleFds(Channel *channel);
    bool TryAddListenFd(Channel *channel);
public:
    NetIoManage(int workerNumber, int workerMaxFd);
    ~NetIoManage();
    void Init();
    void Run();
    void AddListenFd(Channel *channel);
    void JoinThreads();
};

#endif