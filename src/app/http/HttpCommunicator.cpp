#include "HttpCommunicator.h"
#include <stdio.h>
#include <stdlib.h>
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
    Msg msg(8 * 1024); // TODO 这里每次都要申请一次内存，是否有优化的空间
    int ret = read(this->m_fd, msg.buf, msg.maxLen);
    if (ret > 0) {
        msg.len = ret;
        m_httpRequest.PutRawMsg(msg);
        m_httpRequest.ParseRawMsg();
        HandleRequest();
    } else {
        if (errno == EAGAIN) {
            FreeMsg(msg);
            return CommunicatorHandleOK;
        }
        FreeMsg(msg);
        return CommunicatorHandleDelete;
    }
    return CommunicatorHandleOK;
}
CommunicatorHandleResult HttpCommunicator::HandleSocketWrite()
{
    if (m_httpResponse.IsReady()) {
        std::string response = m_httpResponse.MakeResponse();
        m_sendBuf += response;

    }
    if (m_sendBuf.empty()) {
        return CommunicatorHandleOK;
    }
    int ret = write(this->m_fd, m_sendBuf.c_str(), m_sendBuf.size());
    if (ret < 0) {
        return CommunicatorHandleDelete;
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
    return CommunicatorHandleOK;
}
