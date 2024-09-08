

#include <iostream>
using namespace std;

#include "ConnectManage.h"
int main()
{
    ConnectManage conManager(8080);
    conManager.init();
    return 0;

}
