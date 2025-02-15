#include <memory>
#include <unordered_map>
#include "ConnectManage.h"
#include "FileChannel.h"
#include "Channel.h"
#include "App.h"

#ifndef FILESERVER_H
#define FILESERVER_H
class FileServer : public App
{
private:
    int m_port;
    int m_netIoworkerNumber;
    int m_workerMaxFd;
    std::unique_ptr<ConnectManage<FileChannel>> m_fileConnectManager;
    std::unordered_map<std::string, std::string> m_fileMap;
public:
    FileServer(int port, int netIoworkerNumber, int workerMaxFd);
    ~FileServer();
    void Update(Channel *channel) override;
    void JoinThreads();
    void Start();
    void AddFile(std::string file, std::string filePath);
};

#endif