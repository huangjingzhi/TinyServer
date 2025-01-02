
#include "HttpResponse.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "../../commom/Logger.h"
#define HTTP_LOAD_MAX_FILE_SIZE 1024 * 1024 * 4

HttpResponse::HttpResponse(): 
    m_ready(false), m_resCode(HTTPRES_CODE_DEFAULT), m_resType(HTTPRES_TYPE_DEFAULT)
{
}

std::string HttpResponse::CodeToStr(HttpResCode code)
{
    // todo: 改成成员列表
    switch (code) {
        case HTTPRES_CODE_OK:
            return "OK";
        case HTTPRES_CODE_BADREQUEST:
            return "Bad Request";
        case HTTPRES_CODE_FORBIDDEN:
            return "Forbidden";
        case HTTPRES_CODE_NOTFOUND:
            return "Not Found";
        case HTTPRES_CODE_SERVER_ERROR:
            return "Internal Server Error";
    }
    return "";
}

std::string HttpResponse::TypeToStr(HttpResType type)
{
    // todo: 改成成员列表
    switch (type) {
        case HTTPRES_TYPE_HTML:
        case HTTPRES_TYPE_DEFAULT:
            return "text/html";
            break;
        case HTTPRES_TYPE_XML:
            return "text/xml";
            break;
        case HTTPRES_TYPE_PNG:
            return "image/png";
            break;
    }
    return "";
}


bool HttpResponse::SetStatus(HttpResCode code)
{
    std::string stStr = std::to_string(code);
    if (stStr.empty()) {
        return false;
    }
    m_status = "HTTP/1.1 " + stStr + " " + CodeToStr(code) + "\r\n";

    return true;
}

bool HttpResponse::AddStatus(const std::string &status)
{
    m_status = status;
    return false;
}
bool HttpResponse::AddHeader(const std::string &key, const std::string &value)
{
    m_headers[key] = value;
    return true;
}

void HttpResponse::AddBody(const std::string &data)
{
    m_body.insert(m_body.end(), data.begin(), data.end());
    m_headers["Content-length"] = std::to_string(m_body.size());
}
void HttpResponse::SetBody(const std::string &body)
{
    m_body = body;
    m_headers["Content-length"] = std::to_string(m_body.size());
}

bool HttpResponse::LoadFileToBody(const std::string &filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    m_body = buffer.str();
    file.close();
    return true;
}

std::string HttpResponse::MakeResponse()
{
    std::string response = m_status;
    response.reserve(m_status.size() + m_headers.size() * 512 + m_body.size());
    m_headers["Content-length"] = std::to_string(m_body.size());
    for (const auto &pair : m_headers) {
        response += pair.first + ": " + pair.second + "\r\n";
    }
    LOGGER.Log(DEBUG, "[HttpReponse]is finished. resp:\n" + m_body);
    response += "\r\n" + m_body;
    return std::move(response);
}

bool HttpResponse::IsReady() {
    return m_ready;
}
void HttpResponse::SetReady(bool ready) {
    m_ready = ready;
}

void HttpResponse::SetResType(const HttpResType type)
{
    m_resType = type;
    m_headers["Content-type"] = TypeToStr(type);
}

bool HttpResponse::IsSending()
{
    return m_sending;
}

void HttpResponse::SetSending(bool sending)
{
    m_sending = sending;
}