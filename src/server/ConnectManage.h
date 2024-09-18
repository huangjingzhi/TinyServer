#include <iostream>
#include <condition_variable>
#include <thread>
#include <mutex>
#include "NetIoManage.h"

using namespace std;

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
    
    /* private function */
    bool init_socket();
    void setInit(bool isInit);

public:
    ConnectManage(int port, int netIoworkerNumber, int workerMaxFd);
    ~ConnectManage();
    bool init();
    void Run();
    void JoinThreads();
};