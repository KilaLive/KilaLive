#include "HtmlProcess.h"
#include "BaseLog.h"
#include <TlHelp32.h>
#include <QtWidgets/QApplication>
#include <QDir>

//! 构造函数
CHtmlProcess::CHtmlProcess(void) {}

//! 析构函数
CHtmlProcess::~CHtmlProcess(void) {}

//! 开始进程处理
int CHtmlProcess::ReceiveProcess(void)
{
    //! 打开内存共享
    std::string shared_memory_name(SHMEM_HTML_NAME);
    m_ShareComunication = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(HtmlShareData),
                                             shared_memory_name.c_str());
    m_SharedComunicationData =
        (HtmlShareData*)MapViewOfFile(m_ShareComunication, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(HtmlShareData));
    if (m_SharedComunicationData == 0x0)
    {
        return -1;
    }

    //! 读取路径
    std::string logfile = std::string(m_SharedComunicationData->logPath);
    Log_Init(logfile.c_str());
    Log_SetLogLevel(ELL_Info);

    //! 设置初始值
    m_URL = std::string(m_SharedComunicationData->url);

//! 创建事件
#define CREATE_EVENT(name) CreateEventA(nullptr, FALSE, FALSE, (std::string(name)).c_str());

    m_EventListen = CREATE_EVENT(EVENT_EVENT_LISTEN);
    if (m_EventListen == 0x0)
    {
        DWORD errCode = GetLastError();
        Log_Error("Failed to open listen event, error:%lu", errCode);

        return -1;
    }

    m_EventKeepalive = CREATE_EVENT(EVENT_HTML_KEEPALIVE);
    if (m_EventKeepalive == 0x0)
    {
        DWORD errCode = GetLastError();
        Log_Error("Failed to open keeplive event, error:%lu", errCode);

        return -1;
    }

    m_EventDestroy = CreateEventA(NULL, TRUE, FALSE, EVENT_HTML_DESTROY);
    if (m_EventDestroy == 0x0)
    {
        DWORD errCode = GetLastError();
        Log_Error("Failed to create destroy event, error:%lu", errCode);

        return -1;
    }

#undef CREATE_EVENT

    //! 监听事件
    m_Exiting = false;
    m_KeepaliveThread =
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)CHtmlProcess::KeepaliveThreadProcProxy, this, 0, nullptr);
    if (m_KeepaliveThread == 0x0)
    {
        DWORD errCode = GetLastError();
        Log_Error("Failed to create keeplive thread, error:%lu", errCode);

        return -1;
    }

    return 0;
}

//! 停止处理
void CHtmlProcess::StopProcess(void)
{
    m_Exiting = true;
    SetEvent(m_EventDestroy);

    if (m_KeepaliveThread)
    {
        WaitForSingleObject(m_KeepaliveThread, INFINITE);
        CloseHandle(m_KeepaliveThread);
        m_KeepaliveThread = 0x0;
    }

    CloseHandle(m_EventDestroy);
    m_EventDestroy = 0x0;
}

//! 心跳线程
void CHtmlProcess::KeepaliveThreadProcProxy(void* param)
{
    ((CHtmlProcess*)param)->KeepaliveThreadProc();
}

//! 心跳线程
void CHtmlProcess::KeepaliveThreadProc(void)
{
    while (!m_Exiting)
    {
        if (m_EventKeepalive)
        {
            SetEvent(m_EventKeepalive);
        }

        WaitForSingleObject(m_EventDestroy, 1000);
    }
}
