#pragma once

//! 日志级别
enum ELogLevel
{
    ELL_Unknown = 0,  //! 未知
    ELL_Info,         //! 普通信息
    ELL_Warn,         //! 告警信息
    ELL_Error,        //! 错误信息
    ELL_Fatal,        //! 致命信息
};

//! 参数类型
enum EParameterType
{
    EPT_OutputTime,         //! 输出当前时间
    EPT_OutputCodeFile,     //! 输出日志产生的代码文件名称
    EPT_OutputFunction,     //! 输出日志产生的函数名称
    EPT_OutputLineNo,       //! 输出日志产生的代码文件行号
    EPT_Output2Console,     //! 输出日志到控制台
    EPT_Output2DebugBuffer  //! 输出到调试缓冲区
};

//! 函数功能:日志初始化
//! 参数:
//!   1、logfilename-日志文件名称
//! 返回值:  无
bool YCatLog_Init(const char* logfilename = NULL);

//! 函数功能:设置日志输出级别，低于该级别的日志将不输出
//! 参数:
//!     1、level-当前日志级别
//! 返回值:  无
void YCatLog_SetLogLevel(ELogLevel level);

//! 函数功能:立即写入日志到文件中
//! 参数:
//!     1、level-日志级别，低于该级别的日志将不输出
//!     2、format-待写日志格式信息
//! 返回值:  无
void YCatLog_WriteInfoLog(const char* srcFile, const char* function, int lineNo, ELogLevel level, const char* format, ...);

//! 函数功能:配置日志参数,设置日志是否输出时间、代码文件、行号、函数等
//! 参数:
//!     1、type-需要配置的参数类型
//!     2、bOutput-是否在日志文件中输出该类信息(如：时间、代码文件、行号、函数)
//! 返回值:  无
void YCatLog_SetParameter(EParameterType type, bool bOutput);

//! 函数功能:设置日志文件分片大小(单位:M)
//! 参数:
//!     1、bytes-文件限制字节数
//! 返回值:  无
void YCatLog_SetLogFileSliceSize(size_t millions);

//! 函数功能:释放资源
//! 参数:    无
//! 返回值:  无
void YCatLog_Release();

//! 定义宏
#define Log_Init YCatLog_Init
#define Log_SetLogLevel YCatLog_SetLogLevel
#define Log_Info(...) YCatLog_WriteInfoLog(__FILE__, __FUNCTION__, __LINE__, ELL_Info, ##__VA_ARGS__)
#define Log_Warn(...) YCatLog_WriteInfoLog(__FILE__, __FUNCTION__, __LINE__, ELL_Warn, ##__VA_ARGS__)
#define Log_Error(...) YCatLog_WriteInfoLog(__FILE__, __FUNCTION__, __LINE__, ELL_Error, ##__VA_ARGS__)
#define Log_Fatal(...) YCatLog_WriteInfoLog(__FILE__, __FUNCTION__, __LINE__, ELL_Fatal, ##__VA_ARGS__)
#define Log_SetParameter YCatLog_SetParameter
#define Log_SetLogFileSliceSize YCatLog_SetLogFileSliceSize
#define Log_Release YCatLog_Release
