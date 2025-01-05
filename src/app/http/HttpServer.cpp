#include "HttpServer.h"
#include "Logger.h"

HttpServer::HttpServer(int port, int netIoworkerNumber, int workerMaxFd):
    m_port(port),m_netIoworkerNumber(netIoworkerNumber),m_workerMaxFd(workerMaxFd),
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
    this->m_httpConnectManager.reset(new ConnectManage<HttpCommunicator>(
        m_port, m_netIoworkerNumber, m_workerMaxFd, this));
    this->m_httpConnectManager->Init();
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
        response.SetBody("Path " + request.GetPath() +  " Not Found");
        response.SetResType(HTTPRES_TYPE_HTML);
        response.SetReady(true);
        LOGGER.Log(ERROR, "[HttpServer]path not found. path=" + request.GetPath());
        return;
    }
    it->second(&this->m_httpServerInfo, request, response);
    response.SetReady(true);
}

void HttpServer::JoinThreads()
{
    this->m_httpConnectManager->JoinThreads();
}