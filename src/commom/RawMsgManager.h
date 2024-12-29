
#include <iostream>
#include <list>
#include <vector>
#include "Msg.h"

#ifndef RAWMSGMANAGER_H
#define RAWMSGMANAGER_H

class RawMsgManager
{
private:    
    std::vector<char> m_rawMsgbuf; // 是否可以使用string 代替？
    std::vector<std::vector<char>> m_readyMsg;

public:
    RawMsgManager(/* args */);
    ~RawMsgManager();

    bool Empty();

    void PutRawMsg(Msg &rawMsg);
    std::vector<char> GetOneMsg();
    std::vector<std::vector<char>> GetAllMsg();

    bool ParseRawMsg();
};

#endif