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
    // 是否需要释放信号量，锁这些
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
    std::cout << "meet Init in run of ConnectManage." << std::endl; 
    while (true) {
        struct sockaddr_in clientSock;
        socklen_t clientLen = sizeof(clientSock);

        int clientFd = accept(this->m_serverFd, (struct sockaddr *)&clientSock, &clientLen);
        if (clientFd == -1) {
            // todo:log
            std::cout << "accept error." << std::endl;
            continue;
        }
        std::cout << "get fd " << clientFd << "." << std::endl;
        T *t = new T(clientFd);
        std::cout << "[debug] " << "get communicate " << clientFd << " " << t << std::endl;  
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
