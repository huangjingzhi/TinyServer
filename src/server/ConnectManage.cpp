


#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include "ConnectManage.h"
using namespace std;

ConnectManage::ConnectManage(int port, int netIoworkerNumber): m_port(port), m_serverFd(-1),
    m_netIoManage(netIoworkerNumber), m_isInit(false)
{
}

ConnectManage::~ConnectManage()
{
    // 是否需要释放信号量，锁这些

    close(this->m_serverFd);
}
void ConnectManage::setInit(bool isInit)
{
    std::unique_lock<std::mutex> lock(this->m_isInitMutex);
    this->m_isInit = true;
    this->m_isInitConVar.notify_all();
}


bool ConnectManage::init()
{
    this->m_netIoManage.Init();
    this->init_socket();

    this->m_selfTHread = std::thread(ConnectManage::Run, this);
    this->setInit(true);
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
    sockAddr.sin_port = htons(this->m_port);

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
}

void ConnectManage::Run()
{
    {
        std::unique_lock<std::mutex> lock(this->m_isInitMutex);
        while (!this->m_isInit) {
            this->m_isInitConVar.wait(lock);
        }
    }

    while (true) {
        struct sockaddr_in clientSock;
        socklen_t clientLen = sizeof(clientSock);

        int clientFd = accept(this->m_serverFd, (struct sockaddr *)&clientSock, &clientLen);
        if (clientFd == -1) {
            // todo:log
            std::cout << "accept error" << std::endl;
            continue;
        }
        this->m_netIoManage.AddListenFd(clientFd);
    }
}

void ConnectManage::JoinThreads()
{
    this->m_selfTHread.join();
    this->m_netIoManage.JoinThreads();
}