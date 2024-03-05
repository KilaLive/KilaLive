#pragma once

#include "log4cpp/Category.hh"
#include "log4cpp/OstreamAppender.hh"
#include "log4cpp/BasicLayout.hh"
#include "log4cpp/Priority.hh"
#include "log4cpp/PatternLayout.hh"
#include "log4cpp/RollingFileAppender.hh"
#include "UxLogConfig.h"
#include <unordered_map>
#include <mutex>

#if UXLOG_DLL_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

class DLL_API KilaLog
{
public:
    static KilaLog& Instance();
    // 增加一个日志输出
    void InitNewLogger(const std::string& tag);
    // 设置cout日志优先级：DEBUG < INFO < WARN < ERROR < FATAL
    void SetCoutLogPriority(log4cpp::Priority::PriorityLevel loglevel);
    // 设置文件日志优先级：DEBUG < INFO < WARN < ERROR < FATAL
    void        SetFileLogPriority(const std::string& tag, log4cpp::Priority::PriorityLevel loglevel);
    inline void EnableLogToCout(bool isenable)
    {
        islogtocout_ = isenable;
    }
    // 销毁日志类
    void DestoryLog();
    void LogWarn(const std::string& tag, const char* filename, int line, const char* function, const char* format, ...);
    void LogError(const std::string& tag, const char* filename, int line, const char* function, const char* format, ...);
    void LogDebug(const std::string& tag, const char* filename, int line, const char* function, const char* format, ...);
    void LogInfo(const std::string& tag, const char* filename, int line, const char* function, const char* format, ...);
    void LogInfoFixedLength(const std::string& tag, const char* filename, int line, const char* function, const char* service,
                            const char* content);

private:
    KilaLog();
    virtual ~KilaLog();
    // 从文件路径中截取文件名
    const char* GetFileName(const char* fullfilepath);
    // 当前进程exe路径
    std::string GetCurrentExeDirectory();

private:
    std::string               coutlogname_;  //屏幕日志信息
    log4cpp::Category&        coutcategory_;
    log4cpp::OstreamAppender* coutappender_;  //屏幕日志输入
    log4cpp::PatternLayout*   coutlayout_;    //屏幕布局

    bool                                                islogtocout_;
    char*                                               formatbuf_;
    char*                                               logbuf_;
    std::unordered_map<std::string, log4cpp::Category&> logcategorys_;
    std::mutex                                          warn_mutex_;
    std::mutex                                          info_mutex_;
    std::mutex                                          error_mutex_;
    std::mutex                                          debug_mutex_;
};

// 底层逻辑日志
#define UXLOG_WARN(...) KilaLog::Instance().LogWarn(LOG_UXSDK, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);
#define UXLOG_INFO(...) KilaLog::Instance().LogInfo(LOG_UXSDK, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);
#define UXLOG_ERR(...) KilaLog::Instance().LogError(LOG_UXSDK, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);
#define UXLOG_DEBUG(...) KilaLog::Instance().LogDebug(LOG_UXSDK, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define UXLOG_TIM(...) KilaLog::Instance().LogInfo(LOG_UXSDK, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);
#define UXLOG_INFOFIX(service, content)                                                                                      \
    KilaLog::Instance().LogInfoFixedLength(LOG_UXSDK, __FILE__, __LINE__, __FUNCTION__, service, content);

#define LOGFUN UXLOG_DEBUG("%s", __FUNCTION__);

// 守护进程日志
#define DaeLog_Warn(...) KilaLog::Instance().LogWarn(LOG_DAE, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);
#define DaeLog_Info(...) KilaLog::Instance().LogInfo(LOG_DAE, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);
#define DaeLog_Error(...) KilaLog::Instance().LogError(LOG_DAE, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);
#define DaeLog_Fatal(...) KilaLog::Instance().LogDebug(LOG_DAE, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);

// 主进程守护线程日志
#define DaeSrvLog_Warn(...) KilaLog::Instance().LogWarn(LOG_DAESRV, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);
#define DaeSrvLog_Info(...) KilaLog::Instance().LogInfo(LOG_DAESRV, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);
#define DaeSrvLog_Error(...) KilaLog::Instance().LogError(LOG_DAESRV, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);
#define DaeSrvLog_Fatal(...) KilaLog::Instance().LogDebug(LOG_DAESRV, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);

// 开播、连线、连麦日志
#define LivecastLog_Warn(...) KilaLog::Instance().LogWarn(LOG_LIVECAST, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);
#define LivecastLog_Info(...) KilaLog::Instance().LogInfo(LOG_LIVECAST, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);
#define LivecastLog_Error(...) KilaLog::Instance().LogError(LOG_LIVECAST, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);
#define LivecastLog_Fatal(...) KilaLog::Instance().LogDebug(LOG_LIVECAST, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);
#define LivecastLog_Debug(...) KilaLog::Instance().LogDebug(LOG_LIVECAST, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

// 视频采集和渲染日志
#define CapRenderLog_Warn(...) KilaLog::Instance().LogWarn(LOG_CAPRENDER, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);
#define CapRenderLog_Info(...) KilaLog::Instance().LogInfo(LOG_CAPRENDER, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);
#define CapRenderLog_Error(...) KilaLog::Instance().LogError(LOG_CAPRENDER, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);
#define CapRenderLog_Fatal(...) KilaLog::Instance().LogDebug(LOG_CAPRENDER, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);