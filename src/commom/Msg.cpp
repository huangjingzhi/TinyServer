#include "Msg.h"
void FreeMsg(Msg &msg)
{
    free(msg.buf);
    msg.buf = nullptr;
    msg.len = 0;
    msg.maxLen = 0;
}