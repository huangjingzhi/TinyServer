
#include <string>
#include <unordered_map>
#include "HttpRequest.h"
#include "HttpResponse.h"

#ifndef HTTPSERVERINFO_H
#define HTTPSERVERINFO_H

struct st_HttpServerInfo;
typedef struct st_HttpServerInfo  HttpServerInfo;
typedef void (*HttpRequestHandle)
            (const HttpServerInfo *httpServerInfo,const HttpRequest &request, HttpResponse &response);

typedef struct st_HttpServerInfo
{
    std::string  sourceDir;
    std::unordered_map<std::string, HttpRequestHandle> httpRequestHandles;
} HttpServerInfo;

#endif

