#include "App.h"
#include "Channel.h"
#include <vector>
#include <string>
#include "Msg.h"

#ifndef FILECHANNEL_H
#define FILECHANNEL_H
typedef struct st_ReqFileInfo {
    uint32_t fileNameLen;
    char fileName[256];
} ReqFileInfo;

typedef struct st_sendFileHeader {
    uint64_t fileSize;
} SendFileHeader;

typedef struct st_SendFileInfo {
    int fileFd;
    uint64_t fileSize;
    uint64_t sendSize;
} SendFileInfo;


class FileChannel : public Channel
{
private:
    std::vector<char> m_rawBuf;
    std::string m_sendBuf;
    App *m_app;
    ReqFileInfo m_reqFileInfos;
    SendFileHeader m_sendFileHeader;
    SendFileInfo m_sendFileInfos; // 发送完需要重置
    bool IsSending();
    void PushRawMsg(Msg &msg);
    void ParseReq();
    void HandleReq();
    void SetSendFileHeaser();
public:
    FileChannel(int fd, App *app=nullptr);
    ~FileChannel();
    ChannelHandleResult HandleSocketRead() override;
    ChannelHandleResult HandleSocketWrite() override;
    ChannelHandleResult HandleSocketError() override;
    bool IsNeedSendData() override;

    ReqFileInfo &GetReqFileInfos();
    bool SetSendFileInfos(std::string filePath);
};

#endif