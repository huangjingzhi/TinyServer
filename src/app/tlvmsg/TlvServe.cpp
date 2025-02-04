#include "TlvServer.h"
#include "TlvChannel.h"

TlvServer::TlvServer(int port, int netIoworkerNumber, int workerMaxFd):
    m_port(port), m_netIoworkerNumber(netIoworkerNumber), m_workerMaxFd(workerMaxFd)
{
    
}

TlvServer::~TlvServer()
{

}

void TlvServer::Start()
{
    this->m_tlvConnectManager.reset(
        new ConnectManage<TlvChannel>(
            m_port,m_netIoworkerNumber,m_workerMaxFd, this));
    m_tlvConnectManager->Init();
}

void TlvServer::Update(Channel *channel)
{
    TlvChannel *tlvChannel = dynamic_cast<TlvChannel *>(channel);
    if (tlvChannel == nullptr) {
        return;
    }
    auto msgs = tlvChannel->GetAllMsg();
    for (auto &msg : msgs) {
        this->HandleMsg(msg);
    }
}


void TlvServer::JoinThreads()
{
    m_tlvConnectManager->JoinThreads();
}
void TlvServer::HandleMsg(std::vector<char> &msg)
{
    std::cout << "recv msg: " << std::string(msg.begin(), msg.end()) << std::endl;
}
