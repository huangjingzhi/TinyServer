#include "Channel.h"
#include "App.h"

#include "RawMsgManager.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

#ifndef HTTPCHANNEL
#define HTTPCHANNEL

class HttpChannel : public Channel
{
    HttpRequest m_httpRequest;
    HttpResponse m_httpResponse;
    App *m_app;
    std::string m_sendBuf;
    void HandleRequest();
public:
    HttpChannel(int fd, App *app=nullptr);
    ~HttpChannel() override;
    ChannelHandleResult HandleSocketRead() override;
    ChannelHandleResult HandleSocketWrite() override;
    ChannelHandleResult HandleSocketError() override;
    bool IsNeedSendData() override;
    HttpRequest &GetHttpRequest();
    HttpResponse &GetHttpResponse();

};

#endif
