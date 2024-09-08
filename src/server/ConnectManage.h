#include <iostream>
using namespace std;

class ConnectManage
{
private:
    /* data */
    int m_port;
    int m_serverFd;
    /* private function */
    bool init_socket();
public:
    ConnectManage(int port);
    ~ConnectManage();
    bool init();
};