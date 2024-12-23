
#include <iostream>
#include <vector>
#include <mutex>
#include "Communicator.h"

#ifndef NETWORKER_H
#define NETWORKER_H

class NetIoWorker
{
private:
    std::vector<Communicator *> m_listenFds;
    std::mutex m_listenFdsMutex;
    int m_epollFd;
    bool m_isWorking;
    int M_MAX_FDS;

    void DestroyCommunicator(Communicator *communicator);
public:
    NetIoWorker(int maxFds);
    NetIoWorker(const NetIoWorker &netIoWorker);
    ~NetIoWorker();
    bool AddListen(Communicator *communicator); // TODO: 此处可以使用移动构造函数处理、
    void RemoveListenFd(Communicator *communicator);
    void Working();
};

#endif