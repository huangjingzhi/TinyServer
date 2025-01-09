
#include <iostream>
#include <sys/stat.h>
#include "HttpServer.h"
#include "HttpServerInfo.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "commom/Logger.h"
#include <iostream>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
using namespace std;

void ViewBase(const HttpServerInfo *httpServerInfo, const HttpRequest &resq, HttpResponse &response)
{
    response.SetBody("Hello World!");
    response.SetStatus(HTTPRES_CODE_OK);
    response.AddHeader("Content-Type", "text/html");
}

void ViewIndex(const HttpServerInfo *httpServerInfo, const HttpRequest &resq, HttpResponse &response)
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

void ViewVideo(const HttpServerInfo *httpServerInfo, const HttpRequest &resq, HttpResponse &response)
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

void ViewVideo1Mp4(const HttpServerInfo *httpServerInfo, const HttpRequest &resq, HttpResponse &response)
{
    std::string filePath =  httpServerInfo->sourceDir +  "/video/3.mp4";

    if (response.SetSendFile(filePath) == false) {
        response.SetStatus(HTTPRES_CODE_SERVER_ERROR);
        response.SetBody( "file not found.");
        response.AddHeader("Content-Type", "text/html");
        return;
    }
    response.SetStatus(HTTPRES_CODE_OK);
    response.AddHeader("Content-Type", "video/mp4");
}

void ViewVideo1Mp4Buffer(const HttpServerInfo *httpServerInfo, const HttpRequest &resq, HttpResponse &response)
{
    std::string filePath =  httpServerInfo->sourceDir +  "/video/3.mp4";

    if (response.LoadFileToBody(filePath) == false) {
        response.SetStatus(HTTPRES_CODE_SERVER_ERROR);
        response.SetBody( "file not found.");
        response.AddHeader("Content-Type", "text/html");
        return;
    }

    response.SetStatus(HTTPRES_CODE_OK);
    response.AddHeader("Content-Type", "video/mp4");
}

void ViewImage4(const HttpServerInfo *httpServerInfo, const HttpRequest &resq, HttpResponse &response)
{
    std::string filePath = httpServerInfo->sourceDir + "/images/instagram-image3.jpg";

    if (response.LoadFileToBody(filePath) == false) {
        response.SetStatus(HTTPRES_CODE_SERVER_ERROR);
        response.SetBody( "file not found.");
        response.AddHeader("Content-Type", "text/html");
        return;
    }

    response.SetStatus(HTTPRES_CODE_OK);
    response.AddHeader("Content-Type", "image/jpg");
}


void PrintStackTrace() {
    const int max_frames = 1024;
    void* buffer[max_frames];
    int num_frames = backtrace(buffer, max_frames);
    char** symbols = backtrace_symbols(buffer, num_frames);

    std::cerr << "Stack trace:" << std::endl;
    for (int i = 0; i < num_frames; ++i) {
        std::cerr << symbols[i] << std::endl;
    }

    free(symbols);
}

void SignalHandle(int signum) {
    std::cerr << "Error: signal " << signum << std::endl;
    PrintStackTrace();
    exit(signum);
}

int main()
{
    signal(SIGSEGV, SignalHandle);
    signal(SIGABRT, SignalHandle);

    try {
        LOGGER.SetLogLevel(LogLevel::INFO);

        HttpServerInfo httpServerInfo;
        httpServerInfo.sourceDir = "/home/hjz/netserver/src/resources";
        httpServerInfo.httpRequestHandles["/"] = ViewIndex;
        httpServerInfo.httpRequestHandles["/index"] = ViewIndex;
        httpServerInfo.httpRequestHandles["/video"] = ViewVideo;
        httpServerInfo.httpRequestHandles["/video.mp4"] = ViewVideo1Mp4;
        // httpServerInfo.httpRequestHandles["/video.mp4"] = ViewVideo1Mp4Buffer;
        httpServerInfo.httpRequestHandles["/images4"] = ViewImage4;
        HttpServer httpServer(8005, 10, 1024);
        httpServer.InitStaticInfo(httpServerInfo);
        httpServer.Start();
        httpServer.JoinThreads();

    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        PrintStackTrace();
    }

    return 0;
}