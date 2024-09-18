
#include <iostream>
#include <vector>
#include <mutex>

class NetIoWorker
{
private:
    /* data */
    std::vector<int> m_listenFds;
    std::mutex m_listenFdsMutex;
    int m_epollFd;
    bool m_isWorking;
    int M_MAX_FDS;


public:
    NetIoWorker(int maxFds);
    NetIoWorker(const NetIoWorker &netIoWorker);
    ~NetIoWorker();
    bool AddListen(int fd); 
    void Working();
};