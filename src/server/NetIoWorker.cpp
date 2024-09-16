#include "NetIoWorker.h"
#include <algorithm>
#include <string.h>
#include  <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>





NetIoWorker::NetIoWorker(int maxFds):M_MAX_FDS(maxFds), m_epollFd(-1), m_isWorking(false)
{

}

NetIoWorker::~NetIoWorker() 
{
    for (auto fd: this->m_listenFds) {
        close(fd);
    }
}

bool NetIoWorker::AddListen(int fd)
{
    this->m_listenFdsMutex.lock();

    if (this->m_listenFds.size() >= this->M_MAX_FDS) {
        return false;
    }
    epoll_event fdEvent = {0};
    fdEvent.data.fd = fd;
    fdEvent.events = EPOLLIN;
    int ret = epoll_ctl(this->m_epollFd, EPOLL_CTL_ADD, fd, &fdEvent);
    if (ret == -1) {
        std::cout << "add fd error." << std::endl;
        return false;
    }
    this->m_listenFds.push_back(fd);

    this->m_listenFdsMutex.unlock();
}

void NetIoWorker::Working()
{
    int epollFd = epoll_create(1);
    if (epollFd == -1) {
        std::cout << "create epll fd error. " << std::endl; 
        return;
    }
    this->m_epollFd = epollFd;

    while (true) {
        
        epoll_event epoll_events[this->M_MAX_FDS];
        int timeOut = 1000;
        int ret = epoll_wait(this->m_epollFd, epoll_events, this->M_MAX_FDS, timeOut);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            std::cout << "error for  epoll_wait." << std::endl;
            return;
        }
        if (ret == 0) {
            continue;
        }
        
        for (size_t i = 0; i < ret; ++i)
        {
            int tmpFd = epoll_events[i].data.fd;
            if (epoll_events[i].events & EPOLLIN) {
                // 处理可读事件
                char buf[1024] = { 0 }; // todo: 有限制，需要修改
                int ret = recv(tmpFd, buf, 1024, 0);
                if (ret > 0) {
                    int sendRet = send(tmpFd, buf, strlen(buf), 0);
                    if (sendRet == -1) {
                            // todo:log
                        }
                    if (sendRet != strlen(buf)) {
                        // todo:log
                        std::cout << "send error." << std::endl;
                    }
                    if (sendRet == strlen(buf)) {
                        std::cout << "send back msg to " << tmpFd  <<": " << buf << std::endl; 
                    }
                } else {
                    close(tmpFd);
                    this->m_listenFdsMutex.lock();
                    auto it = std::remove_if(this->m_listenFds.begin(), this->m_listenFds.end(), [tmpFd](int i) { return i == tmpFd;});
                    this->m_listenFds.erase(it, this->m_listenFds.end());
                    this->m_listenFdsMutex.unlock();
                    if (ret != 0) {
                        std::cout << "error for recv data from fd=" << tmpFd << std::endl;
                    }
                    continue;
                }
            }
            else if (epoll_events[i].events & EPOLLOUT)
            {
                // 处理可写事件
                std::cout << "meet out event" << std::endl;
                continue;
            }
            else if (epoll_events[i].events & EPOLLERR)
            {
                // 处理出错事件
                close(tmpFd);
                this->m_listenFdsMutex.lock();
                auto it = std::remove_if(this->m_listenFds.begin(), this->m_listenFds.end(), [tmpFd](int i) { return i == tmpFd;});
                this->m_listenFds.erase(it, this->m_listenFds.end());
                this->m_listenFdsMutex.unlock();
                continue;
            }
        }
    }
}

