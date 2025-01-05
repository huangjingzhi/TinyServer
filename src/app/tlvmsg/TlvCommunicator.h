#include "Communicator.h"
#include "App.h"
#include "RawMsgManager.h"

#ifndef TLVCOMMUNICATOR_H
#define TLVCOMMUNICATOR_H

class TlvCommunicator : public Communicator
{
private:
    RawMsgManager m_msgManger;
    App *m_app;
    void HandleMsgs();
public:
    TlvCommunicator(int fd, App *app=nullptr);
    ~TlvCommunicator() override;
    CommunicatorHandleResult HandleSocketRead() override;
    CommunicatorHandleResult HandleSocketWrite() override;
    CommunicatorHandleResult HandleSocketError() override;
};

#endif