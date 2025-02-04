#include "TlvChannel.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "Logger.h"

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
    m_app->Update(this);
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
    return ChannelHandleOK;
}

std::vector<std::vector<char>> TlvChannel::GetAllMsg()
{
    return std::move(m_msgManger.GetAllMsg());
}
