

#include <iostream>
#include "./net/Communicator.h"
#include <arpa/inet.h>
using namespace std;

int main()
{

    for (int i = 0; i < 1; ++i) {
        Communicator communicator("127.0.0.1", 9000);
        std::vector<char> msg {
            '1','2','3','4','a','b'
        };
        for (int j = 0; j < 10; j++) {
            msg.back() = '1' + j;
            communicator.SendMsg(msg);
        }
    }
    return 0;
}