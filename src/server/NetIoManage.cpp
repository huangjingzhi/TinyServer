
#include "NetIoManage.h"
#include <thread>
#include <mutex>
#include <algorithm>

NetIoManage::NetIoManage(int workerNumber): m_netIOWorkers(workerNumber),
    m_netIOWorkerThs(workerNumber),
    m_nextWorkerIndex(0)
{

}

NetIoManage::~NetIoManage()
{
}

void NetIoManage::Init()
{
    for (int i = 0; i < this->m_netIOWorkers.size(); i++) {
        this->m_netIOWorkerThs[i] = std::thread(NetIoWorker::Working, &m_netIOWorkers[i]); // 是否应该放在NetIoWorker里面进行创建线程
    }
    this->m_selfThread = std::thread(NetIoManage::Run, this);
    
}

void NetIoManage::Run()
{
    /* Run 和 AddNeedHandleFds 会不会竞争导致死锁，this->m_needHandleFdsConVar.wait的时候，是Run持续持有锁的*/
    while (true) {
        {
            std::unique_lock<std::mutex> lock(this->m_needHandleFdsMutex);
            while (this->m_needHandleFds.empty()) {
                this->m_needHandleFdsConVar.wait(lock);
            }
            int fd = this->m_needHandleFds.back(); // todo: 这里需要设计处理顺序，数据结构应该使用对应合适的
            if (this->TryAddListenFd(fd)) {
                this->m_needHandleFds.pop_back();
                continue;
            }
        }
    }
}

bool NetIoManage::TryAddListenFd(int fd)
{
    int k = this->m_netIOWorkers.size();
    while (k >= 0) {
        this->m_nextWorkerIndex = (++this->m_nextWorkerIndex) % this->m_netIOWorkers.size();
        if (this->m_netIOWorkers[this->m_nextWorkerIndex].AddListen(fd)) {
            return true;
        }
        --k;
    }
    return false;
}

void NetIoManage::AddListenFd(int fd)
{   
    if(this->TryAddListenFd(fd)) {
        return;
    }
    this->AddNeedHandleFds(fd);
}

void NetIoManage::AddNeedHandleFds(int fd)
{
    std::unique_lock<std::mutex> lock(this->m_needHandleFdsMutex);
    this->m_needHandleFds.push_back(fd);
    this->m_needHandleFdsConVar.notify_one();
}

void NetIoManage::RemoveNeedHandleFds(int fd)
{
    std::unique_lock<std::mutex> lock(this->m_needHandleFdsMutex);
    auto it = std::remove_if(this->m_needHandleFds.begin(), this->m_needHandleFds.end(), [fd](int i) { return i == fd;});
    this->m_needHandleFds.erase(it, this->m_needHandleFds.end());
}

void NetIoManage::JoinThreads()
{
    this->m_selfThread.join();
    for (auto &th : this->m_netIOWorkerThs) {
        th.join();
    }

}