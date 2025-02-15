#include "FileServer.h"
#include "ConnectManage.h"
#include "Logger.h"

Logger FileServerLogger("FileServer.log");

FileServer::FileServer(int port, int netIoworkerNumber, int workerMaxFd)
    : m_port(port), m_netIoworkerNumber(netIoworkerNumber), m_workerMaxFd(workerMaxFd), m_fileConnectManager(nullptr)
{

}

FileServer::~FileServer()
{

}

void FileServer::Start()
{
    m_fileConnectManager.reset(new ConnectManage<FileChannel>(m_port, m_netIoworkerNumber, m_workerMaxFd, this));
    m_fileConnectManager->Init();
}
void FileServer::Update(Channel *channel)
{
    FileChannel *fileChannel = dynamic_cast<FileChannel *>(channel);
    if (fileChannel == nullptr) {
        return;
    }
    ReqFileInfo &reqFileInfos = fileChannel->GetReqFileInfos();
    std::string fileName(reqFileInfos.fileName, reqFileInfos.fileNameLen);
    auto it = m_fileMap.find(fileName);
    if (it == m_fileMap.end()) {
        FileServerLogger.Log(ERROR , "file not found: " + fileName);
        return;
    }
    if (!fileChannel->SetSendFileInfos(it->second)) {
        FileServerLogger.Log(ERROR, "set send file info error: " + fileName);
        return;
    }
}
void FileServer::JoinThreads()
{
    m_fileConnectManager->JoinThreads();
}
void FileServer::AddFile(std::string file, std::string filePath)
{
    m_fileMap[file] = filePath;
}
