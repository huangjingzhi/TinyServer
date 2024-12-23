/*
    用来接收io的内容
*/
#include <iostream>
#ifndef MSG_H
#define MSG_H

#define MSG_MAX_LEN (16 * 1024 * 1024)
#define MSG_MIN_LEN (sizeof(uint32_t)) // 必须包含头部

typedef struct Msg
{
    size_t len;
    size_t maxLen;
    char *buf; // TODO: 这里是否可以使用智能指针进行优化
    /* data */
    Msg(): len(0), maxLen(0), buf(nullptr) {};

    // TODO: 是否可以增加移动构造
    Msg(size_t mLen):len(0), maxLen(0), buf(nullptr) {
        if (mLen > MSG_MAX_LEN) {
            return;
        }
        buf = new char[mLen]; // TODO: 异常优化
        maxLen = mLen;
    }
    
} Msg;

void FreeMsg(Msg &msg);

#endif