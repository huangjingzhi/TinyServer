#include "NetIoWorker.h"
#include <algorithm>
#include <string.h>
#include  <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include "Logger.h"
#include <functional>
#include "TagTimer.h"

NetIoWorker::NetIoWorker(int maxFds):M_MAX_FDS(maxFds), m_epollFd(-1), m_isWorking(false), m_timer(60, 1)
{
}

NetIoWorker::NetIoWorker(const NetIoWorker&netIoWorker)
{
    this->M_MAX_FDS =  netIoWorker.M_MAX_FDS;
    this->m_epollFd = -1;
    this->m_isWorking = false;
    this->m_timer = netIoWorker.m_timer;
}

NetIoWorker::~NetIoWorker() 
{
    for (Communicator *communicator: this->m_listenFds) {
        this->DestroyCommunicator(communicator);
        communicator = nullptr;
    }
    close(this->m_epollFd);
}

void NetIoWorker::DestroyCommunicator(Communicator *communicator)
{
    if (communicator == nullptr) {
        return;
    }
    struct epoll_event event;
    event.data.fd = communicator->GetFd();
    epoll_ctl(this->m_epollFd, EPOLL_CTL_DEL, communicator->GetFd(), &event);
    {
        std::unique_lock<std::mutex> lock(this->m_listenFdsMutex);
        auto it = std::remove_if(this->m_listenFds.begin(), this->m_listenFds.end(), [communicator](Communicator *i) { return i == communicator;});
        this->m_listenFds.erase(it, this->m_listenFds.end());
    }

    this->DleteTimer(communicator->GetFd());
    delete communicator;
    communicator = nullptr;
}

bool NetIoWorker::AddListen(Communicator *communicator)
{
    int clientFd = communicator->GetFd();
    std::unique_lock<std::mutex> lock(this->m_listenFdsMutex);

    if (this->m_listenFds.size() >= this->M_MAX_FDS) {
        return false;
    }

    if (this->SetFdBlocking(clientFd, false) == false) {
        LOGGER.Log(ERROR, "[NetIoWorker]set fd nonblocking error. fd=" + std::to_string(clientFd));
        return false;
    }

    epoll_event fdEvent = {0};
    fdEvent.data.fd = clientFd;
    fdEvent.events = EPOLLIN | EPOLLERR;
    fdEvent.data.ptr = (void *)communicator;
    LOGGER.Log(INFO, "[NetIoWorker]add fd to epoll. fd=" + std::to_string(clientFd));
    int ret = epoll_ctl(this->m_epollFd, EPOLL_CTL_ADD, clientFd, &fdEvent);
    if (ret == -1) {
        LOGGER.Log(ERROR, "[NetIoWorker]add fd to epoll error. fd=" + std::to_string(clientFd));
        return false;
    }
    LOGGER.Log(INFO, "[NetIoWorker]add fd to epoll success. fd=" + std::to_string(clientFd));
    this->AddTimer(clientFd);
    this->m_listenFds.push_back(communicator);
}

void NetIoWorker::RemoveListenFd(Communicator *communicator)
{
    LOGGER.Log(INFO, "[NetIoWorker]remove fd from epoll. fd=" + std::to_string(communicator->GetFd()));
    this->DestroyCommunicator(communicator);
}

void NetIoWorker::Working()
{
    int epollFd = epoll_create(10);
    if (epollFd == -1) {
        LOGGER.Log(ERROR, "[NetIoWorker]create epoll error.");
        return;
    }
    this->m_epollFd = epollFd;
    int64_t preTime = GetCurEpochTimeWithSecond();
    int64_t period = m_timer.GetTimeoutPeriod();
    while (true) {
        int64_t curTime = GetCurEpochTimeWithSecond();
        if (curTime - preTime >= period) {
            this->MoveTimer();
            preTime = curTime;
        }

        epoll_event epoll_events[this->M_MAX_FDS];
        int timeOut = 1000; // TODO: timeout 应该从外层传入，这里的timeout的意义？
        int retLen = epoll_wait(this->m_epollFd, epoll_events, this->M_MAX_FDS, timeOut);
        if (retLen < 0) {
            if (errno == EINTR) {
                continue;
            }
            LOGGER.Log(ERROR, "[NetIoWorker]epoll wait error.");
            return;
        }
        if (retLen == 0) {
            continue;
        }
        
        for (size_t i = 0; i < (size_t)retLen; ++i)
        {
            CommunicatorHandleResult ret = CommunicatorHandleOK;
            int tmpFd = epoll_events[i].data.fd;
            if ((Communicator *)epoll_events[i].data.ptr == nullptr) {
                continue;
            }
            Communicator *communicator = (Communicator *)epoll_events[i].data.ptr;
            if (epoll_events[i].events & EPOLLIN) {
                this->UpdateTimer(communicator->GetFd());
                ret = communicator->HandleSocketRead();
                // 尝试发送数据
                if (ret == CommunicatorHandleOK && communicator->IsNeedSendData()) {
                    ret = communicator->HandleSocketWrite();
                    if (ret == CommunicatorHandleOK && communicator->IsNeedSendData()) {
                        // 没有发送完, 注册写事件
                        communicator->AddEpollEvent(m_epollFd, EPOLLIN | EPOLLOUT | EPOLLERR);
                    }
                }
            } else  if (epoll_events[i].events & EPOLLOUT) {
                this->UpdateTimer(communicator->GetFd());
                ret = ((Communicator *)epoll_events[i].data.ptr)->HandleSocketWrite();
                if (ret == CommunicatorHandleOK && !communicator->IsNeedSendData()) {
                    // 发送完，取消写事件
                    communicator->AddEpollEvent(m_epollFd, EPOLLIN | EPOLLERR);
                }
            } else  if (epoll_events[i].events & EPOLLERR) {
                ret = ((Communicator *)epoll_events[i].data.ptr)->HandleSocketError();
            }

            switch (ret) {
                case CommunicatorHandleOK:
                    continue;
                    break;
                case CommunicatorHandleDelete:
                    this->RemoveListenFd((Communicator *)epoll_events[i].data.ptr);
                    continue;
                default:
                    break;
            }
        }
    }
}

bool NetIoWorker::SetFdBlocking(int fd, bool isBlocking)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        return false;
    }
    if (isBlocking) {
        flags &= ~O_NONBLOCK;
    } else {
        flags |= O_NONBLOCK;
    }
    if (fcntl(fd, F_SETFL, flags) == -1) {
        return false;
    }
    return true;
}

bool NetIoWorker::EpollDelSocketFd(int fd)
{
    struct epoll_event event;
    event.data.fd = fd;
    epoll_ctl(this->m_epollFd, EPOLL_CTL_DEL, fd, &event);
}

void NetIoWorker::TimoutAction(int fd)
{
    {
        std::unique_lock<std::mutex> lock(this->m_listenFdsMutex);
        this->m_listenFds.erase(std::remove_if(
                this->m_listenFds.begin(),
                this->m_listenFds.end(),
                [fd](Communicator *i) { return i->GetFd() == fd;}),
            this->m_listenFds.end());
    }
    close(fd);
}

void NetIoWorker::AddTimer(int fd)
{
    std::unique_lock<std::mutex> lock(m_timerUsed);
    TimeOutAction action = std::bind(&NetIoWorker::TimoutAction, this, fd);
    m_timer.Add(fd, action);
}
void NetIoWorker::DleteTimer(int fd)
{
    std::unique_lock<std::mutex> lock(m_timerUsed);
    m_timer.Delete(fd);
}
void NetIoWorker::UpdateTimer(int fd)
{
    std::unique_lock<std::mutex> lock(m_timerUsed);
    m_timer.Update(fd);
}

void NetIoWorker::MoveTimer()
{
    std::unique_lock<std::mutex> lock(m_timerUsed);
    m_timer.MoveTime(GetCurEpochTimeWithSecond());
}