#include <iostream>


#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

enum CommunicatorHandleResult {
    CommunicatorHandleOK = 0,
    CommunicatorHandleError = 1,
    CommunicatorHandleDelete = 2
};

class Communicator
{
protected:
    int m_fd;
public:
    Communicator(int fd);
    virtual ~Communicator();
    int GetFd() const;
    virtual CommunicatorHandleResult HandleSocketRead();
    virtual CommunicatorHandleResult HandleSocketWrite();
    virtual CommunicatorHandleResult HandleSocketError();
    virtual bool IsNeedSendData();
    void AddEpollEvent(int epollFd, int event);
};

#endif