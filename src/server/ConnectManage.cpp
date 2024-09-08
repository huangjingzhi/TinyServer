


#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>

#include <iostream>
#include "ConnectManage.h"
using namespace std;



ConnectManage::ConnectManage(int port): m_port(port), m_serverFd(-1)
{
}

ConnectManage::~ConnectManage()
{
}

bool ConnectManage::init()
{
    this->init_socket();
}

bool ConnectManage::init_socket()
{
    
    // 创建侦听sock
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd == -1) {
        // todo:log
        return false;
    }

    m_serverFd = serverFd;
    
    // 初始化服务器地址
    struct sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sockAddr.sin_port = this->m_port;
    
    if (bind(this->m_serverFd, (struct sockaddr *)&sockAddr, sizeof(sockAddr)) == -1) {
        // todo:log
        std::cout  << "bind error." << std::endl;
        // if error, need to close fd;
        close(this->m_serverFd);
        return false;
    }


    if (listen(this->m_serverFd, SOMAXCONN) == -1)
    {
        std::cout << "listen error." << std::endl;
        close(this->m_serverFd);
        return -1;
    }
    std::cout << "listen for :" << this->m_serverFd << " in port " << this->m_port << std::endl;

    while (true) {
        struct sockaddr_in clientSock;
        socklen_t clientLen = sizeof(clientSock);

        int clientFd = accept(this->m_serverFd, (struct sockaddr *)&clientSock, &clientLen);
        if (clientFd == -1) {
            // todo:log
            std::cout << "accept error" << std::endl;
            continue;
        }
        
        char buf[1024] = {0};
        int retLen = recv(clientFd, buf, 1024, 0);
        if (retLen == -1) {
            // todo:log
        }
        if (retLen == 0) {
            // close
        }
        if(retLen > 0) {
            std::cout << "recv msg from fd=" << clientFd << ": "  << buf << std::endl;
            
            int sendRet = send(clientFd, buf, strlen(buf), 0);
            if (sendRet == -1) {
                // todo:log
            }
            if (sendRet != strlen(buf)) {
                // todo:log
                std::cout << "send error." << std::endl;
            }
            if (sendRet == strlen(buf)) {
                std::cout << "send back msg to " << clientFd  <<": " << buf << std::endl; 
            }
        }
        close(clientFd);
    }

    close(this->m_serverFd);

}
