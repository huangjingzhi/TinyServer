#include <vector>
#include <string>

#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

class Communicator
{
private:
    /* data */
    std::vector<std::vector<char>> m_sendMsg;

    std::string m_serverHost;
    uint16_t m_serverPost;
    
public:
    Communicator(std::string serverHost, uint16_t serverPort);
    ~Communicator();
    bool SendMsg(std::vector<char> &msg);
};

#endif