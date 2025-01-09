#include "Msg.h"
void FreeMsg(Msg &msg)
{
    if (msg.buf != nullptr)
        free(msg.buf);
    msg.buf = nullptr;
    msg.len = 0;
    msg.maxLen = 0;
}