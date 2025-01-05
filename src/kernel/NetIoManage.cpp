
#include "NetIoManage.h"
#include <thread>
#include <mutex>
#include <algorithm>
#include  <unistd.h>
#include "../commom/Logger.h"

NetIoManage::NetIoManage(int workerNumber, int workerMaxFd): m_netIOWorkers(workerNumber, NetIoWorker(workerMaxFd)),
    m_netIOWorkerThs(workerNumber),
    m_nextWorkerIndex(0)
{

}

NetIoManage::~NetIoManage()
{
}

void NetIoManage::Init()
{
    for (size_t i = 0; i < this->m_netIOWorkers.size(); i++) {
        this->m_netIOWorkerThs[i] = std::thread(&NetIoWorker::Working, &m_netIOWorkers[i]); // 是否应该放在NetIoWorker里面进行创建线程
    }
    this->m_selfThread = std::thread(&NetIoManage::Run, this);
}

void NetIoManage::Run()
{
    // 这里将accept的fd进行缓存，是否会造成对端发了消息，但是但是当前还没有及时处理，从而丢失数据。
    /* Run 和 AddNeedHandleFds 会不会竞争导致死锁，this->m_needHandleFdsConVar.wait的时候，是Run持续持有锁的*/
    while (true) {
        {
            std::unique_lock<std::mutex> lock(this->m_needHandleFdsMutex);
            while (this->m_needHandleFds.empty()) {
                this->m_needHandleFdsConVar.wait(lock);
            }
            Communicator *communicator = this->m_needHandleFds.back(); // todo: 这里需要设计处理顺序，数据结构应该使用对应合适的
            if (this->TryAddListenFd(communicator)) { // 如果return false 这里会一直循环，可以怎么优化？
                this->m_needHandleFds.pop_back();
                continue;
            }
        }
    }
}

bool NetIoManage::TryAddListenFd(Communicator *communicator)
{
    int k = this->m_netIOWorkers.size();
    while (k >= 0) {
        this->m_nextWorkerIndex = (++this->m_nextWorkerIndex) % this->m_netIOWorkers.size();
        if (this->m_netIOWorkers[this->m_nextWorkerIndex].AddListen(communicator)) {
            return true;
        }
        --k;
    }
    return false;
}

void NetIoManage::AddListenFd(Communicator *communicator)
{   
    if(this->TryAddListenFd(communicator)) {
        LOGGER.Log(INFO, "[NetIoManage]add listen fd. fd=" + std::to_string(communicator->GetFd()));
        return;
    }
    this->AddNeedHandleFds(communicator);
}

void NetIoManage::AddNeedHandleFds(Communicator *communicator)
{
    std::unique_lock<std::mutex> lock(this->m_needHandleFdsMutex);
    this->m_needHandleFds.push_back(communicator);
    this->m_needHandleFdsConVar.notify_one();
    LOGGER.Log(DEBUG, "[NetIoManage]put fd=" + std::to_string(communicator->GetFd()) + " into next handle list");
}

void NetIoManage::RemoveNeedHandleFds(Communicator *communicator)
{
    std::unique_lock<std::mutex> lock(this->m_needHandleFdsMutex);
    auto it = std::remove_if(this->m_needHandleFds.begin(), this->m_needHandleFds.end(), [communicator](Communicator *i) { return i == communicator;});
    this->m_needHandleFds.erase(it, this->m_needHandleFds.end());
}

void NetIoManage::JoinThreads()
{
    this->m_selfThread.join();
    for (auto &th : this->m_netIOWorkerThs) {
        th.join();
    }
}