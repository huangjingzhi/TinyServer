
#include "Communicator.h"
#include  <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>


Communicator::Communicator(int fd) : m_fd(fd)
{
}

Communicator::~Communicator()
{
    close(m_fd); // TODO: close 的时候，是否应该判断时机
}

int Communicator::GetFd() const
{
    return m_fd;
}

CommunicatorHandleResult Communicator::HandleSocketRead()
{
    return CommunicatorHandleOK;
}

CommunicatorHandleResult Communicator::HandleSocketWrite()
{
    return CommunicatorHandleOK;
}

CommunicatorHandleResult Communicator::HandleSocketError()
{
    // 出现异常，关闭 fd, 如何让上层 ConnectManage 知道
    return CommunicatorHandleDelete;
}
void Communicator::AddEpollEvent(int epollFd, int event)
{
    struct epoll_event ev = {0};
    ev.events |= event; // Add EPOLLOUT to the existing events
    if (epoll_ctl(epollFd, EPOLL_CTL_MOD, m_fd, &ev) == -1) {
        std::cout << "add epoll event error." << std::endl;
    }
}