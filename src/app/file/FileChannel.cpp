#include "FileChannel.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include "Msg.h"
#include "Logger.h"
#include <cstring>
#include <fcntl.h> 
#include <sys/types.h> 
#include <sys/stat.h>

FileChannel::FileChannel(int fd, App *app) : Channel(fd), m_app(app),
    m_rawBuf(), m_sendBuf(), m_reqFileInfos{0}, m_sendFileInfos{0}
{
    m_rawBuf.reserve(sizeof(ReqFileInfo));
    m_rawBuf.reserve(sizeof(uint32_t));

    m_sendFileInfos.fileFd = -1;
}

FileChannel::~FileChannel()
{

}

bool FileChannel::IsSending()
{
    return m_sendFileInfos.sendSize < m_sendFileInfos.fileSize;
}
void FileChannel::PushRawMsg(Msg &msg)
{
    m_rawBuf.insert(m_rawBuf.end(), msg.buf, msg.buf + msg.len);
}

void FileChannel::ParseReq()
{
    if (m_rawBuf.size() < sizeof(ReqFileInfo)) {
        return;
    }
    (void)memcpy(&m_reqFileInfos, m_rawBuf.data(), sizeof(ReqFileInfo));
    m_rawBuf.erase(m_rawBuf.begin(), m_rawBuf.begin() + sizeof(ReqFileInfo));

    m_reqFileInfos.fileNameLen = ntohl(m_reqFileInfos.fileNameLen);
}

void FileChannel::HandleReq()
{
    if (m_app != nullptr) {
        m_app->Update(this);
    }
}
ChannelHandleResult FileChannel::HandleSocketRead()
{
    Msg msg(4 * 1024);
    int ret = read(this->m_fd, msg.buf, msg.maxLen);
    if (ret > 0) {
        msg.len = ret;
        this->PushRawMsg(msg);
        this->ParseReq();
        this->HandleReq();
    } else {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            FreeMsg(msg);
            return ChannelHandleOK;
        }
        if (ret == 0) {
            LOGGER.Log(INFO, "[FileChannel]client close. fd=" + std::to_string(this->m_fd));
        } else {
            LOGGER.Log(ERROR, "[FileChannel]read error. fd=" + std::to_string(this->m_fd) + " errno=" + std::to_string(errno) 
                + " ret=" + std::to_string(ret));
        }
        FreeMsg(msg);
        return ChannelHandleDelete;
    }

    FreeMsg(msg);
    return ChannelHandleOK;
}
ChannelHandleResult FileChannel::HandleSocketWrite()
{
    if (!IsSending()) {
        return ChannelHandleOK;
    }

    if (!m_sendBuf.empty()) {
        int ret = write(this->m_fd, m_sendBuf.c_str(), m_sendBuf.size());
        if (ret < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return ChannelHandleOK;
            }
            LOGGER.Log(ERROR, "[FileChannel]write error. fd=" + std::to_string(this->m_fd) + " errno=" + std::to_string(errno));
            return ChannelHandleDelete;
        }
        m_sendBuf = m_sendBuf.substr(ret);
    }
    if (!m_sendBuf.empty()) {
        // 需要先发送文件信息，再发送文件
        return ChannelHandleOK;
    }

    off_t offset = static_cast<off_t>(m_sendFileInfos.sendSize);
    ssize_t retSize = sendfile(this->m_fd, m_sendFileInfos.fileFd, &offset, m_sendFileInfos.fileSize - m_sendFileInfos.sendSize);
    if (retSize < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return ChannelHandleOK;
        }
        LOGGER.Log(ERROR, "[FileChannel]sendfile error. fd=" + std::to_string(this->m_fd) + " errno=" + std::to_string(errno));
        return ChannelHandleDelete;
    }

    m_sendFileInfos.sendSize = static_cast<uint64_t>(offset);
    if (m_sendFileInfos.sendSize == m_sendFileInfos.fileSize) {
        close(m_sendFileInfos.fileFd);
        m_sendFileInfos = {0};
        m_reqFileInfos = {0};
    }

    return ChannelHandleOK;
}

ChannelHandleResult FileChannel::HandleSocketError()
{
    LOGGER.Log(ERROR, "[FileChannel]socket error. fd=" + std::to_string(this->m_fd) + " errno=" + std::to_string(errno));
    return ChannelHandleDelete;
}
bool FileChannel::IsNeedSendData()
{
    return IsSending();
}

ReqFileInfo &FileChannel::GetReqFileInfos()
{
    return m_reqFileInfos;
}

bool FileChannel::SetSendFileInfos(std::string filePath)
{
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) < 0) {
        LOGGER.Log(ERROR, "[FileChannel]stat error. filePath=" + filePath + " errno=" + std::to_string(errno));
        return false;
    }
    m_sendFileInfos.fileFd = open(filePath.c_str(), O_RDONLY);
    if (m_sendFileInfos.fileFd < 0) {
        LOGGER.Log(ERROR, "[FileChannel]open error. filePath=" + filePath + " errno=" + std::to_string(errno));
        return false;
    }
    m_sendFileInfos.fileSize = fileStat.st_size;
    m_sendFileInfos.sendSize = 0;

    this->SetSendFileHeaser();
    return true;
}

void FileChannel::SetSendFileHeaser()
{
    m_sendFileHeader.fileSize = m_sendFileInfos.fileSize;


    SendFileHeader tmpSendFileHeader = m_sendFileHeader;
    tmpSendFileHeader.fileSize = htobe64(tmpSendFileHeader.fileSize);
    m_sendBuf = std::string(reinterpret_cast<char *>(&tmpSendFileHeader), sizeof(tmpSendFileHeader));
}