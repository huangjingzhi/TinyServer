#include <vector>
#include <map>
#include <string>
#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H


enum HttpResType {
    HTTPRES_TYPE_DEFAULT = 0,
    HTTPRES_TYPE_HTML,
    HTTPRES_TYPE_XML,
    HTTPRES_TYPE_PNG
};

enum HttpResCode {
    HTTPRES_CODE_DEFAULT = 0,
    HTTPRES_CODE_OK = 200,
    HTTPRES_CODE_BADREQUEST = 400,
    HTTPRES_CODE_FORBIDDEN = 403,
    HTTPRES_CODE_NOTFOUND = 404,
    HTTPRES_CODE_SERVER_ERROR = 500,
};



class HttpResponse
{
    HttpResType m_resType;
    HttpResCode m_resCode;
    bool m_ready;
    bool m_sending;
    std::string m_status;
    std::map<std::string, std::string> m_headers;
    std::string m_body;

    static std::string CodeToStr(HttpResCode code);
    static std::string TypeToStr(HttpResType type);

public:
    HttpResponse();

    bool SetStatus(HttpResCode code);
    bool AddStatus(const std::string &status);
    bool AddHeader(const std::string &key, const std::string &value);
    void AddBody(const std::string &data);
    void SetBody(const std::string &body);
    bool LoadFileToBody(const std::string &filePath);
    std::string MakeResponse();
    bool IsReady();
    void SetReady(bool ready);
    bool IsSending();
    void SetSending(bool sending);
    void SetResType(const HttpResType type);
};

#endif