#include "TlvChannel.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

TlvChannel::TlvChannel(int fd, App *app) : Channel(fd), m_app(app)
{
}

TlvChannel::~TlvChannel()
{
}

ChannelHandleResult TlvChannel::HandleSocketError()
{
    return ChannelHandleDelete;
}

void TlvChannel::HandleMsgs()
{
    auto msgs = m_msgManger.GetAllMsg();
    for (auto &msg : msgs) {
        // 此处也可以通过 m_app 调用上层框架处理 msgs
    }
}

ChannelHandleResult TlvChannel::HandleSocketRead()
{
    Msg msg(8 * 1024);
    int ret = read(this->m_fd, msg.buf, msg.maxLen);
    if (ret > 0) {
        msg.len = ret;
        m_msgManger.PutRawMsg(msg);
        m_msgManger.ParseRawMsg();
        HandleMsgs();
    } else {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            FreeMsg(msg);
            return ChannelHandleOK;
        }
        FreeMsg(msg);
        return ChannelHandleDelete;
    }

    FreeMsg(msg);
    return ChannelHandleOK;
}

ChannelHandleResult TlvChannel::HandleSocketWrite()
{
    std::cout << "meet out event" << std::endl;
    return ChannelHandleOK;
}
