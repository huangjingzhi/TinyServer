
#include <iostream>
#include <sys/stat.h>
#include "app/http/HttpServer.h"
#include "app/http/HttpServerInfo.h"
#include "app/http/HttpRequest.h"
#include "app/http/HttpResponse.h"
using namespace std;

void view_index(const HttpServerInfo *httpServerInfo, const HttpRequest &resq, HttpResponse &response)
{
    std::string filePath =  httpServerInfo->sourceDir +  "/index.html";

    struct stat file;
    if (stat(filePath.c_str(), &file) != 0) {
        response.SetStatus(HTTPRES_CODE_SERVER_ERROR);
        response.SetBody( "file not found.");
        return ;
    }
    response.SetStatus(HTTPRES_CODE_OK);
    response.LoadFileToBody(filePath);
}

int main()
{
    HttpServerInfo httpServerInfo;
    httpServerInfo.sourceDir = "/home/zheng/CLionProjects/HttpServer";
    httpServerInfo.httpRequestHandles["/"] = view_index;
    HttpServer httpServer(8080, 1, 1024);
    httpServer.InitStaticInfo(httpServerInfo);
    // httpServer.Start();
    return 0;
}