
#include <iostream>
#include <sys/stat.h>
#include "app/http/HttpServer.h"
#include "app/http/HttpServerInfo.h"
#include "app/http/HttpRequest.h"
#include "app/http/HttpResponse.h"
#include "commom/Logger.h"
using namespace std;

Logger LOGGER_view_indexCnt{"view_index.log", DEBUG};
std::mutex g_mutex;
int g_cnt = 0;

void view_base(const HttpServerInfo *httpServerInfo, const HttpRequest &resq, HttpResponse &response)
{
    response.SetBody("Hello World!");
    response.SetStatus(HTTPRES_CODE_OK);
    response.AddHeader("Content-Type", "text/html");
    {
        std::unique_lock<std::mutex> lock(g_mutex);
        g_cnt++;
        LOGGER_view_indexCnt.Log(DEBUG, "[view] cnt=" + std::to_string(g_cnt));
    }
}

void view_index(const HttpServerInfo *httpServerInfo, const HttpRequest &resq, HttpResponse &response)
{
    std::string filePath =  httpServerInfo->sourceDir +  "/html/index.html";

    struct stat file;
    if (stat(filePath.c_str(), &file) != 0) {
        response.SetStatus(HTTPRES_CODE_SERVER_ERROR);
        response.SetBody( "file not found.");
        return;
    }
    if (response.SetSendFile(filePath) == false) {
        response.SetStatus(HTTPRES_CODE_SERVER_ERROR);
        response.SetBody( "file not found.");
        response.AddHeader("Content-Type", "text/html");
        return;
    }
    response.SetStatus(HTTPRES_CODE_OK);
    response.AddHeader("Content-Type", "text/html");
}

void view_video(const HttpServerInfo *httpServerInfo, const HttpRequest &resq, HttpResponse &response)
{
    std::string filePath =  httpServerInfo->sourceDir +  "/html/video.html";

    if (response.SetSendFile(filePath) == false) {
        response.SetStatus(HTTPRES_CODE_SERVER_ERROR);
        response.SetBody( "file not found.");
        response.AddHeader("Content-Type", "text/html");
        return;
    }
    response.SetStatus(HTTPRES_CODE_OK);
    response.AddHeader("Content-Type", "text/html");
}

void view_video1mp4(const HttpServerInfo *httpServerInfo, const HttpRequest &resq, HttpResponse &response)
{
    std::string filePath =  httpServerInfo->sourceDir +  "/video/1.mp4";

    if (response.SetSendFile(filePath) == false) {
        response.SetStatus(HTTPRES_CODE_SERVER_ERROR);
        response.SetBody( "file not found.");
        response.AddHeader("Content-Type", "text/html");
        return;
    }
    response.SetStatus(HTTPRES_CODE_OK);
    response.AddHeader("Content-Type", "video/mp4");
}

void view_video1mp4buffer(const HttpServerInfo *httpServerInfo, const HttpRequest &resq, HttpResponse &response)
{
    std::string filePath =  httpServerInfo->sourceDir +  "/video/1.mp4";

    if (response.LoadFileToBody(filePath) == false) {
        response.SetStatus(HTTPRES_CODE_SERVER_ERROR);
        response.SetBody( "file not found.");
        response.AddHeader("Content-Type", "text/html");
        return;
    }

    response.SetStatus(HTTPRES_CODE_OK);
    response.AddHeader("Content-Type", "video/mp4");
}

int main()
{
    LOGGER.SetLogLevel(LogLevel::INFO);

    HttpServerInfo httpServerInfo;
    httpServerInfo.sourceDir = "/home/hjz/netserver/src/resources";
    httpServerInfo.httpRequestHandles["/"] = view_index;
    httpServerInfo.httpRequestHandles["/index"] = view_index;
    httpServerInfo.httpRequestHandles["/video"] = view_video;
    // httpServerInfo.httpRequestHandles["/video.mp4"] = view_video1mp4
    httpServerInfo.httpRequestHandles["/video.mp4"] = view_video1mp4buffer;
    HttpServer httpServer(8005, 1, 20);
    httpServer.InitStaticInfo(httpServerInfo);
    httpServer.Start();
    httpServer.JoinThreads();
    return 0;
}