

#include "App.h"
#include "ConnectManage.h"
#include <memory>
#ifndef TLVSERVER_H
#define TLVSERVER_H
class TlvChannel;
class TlvServer: public App
{
    int m_port;
    int m_netIoworkerNumber;
    int m_workerMaxFd;
    std::unique_ptr<ConnectManage<TlvChannel>> m_tlvConnectManager;
public:
    TlvServer(int port, int netIoworkerNumber, int workerMaxFd);
    ~TlvServer();
    void Start();
    virtual void Update(Channel *channel) override;
    void JoinThreads();

    virtual void HandleMsg(std::vector<char> &msg);
};

#endif
