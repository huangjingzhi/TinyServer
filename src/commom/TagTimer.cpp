#include "TagTimer.h"
#include <chrono>

Logger TIMER_LOG{"Timer.log"};


int64_t GetCurEpochTimeWithSecond()
{
    return chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count();
}