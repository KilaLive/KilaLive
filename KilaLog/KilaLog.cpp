#include "KilaLog.h"
#include <time.h>

KilaLog& KilaLog::Instance()
{
    static KilaLog instance;
    return instance;
}

KilaLog::KilaLog()
    : coutlogname_("cout")
    , coutcategory_(log4cpp::Category::getRoot().getInstance(coutlogname_))
    , islogtocout_(false)
{
    if (coutappender_ == NULL || coutlayout_ == NULL)
    {
        coutappender_ = new log4cpp::OstreamAppender(coutlogname_, &std::cout);
        coutlayout_   = new log4cpp::PatternLayout();
        coutlayout_->setConversionPattern("%d{%Y-%m-%d %H:%M:%S}-[%p]%c: %m%n");
        coutappender_->setLayout(coutlayout_);
        coutcategory_.addAppender(coutappender_);
        coutcategory_.setPriority(log4cpp::Priority::NOTSET);

        logcategorys_.emplace(coutlogname_, coutcategory_);

        const std::string exepath = GetCurrentExeDirectory();
        if (exepath.empty() || exepath == "")
        {
            return;
        }
        const std::string mainlogdir = exepath + "\\log";
        CreateDirectory(mainlogdir.c_str(), NULL);
    }

    formatbuf_ = new char[MAX_LOGMSG_SIZE];
    logbuf_    = new char[TOTAL_LOGMSG_SIZE];
}

KilaLog::~KilaLog()
{
    DestoryLog();
}

void KilaLog::InitNewLogger(const std::string& tag)
{
    const std::string exepath = GetCurrentExeDirectory();
    if (exepath.empty() || exepath == "")
    {
        return;
    }
    const std::string full_logdirpath = exepath + tag;
    CreateDirectory(full_logdirpath.c_str(), NULL);

    constexpr int LOG_MAX_TIME_SIZE               = 24;
    char          nowTime[LOG_MAX_TIME_SIZE + 15] = {0};
    time_t        tt                              = time(NULL);
    struct tm     tmTime;
    localtime_s(&tmTime, &tt);

    const char* dirname = nullptr;
    dirname             = strrchr(full_logdirpath.c_str(), '\\');
    if (dirname == nullptr)
    {
        dirname = "log";
    }
    else
    {
        dirname++;
    }

    std::string format = std::string(dirname) + "_%d_%02d_%02d_%02d_%02d_%02d." + std::string(CLOGFILE_SUFFIX);
    sprintf_s(nowTime, LOG_MAX_TIME_SIZE + 15, format.c_str(), 1900 + tmTime.tm_year, 1 + tmTime.tm_mon, tmTime.tm_mday,
              tmTime.tm_hour, tmTime.tm_min, tmTime.tm_sec);

    std::string logfilepath = full_logdirpath + "\\" + nowTime;

    log4cpp::Category&            filecate = log4cpp::Category::getRoot().getInstance(tag);
    log4cpp::RollingFileAppender* appender =
        new log4cpp::RollingFileAppender("logFile", logfilepath, MAX_LOGFILE_SIZE, MAX_LOGBACKUP_INDEX);
    log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
    layout->setConversionPattern("%d{%Y-%m-%d,%H:%M:%S}-[%p]%c: %m%n");
    appender->setLayout(layout);
    filecate.addAppender(appender);
    filecate.setPriority(log4cpp::Priority::INFO);

    logcategorys_.emplace(tag, filecate);

    std::cout << "KilaLog init success. tag:  " << tag.c_str() << ", filepath: " << logfilepath.c_str() << std::endl;
}

void KilaLog::DestoryLog()
{
    log4cpp::Category::shutdown();

    if (formatbuf_ != nullptr)
    {
        delete[] formatbuf_;
        formatbuf_ = nullptr;
    }

    if (logbuf_ != nullptr)
    {
        delete[] logbuf_;
        logbuf_ = nullptr;
    }
}

void KilaLog::SetCoutLogPriority(log4cpp::Priority::PriorityLevel loglevel)
{
    coutcategory_.setPriority(loglevel);
}

void KilaLog::SetFileLogPriority(const std::string& tag, log4cpp::Priority::PriorityLevel loglevel)
{
    auto it = logcategorys_.find(tag);
    if (it != logcategorys_.end())
    {
        it->second.setPriority(loglevel);
    }
}

void KilaLog::LogWarn(const std::string& tag, const char* filename, int line, const char* function, const char* format, ...)
{
    std::unique_lock<std::mutex> lock(warn_mutex_);

    if (formatbuf_ == nullptr || logbuf_ == nullptr)
    {
        return;
    }

    try
    {
        memset(formatbuf_, 0, MAX_LOGMSG_SIZE);
        va_list vl;
        va_start(vl, format);
        vsprintf_s(formatbuf_, MAX_LOGMSG_SIZE, format, vl);
        va_end(vl);

        memset(logbuf_, 0, TOTAL_LOGMSG_SIZE);
        sprintf(logbuf_, "%s,line %d,%s: %s", GetFileName(filename), line, function, formatbuf_);

        auto it = logcategorys_.find(tag);
        if (it == logcategorys_.end())
        {
            InitNewLogger(tag);
            it = logcategorys_.find(tag);
        }
        it->second.warn("%s", logbuf_);

        if (islogtocout_)
        {
            if (strlen(logbuf_) < 1024)
            {
                coutcategory_.warn("%s", logbuf_);
            }
            else
            {
                coutcategory_.warn("%s", "Message Too Long. Do not cout to screen.");
            }
        }
    }
    catch (...)
    {
    }
}

void KilaLog::LogError(const std::string& tag, const char* filename, int line, const char* function, const char* format, ...)
{
    std::unique_lock<std::mutex> lock(error_mutex_);

    if (formatbuf_ == nullptr || logbuf_ == nullptr)
    {
        return;
    }

    try
    {
        memset(formatbuf_, 0, MAX_LOGMSG_SIZE);
        va_list vl;
        va_start(vl, format);
        vsprintf_s(formatbuf_, MAX_LOGMSG_SIZE, format, vl);
        va_end(vl);

        memset(logbuf_, 0, TOTAL_LOGMSG_SIZE);
        sprintf(logbuf_, "%s,line %d,%s: %s", GetFileName(filename), line, function, formatbuf_);

        auto it = logcategorys_.find(tag);
        if (it == logcategorys_.end())
        {
            InitNewLogger(tag);
            it = logcategorys_.find(tag);
        }
        it->second.error("%s", logbuf_);

        if (islogtocout_)
        {
            if (strlen(logbuf_) < 1024)
            {
                coutcategory_.error("%s", logbuf_);
            }
            else
            {
                coutcategory_.error("%s", "Message Too Long. Do not cout to screen.");
            }
        }
    }
    catch (...)
    {
    }
}

void KilaLog::LogDebug(const std::string& tag, const char* filename, int line, const char* function, const char* format, ...)
{
    std::unique_lock<std::mutex> lock(debug_mutex_);

    if (formatbuf_ == nullptr || logbuf_ == nullptr)
    {
        return;
    }

    try
    {
        memset(formatbuf_, 0, MAX_LOGMSG_SIZE);
        va_list vl;
        va_start(vl, format);
        vsprintf_s(formatbuf_, MAX_LOGMSG_SIZE, format, vl);
        va_end(vl);

        memset(logbuf_, 0, TOTAL_LOGMSG_SIZE);
        sprintf(logbuf_, "%s,line %d,%s: %s", GetFileName(filename), line, function, formatbuf_);

        auto it = logcategorys_.find(tag);
        if (it == logcategorys_.end())
        {
            InitNewLogger(tag);
            it = logcategorys_.find(tag);
        }
        it->second.debug("%s", logbuf_);

        if (islogtocout_)
        {
            if (strlen(logbuf_) < 1024)
            {
                coutcategory_.debug("%s", logbuf_);
            }
            else
            {
                coutcategory_.debug("%s", "Message Too Long. Do not cout to screen.");
            }
        }
    }
    catch (...)
    {
    }
}

void KilaLog::LogInfo(const std::string& tag, const char* filename, int line, const char* function, const char* format, ...)
{
    std::unique_lock<std::mutex> lock(info_mutex_);

    if (formatbuf_ == nullptr || logbuf_ == nullptr)
    {
        return;
    }

    try
    {
        memset(formatbuf_, 0, MAX_LOGMSG_SIZE);
        va_list vl;
        va_start(vl, format);
        vsprintf_s(formatbuf_, MAX_LOGMSG_SIZE, format, vl);
        va_end(vl);

        memset(logbuf_, 0, TOTAL_LOGMSG_SIZE);
        sprintf(logbuf_, "%s,line %d,%s: %s", GetFileName(filename), line, function, formatbuf_);

        auto it = logcategorys_.find(tag);
        if (it == logcategorys_.end())
        {
            InitNewLogger(tag);
            it = logcategorys_.find(tag);
        }

        it->second.info("%s", logbuf_);

        if (islogtocout_)
        {
            if (strlen(logbuf_) < 1024)
            {
                coutcategory_.info("%s", logbuf_);
            }
            else
            {
                coutcategory_.info("%s", "Message Too Long. Do not cout to screen.");
            }
        }
    }
    catch (...)
    {
    }
}

void KilaLog::LogInfoFixedLength(const std::string& tag, const char* filename, int line, const char* function,
                                 const char* service, const char* content)
{
    std::unique_lock<std::mutex> lock(info_mutex_);

    if (formatbuf_ == nullptr || logbuf_ == nullptr)
    {
        return;
    }

    memset(formatbuf_, 0, MAX_LOGMSG_SIZE);

    //! 不能超过2M
    int srcLength  = strlen(content);
    int destLength = 2 * 1024;

    if (srcLength == 0)
    {
        return;
    }

    if (srcLength > destLength)
    {
        memcpy(formatbuf_, content, destLength);
    }
    else
    {
        memcpy(formatbuf_, content, srcLength);
    }

    memset(logbuf_, 0, TOTAL_LOGMSG_SIZE);
    sprintf(logbuf_, "%s,line %d,%s: %s-%s", GetFileName(filename), line, function, service, formatbuf_);

    auto it = logcategorys_.find(tag);
    if (it == logcategorys_.end())
    {
        InitNewLogger(tag);
        it = logcategorys_.find(tag);
    }

    it->second.info("%s", logbuf_);

    if (islogtocout_)
    {
        if (strlen(logbuf_) < 1024)
        {
            coutcategory_.info("%s", logbuf_);
        }
        else
        {
            coutcategory_.info("%s", "Message Too Long. Do not cout to screen.");
        }
    }
}

const char* KilaLog::GetFileName(const char* fullfilepath)
{
    const char* pSrcName = nullptr;
    pSrcName             = strrchr(fullfilepath, '\\');

    if (pSrcName == nullptr)
    {
        pSrcName = fullfilepath;
    }
    else
    {
        pSrcName++;
    }

    return pSrcName;
}

std::string KilaLog::GetCurrentExeDirectory()
{
    std::string str;
    char        path[MAX_PATH];
    ZeroMemory(path, MAX_PATH);
    if (GetModuleFileName(NULL, path, MAX_PATH))
    {
        std::string            tmp(path);
        std::string::size_type pos = tmp.find_last_of("\\/");
        if (pos != std::string::npos)
        {
            str = tmp.substr(0, pos);
        }
    }
    return str;
}