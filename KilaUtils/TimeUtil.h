#ifndef H_TimeUtil_H
#define H_TimeUtil_H

#include "KilaUtils.h"
#include <string>

class KILAUTILS_API CTimeUtil final
{
public:
    //! 读取注册表
    static std::string GetCurrentLiveTime(void);
};

#endif
