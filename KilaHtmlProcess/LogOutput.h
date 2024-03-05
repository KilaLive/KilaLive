#ifndef __Logger_H__
#define __Logger_H__

#include <Windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "BaseLog.h"

using namespace std;

//! 最大日志长度
#define LOG_MAX_SIZE 10240

#define TOTAL_LOG_MAX_SIZE (LOG_MAX_SIZE + 200)

//! 最大时间长度
#define LOG_MAX_TIME_SIZE 32

//! 互斥锁结构体
struct LMutex
{
    //! 互斥锁
    CRITICAL_SECTION section;

    //! 初始化
    void initilize()
    {
        ::InitializeCriticalSection(&section);
    }

    //! 锁住
    void lock()
    {
        ::EnterCriticalSection(&section);
    }

    //! 解锁
    void unlock()
    {
        ::LeaveCriticalSection(&section);
    }

    //! 销毁
    void destroy()
    {
        ::DeleteCriticalSection(&section);
    }
};

//! 日志输出类
class LogOutput
{
public:
    //! 初始化日志类
    static bool initialize(const char* logfilename = NULL, ELogLevel currentLogLevel = ELL_Unknown,
                           bool bOutputCodeFileName = true, bool bOutputFunction = true, bool bOutputFileLineNo = true,
                           bool bOutputTime = true, bool bOutputToConsole = true, bool bOutputToDebugBuffer = true);

    //! 设置日志输出级别
    static void setLogLevel(ELogLevel level);

    //! 设置参数
    static void setParameter(EParameterType type, bool bOutput);

    //! 设置日志文件分配大小
    static void setLogFileSliceBytes(unsigned long long bytes);

    //! 立即写入日志到文件中
    static void writeInfoLog(const char* srcFile, const char* function, int lineNo, ELogLevel level);

    //! 释放资源
    static void destroy();

    //! 返回日志级别
    static ELogLevel getCurrentLogLevel();

    //! 获取格式化文本指针
    static char* getFormatContent();

    //! 进入锁定区域
    static void enterLock();

    //! 退出锁定区域
    static void leaveLock();

    static bool isInited()
    {
        return m_bInited;
    }

private:
    //! 获取格式化时间
    static void getFormatTimeString(char* timeString);

    //! 线程锁
    static LMutex* m_Lock;

    //! 当前日志级别
    static ELogLevel m_CurrentLogLevel;

    //! 日志文件路径
    static string m_LogfilePath;

    //! 是否输出代码文件名称
    static bool m_OutPutCodeFileName;

    //! 是否输出函数
    static bool m_OutPutFunction;

    //! 是否输出行号
    static bool m_OutputFileLineNo;

    //! 是否输出日志输出时间
    static bool m_OutputCurrentTime;

    //! 是否输出日志到控制台
    static bool m_OutputToConsole;

    //! 是否输出日志到调试缓冲区
    static bool m_OutputToDebugBuffer;

    //! 日志格式化空间
    static char* m_Comment;

    //! 总的日志信息
    static char* m_TotalLogComment;

    //! 文件指针
    static fstream m_LogFileStream;

    //! 统计日志写入的字节数
    static size_t m_LimitByteSize;

    //! 当前备份文件索引
    static int m_BackupFileID;

    static bool m_bInited;
};

#endif
