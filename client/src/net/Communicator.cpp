#include "Communicator.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

Communicator::Communicator(std::string serverHost, uint16_t serverPort): m_serverHost(serverHost), m_serverPost(serverPort)
{
}

Communicator::~Communicator()
{
}

bool Communicator::SendMsg(std::vector<char> &msg)
{
    if (m_serverHost.empty()) {
        return false;
    }
    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd < 0) {
        std::cout << "create socket fd fail." << std::endl;
    }

    struct sockaddr_in sockAddr;
    sockAddr.sin_port = htons(m_serverPost);
    sockAddr.sin_addr.s_addr = inet_addr(m_serverHost.data());
    sockAddr.sin_family = AF_INET;
    int ret = connect(sockFd, (struct sockaddr *)&sockAddr, sizeof(sockAddr));
    if ( ret >= 0) {
        std::cout << "connect" << std::endl;
        size_t sendOffset = 0;

        uint32_t msgLen = msg.size();
        uint32_t netMsgLen = htonl(msgLen);
        while (sendOffset < sizeof(uint32_t)) {
            int sendLen = send(sockFd, &netMsgLen + sendOffset, sizeof(uint32_t) - sendOffset, 0);
            if (sendLen == -1) {
                std::cout << "send error" << std::endl;
                return false;
            }
            sendOffset += sendLen;
            std::cout << "sendLen " << sendLen << std::endl;
        }
        std::cout << "[debug]" << msg.size() << " send len " << msgLen << " after to net " << netMsgLen << " retLen " << ntohl(netMsgLen) << std::endl;
        sendOffset = 0;
        while (sendOffset < msg.size()) {
            int sendLen = send(sockFd, msg.data() + sendOffset, msg.size() - sendOffset, 0);
            if (sendLen == -1) {
                std::cout << "send error" << std::endl;
                return false;
            }
            sendOffset += sendLen;
        }
        std::cout << "send msg: len(" << msgLen << "): " << std::string(msg.begin(), msg.end()) << std::endl;
        close(sockFd);
        return true;
    } else {
        std::cout << "connect server error." << std::endl;
        close(sockFd);
        return false;
    }
}
