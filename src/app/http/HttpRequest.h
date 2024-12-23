#include "../../commom/Msg.h"
#include <vector>
#include <string>
#include <map>

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

enum HttpParseState {
    HTTPPARSE_LINE = 0,
    HTTPPARSE_HEADERS,
    HTTPPARSE_BODY,
    HTTPPARSE_FINISH,
    HTTPPARSE_ERROR,
    HTTPPARSE_MAX
};

enum HttpCode {
    HTTPCODE_NO_REQUEST = 0,
    HTTPCODE_GET_REQUEST,
    HTTPCODE_BAD_REQUEST,
    HTTPCODE_NO_RESOURSE,
    HTTPCODE_FORBIDDENT_REQUEST,
    HTTPCODE_FILE_REQUEST,
    HTTPCODE_INTERNAL_ERROR,
    HTTPCODE_CLOSED_CONNECTION,
    HTTPCODE_MAX
};

class HttpRequest
{
private:
    std::vector<char> m_rawMsgbuf;
    HttpParseState m_parseState;
    std::string m_path;
    std::string m_method;
    std::string m_version;
    std::string m_body;
    std::map<std::string, std::string> m_headers;
    std::map<std::string, std::string> m_post;

    bool ParseLine(const std::string &lineData);
    void ParseHeaders(const std::string &lineData);
    void ParseBody(const std::string &lineData);
    void ParsePost();
    void ParseFromUrlencoded();
public:
    HttpRequest();
    ~HttpRequest();
    void PutRawMsg(Msg &rawMsg);
    void ParseRawMsg();

    std::string GetPath();
    std::string GetMethod();
    bool GetHead(const std::string &key, std::string &value);
    bool GetPost(const std::string &key, std::string &value);
};




#endif