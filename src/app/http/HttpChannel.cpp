#include "HttpChannel.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "Logger.h"

HttpChannel::HttpChannel(int fd, App *app) :
    Channel(fd), m_app(app), m_sendBuf()
{
    m_sendBuf.reserve(4 * 1024);
}

HttpChannel::~HttpChannel()
{
}

HttpRequest &HttpChannel::GetHttpRequest()
{
    return m_httpRequest;
}

HttpResponse &HttpChannel::GetHttpResponse()
{
    return m_httpResponse;
}

void HttpChannel::HandleRequest()
{
    if (m_app == nullptr) {
        return;
    }
    if (m_httpRequest.IsFinshed()) {
        m_app->Update(this);
        return;
    }
}

ChannelHandleResult HttpChannel::HandleSocketRead()
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
            return ChannelHandleOK;
        }
        if (ret == 0) {
            LOGGER.Log(INFO, "[HttpChannel]client close. fd=" + std::to_string(this->m_fd));  
        } else {
            LOGGER.Log(ERROR, "[HttpChannel]read error. fd=" + std::to_string(this->m_fd));
        }
        FreeMsg(msg);
        return ChannelHandleDelete;
    }

    FreeMsg(msg);
    return ChannelHandleOK;
}
ChannelHandleResult HttpChannel::HandleSocketWrite()
{
    if (m_httpResponse.IsReady() && !m_httpResponse.IsSending()) {
        std::string response = m_httpResponse.MakeResponse();
        m_sendBuf += response;
        m_httpResponse.SetSending(true);
        if (m_httpResponse.GetSendSrc() == HTTPRES_SRC_FILE) {
            int on = 1;
            (void)setsockopt(this->m_fd, IPPROTO_TCP, TCP_CORK, &on, sizeof(on));
        }
    }

    if (m_sendBuf.empty()) {
        if (m_httpResponse.IsReady() &&
            m_httpResponse.IsSending() &&
            !m_httpResponse.IsSendFileEnd()) { // 就绪并且头部发送完了
            off_t filePos = m_httpResponse.GetSendFilePos();
            ssize_t retSize = sendfile(this->m_fd, m_httpResponse.GetSendFIleFd(), &filePos, m_httpResponse.GetSendFileSize() - filePos);
            if (retSize < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    LOGGER.Log(ERROR, "[HttpChannel]sendfile error. fd=" + std::to_string(this->m_fd));
                    return ChannelHandleDelete;
                }
            }
            m_httpResponse.SetSendFilePos(filePos);
            if (m_httpResponse.IsSendFileEnd()) {
                int on = 0;
                (void)setsockopt(this->m_fd, IPPROTO_TCP, TCP_CORK, &on, sizeof(on));
            }
        }
        return ChannelHandleOK;
    }

    ssize_t ret = write(this->m_fd, m_sendBuf.c_str(), m_sendBuf.size());
    if (ret < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            LOGGER.Log(ERROR, "[HttpChannel]write error. fd=" + std::to_string(this->m_fd));
            return ChannelHandleDelete;
        }
    }
    if (ret == m_sendBuf.size()) {
        m_sendBuf.clear();
    } else {
        m_sendBuf = m_sendBuf.substr(ret);
    }
    return ChannelHandleOK;
}

ChannelHandleResult HttpChannel::HandleSocketError()
{
    LOGGER.Log(ERROR, "[HttpChannel]socket error. fd=" + std::to_string(this->m_fd) + " error=" + std::to_string(errno));
    return ChannelHandleOK;
}

bool HttpChannel::IsNeedSendData()
{
    if (!m_httpRequest.IsFinshed() || !m_httpResponse.IsReady()) {
        return false;
    }
    if (m_httpResponse.IsReady() && !m_httpResponse.IsSending()) {
        std::string response = m_httpResponse.MakeResponse();
        m_sendBuf += response;
        m_httpResponse.SetSending(true);
        if (m_httpResponse.GetSendSrc() == HTTPRES_SRC_FILE) {
            int on = 1;
            (void)setsockopt(this->m_fd, IPPROTO_TCP, TCP_CORK, &on, sizeof(on));
        }
    }

    if (!m_sendBuf.empty()) {
        return true;
    }
    if (m_httpResponse.GetSendSrc() == HTTPRES_SRC_FILE) {
        return !m_httpResponse.IsSendFileEnd();
    }
    return false;
}