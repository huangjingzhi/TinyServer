
#include <iostream>
#include <vector>
#include <mutex>
#include "Communicator.h"
#include "TagTimer.h"

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
    TagTimer<int> m_timer;
    std::mutex m_timerUsed;
    void DestroyCommunicator(Communicator *communicator);
    bool SetFdBlocking(int fd, bool isBlocking);
    bool EpollDelSocketFd(int fd);

    void AddTimer(int fd);
    void DleteTimer(int fd);
    void UpdateTimer(int fd);
    void MoveTimer();
public:
    NetIoWorker(int maxFds);
    NetIoWorker(const NetIoWorker &netIoWorker);
    ~NetIoWorker();
    bool AddListen(Communicator *communicator);
    void RemoveListenFd(Communicator *communicator);
    void Working();
};

#endif