#include "Channel.h"
#include "App.h"
#include "RawMsgManager.h"

#ifndef TLVCHANNEL
#define TLVCHANNEL

class TlvChannel : public Channel
{
private:
    RawMsgManager m_msgManger;
    App *m_app;
    void HandleMsgs();
public:
    TlvChannel(int fd, App *app=nullptr);
    ~TlvChannel() override;
    ChannelHandleResult HandleSocketRead() override;
    ChannelHandleResult HandleSocketWrite() override;
    ChannelHandleResult HandleSocketError() override;
    std::vector<std::vector<char>> GetAllMsg();
};

#endif