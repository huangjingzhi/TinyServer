#include "Channel.h"


#ifndef APP_H
#define APP_H


/*
    应用层Channel 产生的网络数据
*/
class App
{
private:
public:
    App();
    virtual ~App();
    virtual void Update(Channel *channel);
};
#endif