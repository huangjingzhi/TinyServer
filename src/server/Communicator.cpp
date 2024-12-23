
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
    // 处理可读事件
    char buf[1024] = { 0 }; // todo: 有限制，需要修改
    int ret = recv(m_fd, buf, 1024, 0);
    if (ret > 0) {
        int sendRet = send(m_fd, buf, strlen(buf), 0);
        if (sendRet == -1) {
                // todo:log
            }
        if (sendRet != strlen(buf)) {
            // todo:log
            std::cout << "send error." << std::endl;
        }
        if (sendRet == strlen(buf)) {
            std::cout << "send back msg to " << m_fd  <<": " << buf << std::endl; 
        }
    } else {
        if (errno == EAGAIN) {
            return CommunicatorHandleOK;
        }
        return CommunicatorHandleDelete;
    }
    return CommunicatorHandleOK;
}

CommunicatorHandleResult Communicator::HandleSocketWrite()
{
    std::cout << "meet out event" << std::endl;
    return CommunicatorHandleOK;
}

CommunicatorHandleResult Communicator::HandleSocketError()
{
    // 出现异常，关闭 fd, 如何让上层 ConnectManage 知道
    return CommunicatorHandleDelete;
}