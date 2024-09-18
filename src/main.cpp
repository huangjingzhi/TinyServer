

#include <iostream>
using namespace std;

#include "ConnectManage.h"
int main()
{
    ConnectManage conManager(20000, 2, 1);
    conManager.init();
    conManager.JoinThreads();
    return 0;
}
