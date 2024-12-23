#include "HttpRequest.h"
#include <string>
#include <algorithm>
#include <regex>


const std::string CRLF = "\r\n";
int ConverHex(char ch) {
    if(ch >= 'A' && ch <= 'F') return ch -'A' + 10;
    if(ch >= 'a' && ch <= 'f') return ch -'a' + 10;
    return ch;
}

HttpRequest::HttpRequest() : m_rawMsgbuf(0)
{
}

HttpRequest::~HttpRequest()
{
}

void HttpRequest::PutRawMsg(Msg &rawMsg)
{
    m_rawMsgbuf.insert(m_rawMsgbuf.begin(), rawMsg.buf, rawMsg.buf + rawMsg.len);
}

bool HttpRequest::ParseLine(const std::string &lineData)
{
    std::regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch subMatch;
    if(std::regex_match(lineData, subMatch, patten)) {   
        m_method = subMatch[1];
        m_path = subMatch[2];
        m_version = subMatch[3];
        m_parseState = HTTPPARSE_HEADERS;
        return true;
    }
    m_parseState = HTTPPARSE_ERROR;
    return false;
}
void HttpRequest::ParseHeaders(const std::string &lineData)
{
    std::regex patten("^([^:]*): ?(.*)$");
    std::smatch subMatch;
    if(std::regex_match(lineData, subMatch, patten)) {
        m_headers[subMatch[1]] = subMatch[2];
    }
    else {
        m_parseState = HTTPPARSE_BODY;
    }
}

void HttpRequest::ParseBody(const std::string &lineData)
{
    m_body = lineData;
    ParsePost();
    m_parseState = HTTPPARSE_FINISH;
}


void HttpRequest::ParsePost()
{
    if(m_method == "POST" && m_headers["Content-Type"] == "application/x-www-form-urlencoded") {
        ParseFromUrlencoded();
    }  
}

void HttpRequest::ParseFromUrlencoded() {
    if(m_body.empty() == 0) { 
        return;
    }

    std::string key, value;
    int num = 0;
    int n = m_body.size();
    int i = 0, j = 0;

    for(; i < n; i++) {
        char ch = m_body[i];
        switch (ch) {
        case '=':
            key = m_body.substr(j, i - j);
            j = i + 1;
            break;
        case '+':
            m_body[i] = ' ';
            break;
        case '%':
            num = ConverHex(m_body[i + 1]) * 16 + ConverHex(m_body[i + 2]);
            m_body[i + 2] = num % 10 + '0';
            m_body[i + 1] = num / 10 + '0';
            i += 2;
            break;
        case '&':
            value = m_body.substr(j, i - j);
            j = i + 1;
            m_post[key] = value;
            break;
        default:
            break;
        }
    }
    if(m_post.count(key) == 0 && j < i) {
        value = m_body.substr(j, i - j);
        m_post[key] = value;
    }
}

void HttpRequest::ParseRawMsg()
{
    if (this->m_rawMsgbuf.empty()) {
        return;
    }
    while ((!m_rawMsgbuf.empty()) &&  (m_parseState < HTTPPARSE_FINISH)) {
        auto it = std::search(m_rawMsgbuf.begin(), m_rawMsgbuf.end(), CRLF.begin(), CRLF.end());
        if (it == m_rawMsgbuf.end()) {
            break;
        }
        std::string curData(m_rawMsgbuf.begin(),  it); // curData 可以使用std::move传入解析层
        switch (m_parseState) {
            case HTTPPARSE_LINE: {
                this->ParseLine(curData);
                break;
            }
            case HTTPPARSE_HEADERS: {
                this->ParseHeaders(curData);
                break;
            }
            case HTTPPARSE_BODY: {
                this->ParseBody(curData);
                break;
            }
            default:
                break;
        }
        m_rawMsgbuf.erase(m_rawMsgbuf.begin(), it);
    }

    return;
}

std::string HttpRequest::GetPath() {
    return m_path;
}
std::string HttpRequest::GetMethod() {
    return m_method;
}
bool HttpRequest::GetHead(const std::string &key, std::string &value) {
    auto it = m_headers.find(key);
    if (it == m_headers.end()) {
        return false;
    }
    value = m_headers[key];
    return true;
}
bool HttpRequest::GetPost(const std::string &key, std::string &value) {
    auto it = m_post.find(key);
    if (it == m_post.end()) {
        return false;
    }
    value = m_post[key];
    return true;
}
