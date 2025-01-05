#include "Communicator.h"
#include "App.h"

#include "RawMsgManager.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

#ifndef HTTPCOMMUNICATOR
#define HTTPCOMMUNICATOR

class HttpCommunicator : public Communicator
{
    HttpRequest m_httpRequest;
    HttpResponse m_httpResponse;
    App *m_app;
    std::string m_sendBuf;
    void HandleRequest();
public:
    HttpCommunicator(int fd, App *app=nullptr);
    ~HttpCommunicator() override;
    CommunicatorHandleResult HandleSocketRead() override;
    CommunicatorHandleResult HandleSocketWrite() override;
    CommunicatorHandleResult HandleSocketError() override;
    bool IsNeedSendData() override;
    HttpRequest &GetHttpRequest();
    HttpResponse &GetHttpResponse();

};

#endif
