
#include <iostream>
#include <vector>
#include <mutex>
#include "Channel.h"
#include "TagTimer.h"

#ifndef NETWORKER_H
#define NETWORKER_H

class NetIoWorker
{
private:
    std::vector<Channel *> m_listenFds;
    std::mutex m_listenFdsMutex;
    int m_epollFd;
    bool m_isWorking;
    int M_MAX_FDS;
    TagTimer<int> m_timer;
    std::mutex m_timerUsed;
    void DestroyChannel(Channel *channel);
    bool SetFdBlocking(int fd, bool isBlocking);
    bool EpollDelSocketFd(int fd);

    void AddTimer(int fd);
    void DleteTimer(int fd);
    void UpdateTimer(int fd);
    void MoveTimer();
    void TimoutAction(int fd);
public:
    NetIoWorker(int maxFds);
    NetIoWorker(const NetIoWorker &netIoWorker);
    ~NetIoWorker();
    bool AddListen(Channel *channel);
    void RemoveListenFd(Channel *channel);
    void Working();
};

#endif