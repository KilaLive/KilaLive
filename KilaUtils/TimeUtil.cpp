#include "TimeUtil.h"
#include <Windows.h>
#include <time.h>

std::string CTimeUtil::GetCurrentLiveTime(void)
{
    constexpr int MAX_TIME_SIZE               = 24;
    char          nowTime[MAX_TIME_SIZE + 15] = {0};
    time_t        tt                          = time(NULL);
    struct tm     tmTime;
    localtime_s(&tmTime, &tt);

    std::string format = "%d_%02d_%02d_%02d_%02d_%02d";
    sprintf_s(nowTime, MAX_TIME_SIZE + 15, format.c_str(), 1900 + tmTime.tm_year, 1 + tmTime.tm_mon, tmTime.tm_mday,
              tmTime.tm_hour, tmTime.tm_min, tmTime.tm_sec);

    return nowTime;
}
