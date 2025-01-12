
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
}

void NetIoManage::Run()
{
    while (true) {
        {
            std::unique_lock<std::mutex> lock(this->m_needHandleFdsMutex);
            while (this->m_needHandleFds.empty()) {
                this->m_needHandleFdsConVar.wait(lock);
            }
            Channel *channel = this->m_needHandleFds.back(); // todo: 这里需要设计处理顺序，数据结构应该使用对应合适的
            if (this->TryAddListenFd(channel)) { // 如果return false 这里会一直循环，可以怎么优化？
                this->m_needHandleFds.pop_back();
                continue;
            }
        }
    }
}

bool NetIoManage::TryAddListenFd(Channel *channel)
{
    int k = this->m_netIOWorkers.size();
    while (k >= 0) {
        this->m_nextWorkerIndex = (++this->m_nextWorkerIndex) % this->m_netIOWorkers.size();
        if (this->m_netIOWorkers[this->m_nextWorkerIndex].AddListen(channel)) {
            return true;
        }
        --k;
    }
    return false;
}

void NetIoManage::AddListenFd(Channel *channel)
{   
    if(this->TryAddListenFd(channel)) {
        LOGGER.Log(INFO, "[NetIoManage]add listen fd. fd=" + std::to_string(channel->GetFd()));
        return;
    }
    if (channel != nullptr) {
        delete channel;
        channel = nullptr;
    }
}

void NetIoManage::AddNeedHandleFds(Channel *channel)
{
    std::unique_lock<std::mutex> lock(this->m_needHandleFdsMutex);
    this->m_needHandleFds.push_back(channel);
    this->m_needHandleFdsConVar.notify_one();
    LOGGER.Log(DEBUG, "[NetIoManage]put fd=" + std::to_string(channel->GetFd()) + " into next handle list");
}

void NetIoManage::RemoveNeedHandleFds(Channel *channel)
{
    std::unique_lock<std::mutex> lock(this->m_needHandleFdsMutex);
    auto it = std::remove_if(this->m_needHandleFds.begin(), this->m_needHandleFds.end(), [channel](Channel *i) { return i == channel;});
    this->m_needHandleFds.erase(it, this->m_needHandleFds.end());
}

void NetIoManage::JoinThreads()
{
    this->m_selfThread.join();
    for (auto &th : this->m_netIOWorkerThs) {
        th.join();
    }
}