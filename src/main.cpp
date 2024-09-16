

#include <iostream>
using namespace std;

#include "ConnectManage.h"
int main()
{
    ConnectManage conManager(20000, 10);
    conManager.init();
    conManager.JoinThreads();
    return 0;

}
