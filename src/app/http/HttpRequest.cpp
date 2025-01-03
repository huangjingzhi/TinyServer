#include "HttpRequest.h"
#include <string>
#include <algorithm>
#include <regex>
#include "../../commom/Logger.h"

const std::string CRLF = "\r\n";
int ConverHex(char ch) {
    if(ch >= 'A' && ch <= 'F') return ch -'A' + 10;
    if(ch >= 'a' && ch <= 'f') return ch -'a' + 10;
    return ch;
}
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

HttpRequest::HttpRequest() : m_rawMsgbuf(0),
    m_parseState(HTTPPARSE_LINE), m_path(""), m_method(""), m_version(""), m_body(""), m_headers(), m_post()
{
}

HttpRequest::~HttpRequest()
{
}

void HttpRequest::PutRawMsg(Msg &rawMsg)
{
    m_rawMsgbuf.insert(m_rawMsgbuf.begin(), rawMsg.buf, rawMsg.buf + rawMsg.len);
    LOGGER.Log(DEBUG, "[HttpRequest]put raw msg. msg=" + std::string(rawMsg.buf, rawMsg.buf + rawMsg.len));
}

bool HttpRequest::ParseLine(const std::string &lineData)
{
    LOGGER.Log(DEBUG, "[HttpRequest]parse line. lineData=" + lineData);
    std::istringstream iss(lineData);
    std::vector<std::string> words;
    std::string word;
    std::string wordStr;
    while (iss >> word) {
        words.push_back(word);
        wordStr += word + " ";
    }
    LOGGER.Log(DEBUG, "[HttpRequest]parse line. words=" + wordStr);
    if (words.size() != 3) {
        LOGGER.Log(ERROR, "[HttpRequest]parse line error. words.size()=" + std::to_string(words.size())
        + " lineData=" + lineData
        + " words=" + wordStr);
        m_parseState = HTTPPARSE_ERROR;
        return false;
    }
    
    m_method = words[0];
    m_path = words[1];
    m_version = words[2];
    m_parseState = HTTPPARSE_HEADERS;
    return true;
}
void HttpRequest::ParseHeaders(const std::string &lineData)
{

    size_t pos = lineData.find(":");
    if (pos == std::string::npos) {
        m_parseState = HTTPPARSE_BODY;
        return;
    }
    std::string val = lineData.substr(pos + 1);
    std::string key =  lineData.substr(0, pos);
    trim(val);
    trim(key);
    m_headers[key] = val;
    LOGGER.Log(DEBUG, "[HttpRequest]parse headers. key=" + key + " val=" + val);
}

void HttpRequest::ParseBody(const std::string &lineData)
{
    size_t bodyLen = GetContentLength();
    if (bodyLen == m_body.size()) {
        m_parseState = HTTPPARSE_FINISH;
        return;
    }
    m_body += lineData;
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
        std::string curData(m_rawMsgbuf.begin(),  it + CRLF.length()); // curData 可以使用std::move传入解析层
        
        switch (m_parseState) {
            case HTTPPARSE_LINE: {
                this->ParseLine(curData);
                break;
            }
            case HTTPPARSE_HEADERS: {
                this->ParseHeaders(curData);
                if (curData == CRLF) {
                    m_parseState = HTTPPARSE_BODY;
                    // 如果没有数据长度，直接结束
                    if (GetContentLength() == 0) {
                        m_parseState = HTTPPARSE_FINISH;
                    }
                }
                break;
            }
            case HTTPPARSE_BODY: {
                this->ParseBody(curData);
                break;
            }
            default:
                break;
        }
        m_rawMsgbuf.erase(m_rawMsgbuf.begin(), it + (CRLF.end() - CRLF.begin()));
    }
    if (m_parseState == HTTPPARSE_FINISH) {
        std::string headerStr = "";
        for (auto &pair : m_headers) {
            headerStr += pair.first + ":" + pair.second + "\n";
        }
        LOGGER.Log(DEBUG, "[HttpRequest]parse finish. path=" + m_path + " method=" + m_method
            + " version=" + m_version + "\nheaders=" + headerStr + "\nbody=" + m_body);
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

bool HttpRequest::IsFinshed() {
    return m_parseState == HTTPPARSE_FINISH;
}

size_t HttpRequest::GetContentLength() {
    auto it = m_headers.find("Content-Length");
    if (it == m_headers.end()) {
        return 0;
    }
    return std::stoi(it->second);
}