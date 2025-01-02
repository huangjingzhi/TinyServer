
#include "Communicator.h"
#include  <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "../commom/Logger.h"

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
    epoll_event fdEvent = {0};
    fdEvent.data.fd = m_fd;
    fdEvent.events = event;
    fdEvent.data.ptr = (void *)this;
    if (epoll_ctl(epollFd, EPOLL_CTL_MOD, m_fd, &fdEvent) == -1) {
        LOGGER.Log(ERROR, "[Communicator]add epoll event error. fd=" + std::to_string(m_fd) + " event=" + std::to_string(event));
    }
}