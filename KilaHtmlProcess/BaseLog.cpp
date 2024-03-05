// YCatLog.cpp : 定义 DLL 应用程序的导出函数。
//
#include <windows.h>
#include "BaseLog.h"
#include "LogOutput.h"

//! 函数功能:日志初始化
//! 参数:
//!   1、logfilename-日志文件名称
//! 返回值:  无
bool YCatLog_Init(const char* logfilename)
{
    return LogOutput::initialize(logfilename);
}

//! 函数功能:设置日志输出级别，低于该级别的日志将不输出
//! 参数:
//!     1、level-当前日志级别
//! 返回值:  无
void YCatLog_SetLogLevel(ELogLevel level)
{
    if (!LogOutput::isInited())
    {
        return;
    }
    LogOutput::setLogLevel(level);
}

//! 函数功能:立即写入日志到文件中
//! 参数:
//!     1、level-日志级别，低于该级别的日志将不输出
//!     2、information-待写日志信息
//! 返回值:  无
void YCatLog_WriteInfoLog(const char* srcFile, const char* function, int lineNo, ELogLevel level, const char* format, ...)
{
    if (!LogOutput::isInited())
    {
        return;
    }

    //! 判断日志级别
    if (level < LogOutput::getCurrentLogLevel())
    {
        return;
    }

    LogOutput::enterLock();

    //! 格式化内容输出
    va_list vl;
    va_start(vl, format);
    vsprintf_s(LogOutput::getFormatContent(), LOG_MAX_SIZE, format, vl);
    va_end(vl);

    //! 输出日志
    LogOutput::writeInfoLog(srcFile, function, lineNo, level);

    //! 退出锁定区域
    LogOutput::leaveLock();
}

//! 函数功能:配置日志参数,设置日志是否输出时间、代码文件、行号、函数等
//! 参数:
//!     1、type-需要配置的参数类型
//!     2、bOutput-是否在日志文件中输出该类信息(如：时间、代码文件、行号、函数)
//! 返回值:  无
void YCatLog_SetParameter(EParameterType type, bool bOutput)
{
    if (!LogOutput::isInited())
    {
        return;
    }
    LogOutput::setParameter(type, bOutput);
}

//! 函数功能:设置日志文件分配大小
//! 参数:
//!     1、bytes-文件限制字节数
//! 返回值:  无
void YCatLog_SetLogFileSliceSize(size_t millions)
{
    if (!LogOutput::isInited())
    {
        return;
    }
    LogOutput::setLogFileSliceBytes(millions * 1024 * 1024);
}

//! 函数功能:释放资源
//! 参数:    无
//! 返回值:  无
void YCatLog_Release()
{
    if (!LogOutput::isInited())
    {
        return;
    }
    LogOutput::destroy();
}
