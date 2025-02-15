#include "FileServer.h"
#include <signal.h>


int main()
{
    signal(SIGFPE, SIG_IGN);

    FileServer fileServer(7000, 1, 1024);
    fileServer.AddFile("a", "/home/hjz/netserver/src/resources/video/480_1.mp4");
    fileServer.AddFile("b", "/home/hjz/netserver/src/resources/video/2.mp4");
    fileServer.AddFile("c", "/home/hjz/netserver/src/resources/video/3.mp4");
    fileServer.AddFile("d", "/home/hjz/netserver/src/resources/video/base2.mp4");


    fileServer.Start();
    fileServer.JoinThreads();
    return 0;
}