#include "HttpServer.h"

HttpServer::HttpServer(int port, int netIoworkerNumber, int workerMaxFd):
    m_httpConnectManager(nullptr)
{

}
HttpServer::~HttpServer()
{

}
void HttpServer::InitStaticInfo(const HttpServerInfo &httpServerInfo)
{
    this->m_httpServerInfo.sourceDir = httpServerInfo.sourceDir;
    this->m_httpServerInfo.httpRequestHandles = httpServerInfo.httpRequestHandles;
}

void HttpServer::Start()
{
    this->m_httpConnectManager.reset(new ConnectManage<HttpCommunicator>(8080, 1, 1024, this));
    this->m_httpConnectManager->Init();
    this->m_httpConnectManager->JoinThreads();
}
void HttpServer::Update(Communicator *communicator)
{
    HttpCommunicator *httpCommunicator = dynamic_cast<HttpCommunicator *>(communicator);
    if (httpCommunicator == nullptr) {
        return;
    }
    HttpRequest &request = httpCommunicator->GetHttpRequest();
    HttpResponse &response = httpCommunicator->GetHttpResponse();
    auto it = this->m_httpServerInfo.httpRequestHandles.find(request.GetPath());
    if (it == this->m_httpServerInfo.httpRequestHandles.end()) {
        response.SetStatus(HTTPRES_CODE_NOTFOUND);
        response.SetBody("Not Found");
        response.SetResType(HTTPRES_TYPE_HTML);
        response.SetReady(true);
        return;
    }
    it->second(&this->m_httpServerInfo, request, response);
    response.SetReady(true);
}