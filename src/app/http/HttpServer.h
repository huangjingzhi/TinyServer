
#include "HttpRequest.h"
#include "HttpChannel.h"
#include "HttpResponse.h"
#include "HttpServerInfo.h"
#include "ConnectManage.h"
#include "App.h"

#include <unordered_map>
#include <memory>

#ifndef HTTPSERVER_H
#define HTTPSERVER_H


class HttpServer: public App
{
    int m_port;
    int m_netIoworkerNumber;
    int m_workerMaxFd;
    std::unique_ptr<ConnectManage<HttpChannel>> m_httpConnectManager;
    HttpServerInfo m_httpServerInfo;
public:
    HttpServer(int port, int netIoworkerNumber, int workerMaxFd);
    ~HttpServer();
    void Start();
    void InitStaticInfo(const HttpServerInfo &httpServerInfo);
    virtual void Update(Channel *channel) override;
    void JoinThreads();
};

#endif