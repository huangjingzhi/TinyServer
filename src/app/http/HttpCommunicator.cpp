#include "HttpCommunicator.h"

HttpCommunicator::HttpCommunicator(int fd, App *app=nullptr) : Communicator(fd), m_app(app)
{

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
    m_app->Update(this);
}
