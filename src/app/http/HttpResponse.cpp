
#include "HttpResponse.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "Logger.h"
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define HTTP_LOAD_MAX_FILE_SIZE 1024 * 1024 * 4

HttpResponse::HttpResponse():
    m_resCode(HTTPRES_CODE_DEFAULT), m_resType(HTTPRES_TYPE_DEFAULT), m_ready(false), m_sending(false),
    m_status(""), m_headers(), m_body(), m_filePath(""), m_fileSize(0), m_filePos(0),m_resSrc(HTTPRES_SRC_MEM), m_fd(-1) 
{
}

HttpResponse::~HttpResponse()
{
    if (m_fd != -1) {
        close(m_fd);
    }
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
    m_headers["Content-length"] = std::to_string(m_body.size());
    return true;
}

std::string HttpResponse::MakeResponse()
{
    if (m_resSrc == HTTPRES_SRC_FILE) {
        if (m_fd == -1) {
            LOGGER.Log(ERROR, "[HttpResponse]file not open. file=" + m_filePath);
            return "";
        }
        if (m_filePos > m_fileSize) {
            close(m_fd);
            m_fd = -1;
            return "";
        }
        m_headers["Content-length"] = std::to_string(m_fileSize);
    } else {
        m_headers["Content-length"] = std::to_string(m_body.size());
    }

    std::string response = m_status;
    response.reserve(m_status.size() + m_headers.size() * 512);
    for (const auto &pair : m_headers) {
        response += pair.first + ": " + pair.second + "\r\n";
    }
    response += "\r\n";

    if (m_resSrc == HTTPRES_SRC_MEM) {
        response += m_body;
    }

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

bool HttpResponse::SetSendFile(const std::string &filePath)
{
    m_filePath = filePath;
    m_fd = open(filePath.c_str(), O_RDONLY);
    if (m_fd == -1) {
        LOGGER.Log(ERROR, "[HttpResponse]open file error. file=" + filePath);
        return false;
    }
    struct stat file_stat;
    if (fstat(m_fd, &file_stat) < 0) {
        LOGGER.Log(ERROR, "[HttpResponse]fstat file error. file=" + filePath);
        return false;
    }
    m_fileSize = file_stat.st_size;
    m_filePos = 0;
    m_resSrc = HTTPRES_SRC_FILE;
    if (m_headers["Content-Type"] == "") {
        m_headers["Connection"] = "keep-alive";
        m_headers["Content-Range"] = "bytes " + std::to_string(m_filePos) + "-" + std::to_string(m_fileSize) + "/" + std::to_string(m_fileSize);
    }
    return true;
}

int HttpResponse::GetSendFIleFd()
{
    return m_fd;
}

off_t HttpResponse::GetSendFilePos()
{
    return m_filePos;
}
off_t HttpResponse::GetSendFileSize()
{
    return m_fileSize;
}
void HttpResponse::SetSendFilePos(off_t pos)
{
    m_filePos = pos;
}

bool HttpResponse::IsSendFileEnd()
{
    return m_filePos >= m_fileSize;
}

HttpResSrc HttpResponse::GetSendSrc()
{
    return m_resSrc;
}