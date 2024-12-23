

#include <iostream>
#include "./net/Communicator.h"
#include <arpa/inet.h>
using namespace std;

int main()
{

    for (int i = 0; i < 5; ++i) {
        Communicator communicator("127.0.0.1", 20000);
        std::vector<char> msg {
            '1','2','3','4','a','b'
        };
        communicator.SendMsg(msg);
            communicator.SendMsg(msg);


        for (int j = 0; j < 10; j++) {
            communicator.SendMsg(msg);
        }

        Communicator communicator1("127.0.0.1", 20000);

        
        for (int j = 0; j < 10; j++) {
            communicator1.SendMsg(msg);
        }
            Communicator communicator2("127.0.0.1", 20000);
        for (int j = 0; j < 10; j++) {
            communicator2.SendMsg(msg);
        }
            Communicator communicator3("127.0.0.1", 20000);
        for (int j = 0; j < 10; j++) {
            communicator3.SendMsg(msg);
        }
    }

    
    uint32_t len = 4;
    uint32_t toNetLen = htonl(len);
    uint32_t retLen = ntohl(toNetLen);
    std::cout << len << " " << toNetLen << " " << retLen;

    return 0;
}