
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



void view_base(const HttpServerInfo *httpServerInfo, const HttpRequest &resq, HttpResponse &response)
{
    response.SetBody("Hello World!");
    response.SetStatus(HTTPRES_CODE_OK);
    response.AddHeader("Content-Type", "text/html");
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

void view_video1mp4buffer(const HttpServerInfo *httpServerInfo, const HttpRequest &resq, HttpResponse &response)
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

void view_image4(const HttpServerInfo *httpServerInfo, const HttpRequest &resq, HttpResponse &response)
{
    std::string filePath = httpServerInfo->sourceDir + "/images/instagram-image4.jpg";

    if (response.LoadFileToBody(filePath) == false) {
        response.SetStatus(HTTPRES_CODE_SERVER_ERROR);
        response.SetBody( "file not found.");
        response.AddHeader("Content-Type", "text/html");
        return;
    }

    response.SetStatus(HTTPRES_CODE_OK);
    response.AddHeader("Content-Type", "image/jpg");
}


void print_stack_trace() {
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

void signal_handler(int signum) {
    std::cerr << "Error: signal " << signum << std::endl;
    print_stack_trace();
    exit(signum);
}

int main()
{

    signal(SIGSEGV, signal_handler); // 捕获段错误信号
    signal(SIGABRT, signal_handler); // 捕获中止信号


    try {
        LOGGER.SetLogLevel(LogLevel::INFO);

        HttpServerInfo httpServerInfo;
        httpServerInfo.sourceDir = "/home/hjz/netserver/src/resources";
        httpServerInfo.httpRequestHandles["/"] = view_index;
        httpServerInfo.httpRequestHandles["/index"] = view_index;
        httpServerInfo.httpRequestHandles["/video"] = view_video;
        httpServerInfo.httpRequestHandles["/video.mp4"] = view_video1mp4;
        // httpServerInfo.httpRequestHandles["/video.mp4"] = view_video1mp4buffer;
        httpServerInfo.httpRequestHandles["/images4"] = view_image4;
        HttpServer httpServer(8005, 2, 20);
        httpServer.InitStaticInfo(httpServerInfo);
        httpServer.Start();
        httpServer.JoinThreads();
    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        print_stack_trace();
    }


    return 0;
}