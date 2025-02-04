#include "TlvServer.h"
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
int main()
{

    signal(SIGFPE, SIG_IGN);
    
    TlvServer tlvServer(9000, 1, 1024);
    tlvServer.Start();
    tlvServer.JoinThreads();
}