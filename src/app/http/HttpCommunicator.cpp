#include "HttpCommunicator.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Logger.h"
#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>

HttpCommunicator::HttpCommunicator(int fd, App *app) :
    Communicator(fd), m_app(app), m_sendBuf()
{
    m_sendBuf.reserve(4 * 1024);
}

HttpCommunicator::~HttpCommunicator()
{
}

HttpRequest &HttpCommunicator::GetHttpRequest()
{
    return m_httpRequest;
}

HttpResponse &HttpCommunicator::GetHttpResponse()
{
    return m_httpResponse;
}

void HttpCommunicator::HandleRequest()
{
    if (m_app == nullptr) {
        return;
    }
    if (m_httpRequest.IsFinshed()) {
        m_app->Update(this);
        return;
    }
}

CommunicatorHandleResult HttpCommunicator::HandleSocketRead()
{
    Msg msg(8 * 1024);
    int ret = read(this->m_fd, msg.buf, msg.maxLen);
    if (ret > 0) {
        msg.len = ret;
        m_httpRequest.PutRawMsg(msg);
        m_httpRequest.ParseRawMsg();
        HandleRequest();
    } else {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            FreeMsg(msg);
            return CommunicatorHandleOK;
        }
        if (ret == 0) {
            LOGGER.Log(INFO, "[HttpCommunicator]client close. fd=" + std::to_string(this->m_fd));  
        } else {
            LOGGER.Log(ERROR, "[HttpCommunicator]read error. fd=" + std::to_string(this->m_fd));
        }
        FreeMsg(msg);
        return CommunicatorHandleDelete;
    }

    FreeMsg(msg);
    return CommunicatorHandleOK;
}
CommunicatorHandleResult HttpCommunicator::HandleSocketWrite()
{
    if (m_httpResponse.IsReady() && !m_httpResponse.IsSending()) {
        std::string response = m_httpResponse.MakeResponse();
        m_sendBuf += response;
        m_httpResponse.SetSending(true);
    }

    if (m_sendBuf.empty()) {
        if (m_httpResponse.IsReady() &&
            m_httpResponse.IsSending() &&
            !m_httpResponse.IsSendFileEnd()) { // 就绪并且头部发送完了
            off_t filePos = m_httpResponse.GetSendFilePos();
            ssize_t retSize = sendfile(this->m_fd, m_httpResponse.GetSendFIleFd(), &filePos, m_httpResponse.GetSendFileSize() - filePos);
            if (retSize < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    LOGGER.Log(ERROR, "[HttpCommunicator]sendfile error. fd=" + std::to_string(this->m_fd));
                    return CommunicatorHandleDelete;
                }
            }
            m_httpResponse.SetSendFilePos(filePos);
        }
        return CommunicatorHandleOK;
    }

    ssize_t ret = write(this->m_fd, m_sendBuf.c_str(), m_sendBuf.size());
    if (ret < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            LOGGER.Log(ERROR, "[HttpCommunicator]write error. fd=" + std::to_string(this->m_fd));
            return CommunicatorHandleDelete;
        }
    }
    if (ret == m_sendBuf.size()) {
        m_sendBuf.clear();
    } else {
        m_sendBuf = m_sendBuf.substr(ret);
    }
    return CommunicatorHandleOK;
}

CommunicatorHandleResult HttpCommunicator::HandleSocketError()
{
    LOGGER.Log(ERROR, "[HttpCommunicator]socket error. fd=" + std::to_string(this->m_fd) + " error=" + std::to_string(errno));
    return CommunicatorHandleOK;
}

bool HttpCommunicator::IsNeedSendData()
{
    if (!m_httpRequest.IsFinshed() || !m_httpResponse.IsReady()) {
        return false;
    }
    if (m_httpResponse.IsReady() && !m_httpResponse.IsSending()) {
        std::string response = m_httpResponse.MakeResponse();
        m_sendBuf += response;
        m_httpResponse.SetSending(true);
    }

    if (!m_sendBuf.empty()) {
        return true;
    }
    if (m_httpResponse.GetSendSrc() == HTTPRES_SRC_FILE) {
        return !m_httpResponse.IsSendFileEnd();
    }
    return false;
}