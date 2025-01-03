#include "HttpCommunicator.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>
#include "../../commom/Logger.h"

Logger LOGGER_Read{"HttpCommunicator_read.log", DEBUG};
Logger LOGGER_Write{"HttpCommunicator_write.log", DEBUG};
Logger LOGGER_RW{"HttpCommunicator_RW.log", DEBUG};

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

        if (m_httpResponse.IsReady() && !m_httpResponse.IsSending()) {
            std::string response = m_httpResponse.MakeResponse();
            m_sendBuf += response;
            m_httpResponse.SetSending(true);
        }
        return;
    }
}

CommunicatorHandleResult HttpCommunicator::HandleSocketRead()
{
    Msg msg(8 * 1024);
    int ret = read(this->m_fd, msg.buf, msg.maxLen);
    if (ret > 0) {
        LOGGER_Read.Log(DEBUG, "[HttpCommunicator]read msg. fd=" + std::to_string(this->m_fd) + " msg=\n" + std::string(msg.buf, ret));
        LOGGER_RW.Log(DEBUG, "[HttpCommunicator]read msg. fd=" + std::to_string(this->m_fd) + " msg=\n" + std::string(msg.buf, ret));
        msg.len = ret;
        LOGGER.Log(DEBUG, "[HttpCommunicator]read msg. fd=" + std::to_string(this->m_fd) + " msg=" + std::string(msg.buf, msg.len));
        m_httpRequest.PutRawMsg(msg);
        m_httpRequest.ParseRawMsg();
        HandleRequest();
    } else {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            FreeMsg(msg);
            return CommunicatorHandleOK;
        }
        FreeMsg(msg);
        if (ret == 0) {
            LOGGER.Log(INFO, "[HttpCommunicator]client close. fd=" + std::to_string(this->m_fd));  
        } else {
            LOGGER.Log(ERROR, "[HttpCommunicator]read error. fd=" + std::to_string(this->m_fd));
        }
        return CommunicatorHandleDelete;
    }
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
        return CommunicatorHandleOK;
    }

    int ret = write(this->m_fd, m_sendBuf.c_str(), m_sendBuf.size());
    if (ret < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            LOGGER.Log(ERROR, "[HttpCommunicator]write error. fd=" + std::to_string(this->m_fd));
            return CommunicatorHandleDelete;
        }
    }
    if (ret == m_sendBuf.size()) {
        LOGGER.Log(DEBUG, "[HttpCommunicator]write msg. fd=" + std::to_string(this->m_fd) + " msg=" + m_sendBuf);
        LOGGER_Write.Log(DEBUG, "[HttpCommunicator]write msg. fd=" + std::to_string(this->m_fd) + " msg=" + m_sendBuf);
        LOGGER_RW.Log(DEBUG, "[HttpCommunicator]write msg. fd=" + std::to_string(this->m_fd) + " msg=" + m_sendBuf);
        m_sendBuf.clear();
    } else {
        LOGGER.Log(DEBUG, "[HttpCommunicator]write msg. fd=" + std::to_string(this->m_fd) + " msg=" + m_sendBuf.substr(0, ret));
        LOGGER_Write.Log(DEBUG, "[HttpCommunicator]write msg. fd=" + std::to_string(this->m_fd) + " msg=" + m_sendBuf.substr(0, ret));
        LOGGER_RW.Log(DEBUG, "[HttpCommunicator]write msg. fd=" + std::to_string(this->m_fd) + " msg=" + m_sendBuf.substr(0, ret));
        m_sendBuf = m_sendBuf.substr(ret);
    }
    return CommunicatorHandleOK;
}

CommunicatorHandleResult HttpCommunicator::HandleSocketError()
{
    LOGGER.Log(ERROR, "[HttpCommunicator]socket error. fd=" + std::to_string(this->m_fd) + " error=" + std::to_string(errno));
    return CommunicatorHandleOK;
}