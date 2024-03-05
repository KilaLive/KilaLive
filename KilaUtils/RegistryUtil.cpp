#include "RegistryUtil.h"
#include <Windows.h>

//! 读取注册表
bool CRegistryUtil::ReadRegistry(const char* path, const char* key, char* value)
{
    HKEY hKey;
    int  ret = RegOpenKeyExA(HKEY_CURRENT_USER, path, 0, KEY_EXECUTE, &hKey);
    if (ret != ERROR_SUCCESS)
    {
        return false;
    }

    //! 读取KEY
    DWORD dwType = REG_SZ;
    DWORD cbData = 256;
    ret          = RegQueryValueExA(hKey, key, NULL, &dwType, (LPBYTE)value, &cbData);
    if (ret != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);

    return true;
}

//! 写入注册表
bool CRegistryUtil::WriteRegistry(const char* path, const char* key, const char* value)
{
    HKEY  hKey;
    DWORD dwDisp;
    DWORD dwType = REG_SZ;

    int ret =
        RegCreateKeyExA(HKEY_CURRENT_USER, path, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisp);
    if (ret != ERROR_SUCCESS)
    {
        return 1;
    }

    ret = RegSetValueExA(hKey, key, 0, dwType, (BYTE*)value, strlen(value));
    if (ret != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return 1;
    }

    RegCloseKey(hKey);

    return 0;
}

//! 检查上次关播时间到现在是否已经超过10s
bool CRegistryUtil::CheckCloseTimeBeyond10Second(void)
{
    //! 读取注册表
    char closeRoomTime[256] = {0};
    bool result             = CRegistryUtil::ReadRegistry("Software\\KilaKila", "close_room_time", closeRoomTime);
    if (result)
    {
        int64_t closeTime   = atoll(closeRoomTime);
        int64_t currentTime = GetTickCount64();
        int64_t diffTime    = currentTime - closeTime;
        if (diffTime < (10 * 1000) && diffTime > 0)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    return true;
}

//! 检查上次退出媒体房间的时间是否超过10s
bool CRegistryUtil::CheckExitMediaRoomBeyond10Second(void)
{
    //! 读取注册表
    char closeRoomTime[256] = {0};
    bool result             = CRegistryUtil::ReadRegistry("Software\\KilaKila", "exit_mediaroom_time", closeRoomTime);
    if (result)
    {
        int64_t closeTime   = atoll(closeRoomTime);
        int64_t currentTime = GetTickCount64();
        int64_t diffTime    = currentTime - closeTime;
        if (diffTime < (10 * 1000) && diffTime > 0)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    return true;
}
