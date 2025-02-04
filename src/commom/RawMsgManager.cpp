#include "RawMsgManager.h"
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

RawMsgManager::RawMsgManager(): m_rawMsgbuf(0), m_readyMsg(0)
{
    m_rawMsgbuf.reserve(2 * 1024);
}

RawMsgManager::~RawMsgManager()
{
}

bool RawMsgManager::Empty() {
    return m_readyMsg.empty();
}

void RawMsgManager::PutRawMsg(Msg &rawMsg)
{
    m_rawMsgbuf.insert(m_rawMsgbuf.end(), rawMsg.buf, rawMsg.buf + rawMsg.len);
}

std::vector<char> RawMsgManager::GetOneMsg()
{
    auto msg = std::move(m_readyMsg.front());
    m_readyMsg.erase(m_readyMsg.begin());
    return std::move(msg);
}

bool RawMsgManager::ParseRawMsg()
{
    uint32_t msgLen = 0;
    size_t headLen = sizeof(msgLen);
    if (m_rawMsgbuf.size() < headLen) {
        return false;
    }

    (void *)memcpy(&msgLen, m_rawMsgbuf.data(), headLen);

    msgLen = ntohl(msgLen);
    if (msgLen > MSG_MAX_LEN) {
        return false;
    }

    if ((msgLen + headLen) > m_rawMsgbuf.size()) {
        return false;
    }

    m_readyMsg.push_back(std::move(
        std::vector<char>(m_rawMsgbuf.begin() + headLen, m_rawMsgbuf.begin() + headLen + msgLen)));
    m_rawMsgbuf.erase(m_rawMsgbuf.begin(), m_rawMsgbuf.begin() + headLen + msgLen);

    return true;
}

std::vector<std::vector<char>> RawMsgManager::GetAllMsg()
{
    auto msgs = std::move(m_readyMsg);
    return std::move(msgs);
}