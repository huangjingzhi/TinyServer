#include <iostream>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "NetIoManage.h"
#include "Communicator.h"
#include "App.h"
#include "Logger.h"

using namespace std;

#ifndef CONNECTMANAGE_H
#define CONNECTMANAGE_H

template <typename T> // todo: 考虑一下：这里可以让APP提供一个构造Coumunicator的函数，就可以不用构造函数了
class ConnectManage
{
private:
    /* data */
    int m_port;
    int m_serverFd;
    std::thread m_selfThread;
    NetIoManage m_netIoManage;
    
    bool m_isInit;
    std::condition_variable m_isInitConVar;
    std::mutex m_isInitMutex;
    
    App *m_app;
    /* private function */
    bool InitListenSocket();
    void SetInitState(bool isInit);

public:
    ConnectManage(int a, int b ,int c, App *app=nullptr);
     ~ConnectManage();
    bool Init();
    void Run();
    void JoinThreads();
};

template <typename T>
ConnectManage<T>::ConnectManage(int port, int netIoworkerNumber, int workerMaxFd, App *app):
    m_port(port), m_serverFd(-1), m_app(app),
    m_netIoManage(netIoworkerNumber, workerMaxFd), m_isInit(false)
{
}

template <typename T>
ConnectManage<T>::~ConnectManage()
{
    close(this->m_serverFd);
}

template <typename T>
void ConnectManage<T>::SetInitState(bool isInit)
{
    std::unique_lock<std::mutex> lock(this->m_isInitMutex);
    this->m_isInit = true;
    this->m_isInitConVar.notify_all();
}

template <typename T>
bool ConnectManage<T>::InitListenSocket()
{
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd == -1) {
        LOGGER.Log(ERROR, "[ConnectManage]Failed to create socket.");
        return false;
    }

    m_serverFd = serverFd;

    int opt = 1;
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        LOGGER.Log(ERROR, "[ConnectManage]Failed to set socket opt(SO_REUSEADDR).");
        close(serverFd);
        return -1;
    }

    // 初始化服务器地址
    struct sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sockAddr.sin_port = htons(this->m_port);

    if (bind(this->m_serverFd, (struct sockaddr *)&sockAddr, sizeof(sockAddr)) == -1) {
        LOGGER.Log(ERROR, "[ConnectManage]Failed to bind socket.");
        close(this->m_serverFd);
        return false;
    }

    if (listen(this->m_serverFd, SOMAXCONN) == -1)
    {
        LOGGER.Log(ERROR, "[ConnectManage]Failed to listen.");
        close(this->m_serverFd);
        return -1;
    }
    return true;
}

template <typename T>
bool ConnectManage<T>::Init()
{
    bool ret = this->InitListenSocket();
    if (!ret) {
        return false;
    }

    this->m_netIoManage.Init();

    this->m_selfThread = std::thread(&ConnectManage::Run, this);
    this->SetInitState(true);
    return true;
}

template <typename T>
void ConnectManage<T>::Run()
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
            LOGGER.Log(ERROR, "[ConnectManage]Failed to accept.");
            continue;
        }
        LOGGER.Log(INFO, "[ConnectManage]Accept client. fd=" + std::to_string(clientFd));
        T *t = new T(clientFd, this->m_app);
        this->m_netIoManage.AddListenFd(t);
    }
}

template <typename T>
void ConnectManage<T>::JoinThreads()
{
    this->m_selfThread.join();
    this->m_netIoManage.JoinThreads();
}

#endif
