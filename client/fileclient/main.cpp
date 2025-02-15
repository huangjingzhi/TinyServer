#include <iostream>
#include <fcntl.h> 
#include <sys/types.h> 
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <fstream>


typedef struct st_ReqFileInfo {
    uint32_t fileNameLen;
    char fileName[256];
} ReqFileInfo;

typedef struct st_sendFileHeader {
    uint64_t fileSize;
} SendFileHeader;


int main()
{
    std::string filePath = "d";
    
    int sockeFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockeFd == -1) {
        std::cout << "socket error" << std::endl;
        return -1;
    }
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(7000);

    if (connect(sockeFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        close(sockeFd);
        std::cout << "connect error" << std::endl;
        return -1;
    }

    ReqFileInfo reqFileInfo;
    reqFileInfo.fileNameLen = filePath.length();
    strcpy(reqFileInfo.fileName, filePath.c_str());
    reqFileInfo.fileNameLen =  htonl(reqFileInfo.fileNameLen);
    int ret = send(sockeFd, &reqFileInfo, sizeof(reqFileInfo), 0);
    if (ret == -1) {
        close(sockeFd);
        std::cout << "send error" << std::endl;
        return -1;
    }

    std::cout << "send len " << ret << std::endl;


    // 接收文件头信息
    SendFileHeader sendFileHeader;
    ret = recv(sockeFd, &sendFileHeader, sizeof(sendFileHeader), 0);
    if (ret != sizeof(sendFileHeader)) {
        close(sockeFd);
        std::cout << ret <<  std::endl;
        std::cout << "recv error" << std::endl;
        return -1;
    }
    std::cout << "recv len " << ret << std::endl;
    std::cout << sendFileHeader.fileSize << std::endl;

    std::cout << "recv file header" << std::endl;
    sendFileHeader.fileSize = be64toh (sendFileHeader.fileSize);
    std::cout << "file size: " << sendFileHeader.fileSize << std::endl;

    
    // 接收文件
    int fileFd = open(filePath.c_str(), O_CREAT | O_WRONLY, 0644);
    if (fileFd == -1) {
        close(sockeFd);
        std::cout << "open file error" << std::endl;
        return -1;
    }

    std::ofstream outputFile(filePath, std::ios::binary);
    if (!outputFile) {
        close(sockeFd);
        std::cerr << "Failed to open output file" << std::endl;
        close(fileFd);
        return -1;
    }

    const size_t bufferSize = 1024 * 1024;  // 缓冲区大小为1MB
    char buffer[bufferSize];
    size_t totalReceived = 0;
    while (totalReceived < sendFileHeader.fileSize) {
        size_t bytesToReceive = std::min(bufferSize, sendFileHeader.fileSize - totalReceived);
        ssize_t bytesReceived = recv(sockeFd, buffer, bytesToReceive, 0);
        if (bytesReceived == -1) {
            close(sockeFd);
            std::cerr << "Failed to receive data" << std::endl;
            close(fileFd);
            return -1;
        }
        outputFile.write(buffer, bytesReceived);
        totalReceived += bytesReceived;
    }
    outputFile.close();
    close(fileFd);
    close(sockeFd);
    std::cout << "file received" << std::endl;

}