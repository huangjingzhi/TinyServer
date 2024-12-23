#include "TlvCommunicator.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

TlvCommunicator::TlvCommunicator(int fd, App *app=nullptr) : Communicator(fd), m_app(app)
{
}

TlvCommunicator::~TlvCommunicator()
{
}

CommunicatorHandleResult TlvCommunicator::HandleSocketError()
{
    return CommunicatorHandleDelete;
}

void TlvCommunicator::HandleMsgs()
{
    auto msgs = m_msgManger.GetAllMsg();
    for (auto &msg : msgs) {
        std::cout << "get msg from fd=" << m_fd << " " << std::string(msg.begin(), msg.end()) << std::endl;
        // 此处也可以通过 m_app 调用上层框架处理 msgs
    }
}

CommunicatorHandleResult TlvCommunicator::HandleSocketRead()
{
    Msg msg(8 * 1024); // TODO 这里每次都要申请一次内存，是否有优化的空间
    int ret = read(this->m_fd, msg.buf, msg.maxLen);
    if (ret > 0) {
        msg.len = ret;
        m_msgManger.PutRawMsg(msg);
        m_msgManger.ParseRawMsg();
        // HandleMsgs();
    } else {
        if (errno == EAGAIN) {
            FreeMsg(msg);
            return CommunicatorHandleOK;
        }
        FreeMsg(msg);
        return CommunicatorHandleDelete;
    }

    FreeMsg(msg);
    return CommunicatorHandleOK;
}

CommunicatorHandleResult TlvCommunicator::HandleSocketWrite()
{
    std::cout << "meet out event" << std::endl;
    return CommunicatorHandleOK;
}
