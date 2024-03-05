#ifndef H_RegistryUtil_H
#define H_RegistryUtil_H

#include "KilaUtils.h"
#include <string>
#include <vector>

class KILAUTILS_API CRegistryUtil final
{
public:
    //! 读取注册表
    static bool ReadRegistry(const char* path, const char* key, char* value);

    //! 写入注册表
    static bool WriteRegistry(const char* path, const char* key, const char* value);

    //! 检查上次关播时间到现在是否已经超过10s
    static bool CheckCloseTimeBeyond10Second(void);

    //! 检查上次退出媒体房间的时间是否超过10s
    static bool CheckExitMediaRoomBeyond10Second(void);
};

#endif
