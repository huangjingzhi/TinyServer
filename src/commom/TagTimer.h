#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iostream>
#include <functional>
#include "Logger.h"
using namespace std;


#ifndef TAGTIMER_H
#define TAGTIMER_H

int64_t GetCurEpochTimeWithSecond();

using TimeOutAction = std::function<void()>;

template <typename T>
class TagTimer
{
    vector< pair<int64_t, unordered_set<T> > > m_slots; // 时间槽， 槽中使用 set 保存数据
    unordered_map<T, size_t> m_latestPos;
    uint32_t m_slotTime;
    uint32_t m_slotSize;
    size_t m_curPos;
    bool m_isAutomaticUpdate;
    unordered_map<T, TimeOutAction> m_timeoutActions;
    void MoveTime(int64_t t);
public:
    TagTimer();
    TagTimer(uint32_t slotSize, uint32_t slotTime, bool automatic=false);
    ~TagTimer();
    TagTimer(const TagTimer &timer);
    void Add(const T &member, const TimeOutAction &action);
    void Update(const T &member);
    void Delete(const T &member);
    uint32_t GetTimeoutPeriod();
};

template <typename T>
TagTimer<T>::TagTimer(uint32_t slotSize, uint32_t slotTime, bool automatic)
    : m_slotSize(slotSize), m_slotTime(slotTime), m_isAutomaticUpdate(automatic)
{
    m_slots.resize(m_slotSize);
    m_curPos = 0;
    m_slots[m_curPos].first = GetCurEpochTimeWithSecond();
}

template <typename T>
TagTimer<T>::TagTimer(): TagTimer(60, 1)
{
}

template <typename T>
TagTimer<T>::~TagTimer()
{

}

template <typename T>
TagTimer<T>::TagTimer(const TagTimer &timer)
{
    this->m_slots = timer.m_slots;
    this->m_latestPos = timer.m_latestPos;
    this->m_slotTime = timer.m_slotTime;
    this->m_slotSize = timer.m_slotSize;
    this->m_isAutomaticUpdate = timer.m_isAutomaticUpdate;
    this->m_timeoutActions = timer.m_timeoutActions;
}

template <typename T>
void TagTimer<T>::MoveTime(int64_t t)
{
    int64_t slotTime = m_slots[m_curPos].first;
    if (t < slotTime) {
        return;
    }

    size_t steps = ((t - slotTime)/ m_slotTime);
    for (size_t i = 0; i < steps; ++i)
    {
        slotTime += m_slotTime;
    
        m_curPos = (m_curPos + 1) % m_slotSize;
        for (auto &it : m_slots[m_curPos].second) {
            if (m_curPos == m_latestPos[it]) {
                if (m_latestPos.find(it) == m_latestPos.end()) {
                    continue;
                }
                // do something for time out
                auto itAction = m_timeoutActions.find(it);
                if (itAction != m_timeoutActions.end())
                {
                    itAction->second();
                }
                this->Delete(it);
            }
        }
        m_slots[m_curPos].second.clear();
        m_slots[m_curPos].first = slotTime;    
    }
}

template <typename T>
void TagTimer<T>::Add(const T &member, const TimeOutAction &action)
{
    int64_t t = GetCurEpochTimeWithSecond();
    this->MoveTime(t);

    m_slots[m_curPos].second.insert(member);
    m_latestPos[member] = m_curPos;
    m_timeoutActions[member] = action;
    
}

template <typename T>
void TagTimer<T>::Update(const T &member)
{
    int64_t t = GetCurEpochTimeWithSecond();
    this->MoveTime(t);

    m_slots[m_curPos].second.insert(member);
    m_latestPos[member] = m_curPos;
}

template <typename T>
void TagTimer<T>::Delete(const T &member)
{
    auto it = m_latestPos.find(member);
    if (it != m_latestPos.end())
    {
        m_latestPos.erase(it);
    }

    auto itAction = m_timeoutActions.find(member);
    if (itAction != m_timeoutActions.end())
    {
        m_timeoutActions.erase(itAction);
    }
}

template <typename T>
uint32_t TagTimer<T>::GetTimeoutPeriod()
{
    return m_slotTime;
}
#endif