#include "NetIoWorker.h"
#include <algorithm>
#include <string.h>
#include  <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>

NetIoWorker::NetIoWorker(int maxFds):M_MAX_FDS(maxFds), m_epollFd(-1), m_isWorking(false)
{
}

NetIoWorker::NetIoWorker(const NetIoWorker&netIoWorker)
{
    this->M_MAX_FDS =  netIoWorker.M_MAX_FDS;
    this->m_epollFd = -1;
    this->m_isWorking = false;
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

    delete communicator;
    communicator = nullptr;
}

bool NetIoWorker::AddListen(Communicator *communicator)
{
    std::unique_lock<std::mutex> lock(this->m_listenFdsMutex);

    if (this->m_listenFds.size() >= this->M_MAX_FDS) {
        return false;
    }

    if (this->SetFdBlocking(communicator->GetFd(), false) == false) {
        std::cout << "set fd nonblocking error for fd=" << communicator->GetFd() << std::endl;
        return false;
    }

    epoll_event fdEvent = {0};
    fdEvent.data.fd = communicator->GetFd();
    fdEvent.events = EPOLLIN;
    fdEvent.data.ptr = (void *)communicator;
    std::cout << "[test] add fd = " << communicator->GetFd() <<  " to epoll." << std::endl;
    int ret = epoll_ctl(this->m_epollFd, EPOLL_CTL_ADD, communicator->GetFd(), &fdEvent);
    if (ret == -1) {
        std::cout << "add fd error for add fd=" << communicator->GetFd() << std::endl;
        return false;
    }
    std::cout << "[test] add fd = " << communicator->GetFd() <<  " to epoll end." << std::endl;

    this->m_listenFds.push_back(communicator);
}

void NetIoWorker::RemoveListenFd(Communicator *communicator)
{
    std::cout << "[debug] to destroy " << communicator->GetFd() << " " << communicator << std::endl;
    this->DestroyCommunicator(communicator);
}

void NetIoWorker::Working()
{
    int epollFd = epoll_create(10); // TODO: 进一步分析这里需要大于0的原因，应该设置什么合适的值
    if (epollFd == -1) {
        std::cout << "create epll fd error. " << std::endl; 
        return;
    }
    this->m_epollFd = epollFd;

    while (true) {
        epoll_event epoll_events[this->M_MAX_FDS];
        int timeOut = 1000; // TODO: timeout 应该从外层传入，这里的timeout的意义？
        int retLen = epoll_wait(this->m_epollFd, epoll_events, this->M_MAX_FDS, timeOut);
        if (retLen < 0) {
            if (errno == EINTR) {
                continue;
            }
            std::cout << "error for  epoll_wait." << std::endl;
            return;
        }
        if (retLen == 0) {
            continue;
        }
        
        for (size_t i = 0; i < (size_t)retLen; ++i)
        {
            CommunicatorHandleResult ret = CommunicatorHandleOK;
            int tmpFd = epoll_events[i].data.fd;
            // TODO: 这里需要判断 epoll_events[i].data.ptr 是否是空，已经有效
            if ((Communicator *)epoll_events[i].data.ptr == nullptr) {
                continue;
            }
            if (epoll_events[i].events & EPOLLIN) {
                ret = ((Communicator *)epoll_events[i].data.ptr)->HandleSocketRead();
            }
            else if (epoll_events[i].events & EPOLLOUT) // TODO: 分析：这里使用else if，是不是意味着不能处理EPOLLIN，EPOLLOUT同时到来的
            {
                ret = ((Communicator *)epoll_events[i].data.ptr)->HandleSocketWrite();
            }
            else if (epoll_events[i].events & EPOLLERR)
            {
                ret = ((Communicator *)epoll_events[i].data.ptr)->HandleSocketError();
            }

            switch (ret)
            {
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