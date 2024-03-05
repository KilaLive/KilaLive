#pragma once

#define MAX_LOGFILE_SIZE (5 * 1024 * 1024)  // 最大日志文件大小

#define MAX_LOGBACKUP_INDEX 10  // 最多备份日志文件个数

#define LOG_UPLOAD_RTC 1  // 0：日志不上传RTC文件 1：上传

#define MAX_LOGMSG_SIZE (1024 * 1024)  // 最大日志消息

#define TOTAL_LOGMSG_SIZE (MAX_LOGMSG_SIZE + 256)  // 加上时间、文件名等的最大日志消息

#define LOG_DMPS "\\log\\dmps"            // 奔溃文件目录
#define LOG_IM "\\log\\im"                // im日志目录
#define LOG_PC "\\log\\pc"                // pc业务逻辑日志目录
#define LOG_RTC "\\log\\rtc"              // rtc日志目录
#define LOG_UXSDK "\\log\\uxsdk"          // uxsdk日志目录
#define LOG_DAE "\\log\\zdae"             // 守护进程日志目录
#define LOG_DAESRV "\\log\\daesrv"        // 守护进程日志目录
#define LOG_LIVECAST "\\log\\livecast"    // 开播日志目录
#define LOG_CAPRENDER "\\log\\capRender"  // 采集和渲染日志目录

#define CLOGFILE_SUFFIX "clog"  // LOG4CPP日志文件后缀标识