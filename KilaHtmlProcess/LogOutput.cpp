#include "LogOutput.h"
#include <time.h>

//! 静态变量初始化
LMutex*   LogOutput::m_Lock                = NULL;
ELogLevel LogOutput::m_CurrentLogLevel     = ELL_Unknown;
string    LogOutput::m_LogfilePath         = "";
bool      LogOutput::m_OutPutCodeFileName  = false;
bool      LogOutput::m_OutPutFunction      = false;
bool      LogOutput::m_OutputFileLineNo    = false;
bool      LogOutput::m_OutputCurrentTime   = false;
bool      LogOutput::m_OutputToConsole     = false;
bool      LogOutput::m_OutputToDebugBuffer = false;
char*     LogOutput::m_Comment             = NULL;
char*     LogOutput::m_TotalLogComment     = NULL;
fstream   LogOutput::m_LogFileStream;
int       LogOutput::m_BackupFileID = 1;
bool      LogOutput::m_bInited      = false;

//! 默认5M文件大小
size_t LogOutput::m_LimitByteSize = 1024 * 1024 * 10;

//! 初始化日志类
bool LogOutput::initialize(const char* logfilename, ELogLevel currentLogLevel, bool bOutputCodeFileName, bool bOutputFunction,
                           bool bOutputFileLineNo, bool bOutputTime, bool bOutputToConsole, bool bOutputToDebugBuffer)
{
    //! 创建互斥锁
    m_Lock = new LMutex();
    if (m_Lock == NULL)
    {
        return false;
    }

    m_Lock->initilize();

    //! 创建日志输出内容空间
    m_Comment = new char[LOG_MAX_SIZE];
    if (m_Comment == NULL)
    {
        return false;
    }

    m_TotalLogComment = new char[TOTAL_LOG_MAX_SIZE];
    if (m_TotalLogComment == NULL)
    {
        return false;
    }

    m_CurrentLogLevel     = currentLogLevel;
    m_OutPutCodeFileName  = bOutputCodeFileName;
    m_OutPutFunction      = bOutputFunction;
    m_OutputFileLineNo    = bOutputFileLineNo;
    m_OutputCurrentTime   = bOutputTime;
    m_OutputToConsole     = bOutputToConsole;
    m_OutputToDebugBuffer = bOutputToDebugBuffer;

    //! 如果输入文件名称为空，则用当前时间生成日志文件
    if (!logfilename)
    {
        char nowTime[LOG_MAX_TIME_SIZE + 15] = {0};

        //! 获取当前时间
        time_t    tt = time(NULL);
        struct tm tmTime;
        localtime_s(&tmTime, &tt);

        sprintf_s(nowTime, LOG_MAX_TIME_SIZE + 15, "Log_%d_%02d_%02d_%02d_%02d_%02d.txt", 1900 + tmTime.tm_year,
                  1 + tmTime.tm_mon, tmTime.tm_mday, tmTime.tm_hour, tmTime.tm_min, tmTime.tm_sec);

        m_LogfilePath = nowTime;
    }
    else
    {
        m_LogfilePath = logfilename;
    }

    //! 打开日志文件
    m_LogFileStream.open(m_LogfilePath, ios_base::in | ios_base::out | ios_base::trunc);
    if (!(m_LogFileStream.is_open()))
    {
        return false;
    }
    m_bInited = true;
    return true;
}

//! 设置日志输出级别
void LogOutput::setLogLevel(ELogLevel level)
{
    m_CurrentLogLevel = level;
}

//! 设置参数
void LogOutput::setParameter(EParameterType type, bool bOutput)
{
    switch (type)
    {
    case EPT_OutputCodeFile:
        m_OutPutCodeFileName = bOutput;
        break;
    case EPT_OutputTime:
        m_OutputCurrentTime = bOutput;
        break;
    case EPT_OutputFunction:
        m_OutPutFunction = bOutput;
        break;
    case EPT_OutputLineNo:
        m_OutputFileLineNo = bOutput;
        break;
    case EPT_Output2Console:
        m_OutputToConsole = bOutput;
        break;
    case EPT_Output2DebugBuffer:
        m_OutputToDebugBuffer = bOutput;
        break;
    default:
        break;
    }
}

//! 设置日志文件分配大小
void LogOutput::setLogFileSliceBytes(unsigned long long bytes)
{
    m_LimitByteSize = bytes;
}

//! 获取格式化时间
void LogOutput::getFormatTimeString(char* timeString)
{
    SYSTEMTIME st;
    GetLocalTime(&st);

    sprintf_s(timeString, LOG_MAX_TIME_SIZE, "[%d-%02d-%02d %02d:%02d:%02d.%03d]", st.wYear, st.wMonth, st.wDay, st.wHour,
              st.wMinute, st.wSecond, st.wMilliseconds);
}

//! 日志级别文本信息
const char* g_LogLevel[] = {"Unknown", "Info", "Warn", "Error", "Fatal"};

//! 立即写入日志到文件中
void LogOutput::writeInfoLog(const char* srcFile, const char* function, int lineNo, ELogLevel level)
{
    if (level < m_CurrentLogLevel)
    {
        return;
    }

    if (!(m_LogFileStream.is_open()))
    {
        return;
    }

    //! 初始化
    memset(m_TotalLogComment, 0, TOTAL_LOG_MAX_SIZE);

    //! 如果要写入时间
    if (m_OutputCurrentTime)
    {
        char nowTime[LOG_MAX_TIME_SIZE] = {0};
        getFormatTimeString(nowTime);

        char str[1024];
        sprintf_s(str, "%s-", nowTime);
        strcat_s(m_TotalLogComment, TOTAL_LOG_MAX_SIZE, str);
    }

    char str[1024];
    sprintf_s(str, "%s-", g_LogLevel[level]);
    strcat_s(m_TotalLogComment, TOTAL_LOG_MAX_SIZE, str);

    //! 如果要写入代码名称
    if (m_OutPutCodeFileName)
    {
        //! 获取代码名称
        const char* pSrcName = NULL;
        pSrcName             = strrchr(srcFile, '\\');

        if (pSrcName == NULL)
        {
            pSrcName = __FILE__;
        }
        else
        {
            pSrcName++;
        }

        char str[1024];
        sprintf_s(str, "%s-", pSrcName);
        strcat_s(m_TotalLogComment, TOTAL_LOG_MAX_SIZE, str);
    }

    //! 输出函数名称
    if (m_OutPutFunction)
    {
        char str[1024];
        sprintf_s(str, "%s-", function);
        strcat_s(m_TotalLogComment, TOTAL_LOG_MAX_SIZE, str);
    }

    //! 输出行号
    if (m_OutputFileLineNo)
    {
        char str[1024];
        sprintf_s(str, "(%05d)-", lineNo);
        strcat_s(m_TotalLogComment, TOTAL_LOG_MAX_SIZE, str);
    }

    //! 连接用户自定义格式日志
    strcat_s(m_TotalLogComment, TOTAL_LOG_MAX_SIZE, m_Comment);

    //! 输出到文件中
    m_LogFileStream << m_TotalLogComment << endl;

    //! 超过最大限制
    if ((size_t)m_LogFileStream.tellp() > m_LimitByteSize)
    {
        //! 关闭源文件
        m_LogFileStream.close();

        //! 生成备份文件名称
        string backupLogFile = m_LogfilePath;
        size_t position      = backupLogFile.find_last_of('.');
        backupLogFile.erase(position, backupLogFile.length() - position);

        stringstream ss;
        string       numString = "";
        ss << m_BackupFileID;
        ss >> numString;

        backupLogFile += numString + ".txt";

        //! 重新命令当前日志文件
        rename(m_LogfilePath.c_str(), backupLogFile.c_str());

        //! 打开新的日志文件
        m_LogFileStream.open(m_LogfilePath, ios_base::in | ios_base::out | ios_base::trunc);
        if (!(m_LogFileStream.is_open()))
        {
            return;
        }

        ++m_BackupFileID;
    }

    //! 输出到控制台
    if (m_OutputToConsole)
    {
        printf("%s\n", m_TotalLogComment);
    }

    //! 输出到调试缓冲区
    if (m_OutputToDebugBuffer)
    {
        ::OutputDebugStringA(m_TotalLogComment);
    }
}

//! 释放资源
void LogOutput::destroy()
{
    m_Lock->lock();

    //! 关闭文件
    if (m_LogFileStream.is_open())
    {
        m_LogFileStream.close();
    }

    //! 解除锁定
    m_Lock->unlock();

    //! 释放锁
    if (m_Lock != NULL)
    {
        m_Lock->destroy();
        delete m_Lock;
        m_Lock = NULL;
    }

    //! 释放空间
    if (m_Comment != NULL)
    {
        delete[] m_Comment;
        m_Comment = NULL;
    }

    if (m_TotalLogComment != NULL)
    {
        delete[] m_TotalLogComment;
        m_TotalLogComment = NULL;
    }
    m_bInited = false;
}

//! 返回日志级别
ELogLevel LogOutput::getCurrentLogLevel()
{
    return m_CurrentLogLevel;
}

//! 获取格式化文本指针
char* LogOutput::getFormatContent()
{
    return m_Comment;
}

//! 进入锁定区域
void LogOutput::enterLock()
{
    m_Lock->lock();
}

//! 退出锁定区域
void LogOutput::leaveLock()
{
    m_Lock->unlock();
}
