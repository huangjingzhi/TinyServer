#include <iostream>

#ifndef CHANNEL
#define CHANNEL

enum ChannelHandleResult {
    ChannelHandleOK = 0,
    ChannelHandleError = 1,
    ChannelHandleDelete = 2
};

class Channel
{
protected:
    int m_fd;
public:
    Channel(int fd);
    virtual ~Channel();
    int GetFd() const;
    virtual ChannelHandleResult HandleSocketRead();
    virtual ChannelHandleResult HandleSocketWrite();
    virtual ChannelHandleResult HandleSocketError();
    virtual bool IsNeedSendData();
    void AddEpollEvent(int epollFd, int event);
};

#endif