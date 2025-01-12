
#include "Channel.h"
#include  <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "Logger.h"

Channel::Channel(int fd) : m_fd(fd)
{
}

Channel::~Channel()
{
    close(m_fd);
}

int Channel::GetFd() const
{
    return m_fd;
}

ChannelHandleResult Channel::HandleSocketRead()
{
    return ChannelHandleOK;
}

ChannelHandleResult Channel::HandleSocketWrite()
{
    return ChannelHandleOK;
}

ChannelHandleResult Channel::HandleSocketError()
{
    // 出现异常，关闭 fd, 如何让上层 ConnectManage 知道
    return ChannelHandleDelete;
}
void Channel::AddEpollEvent(int epollFd, int event)
{
    epoll_event fdEvent = {0};
    fdEvent.data.fd = m_fd;
    fdEvent.events = event;
    fdEvent.data.ptr = (void *)this;
    if (epoll_ctl(epollFd, EPOLL_CTL_MOD, m_fd, &fdEvent) == -1) {
        LOGGER.Log(ERROR, "[Channel]add epoll event error. fd=" + std::to_string(m_fd) + " event=" + std::to_string(event));
    }
}

bool Channel::IsNeedSendData()
{
    return true;
}