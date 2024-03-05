#ifndef __HtmlProcess_H__
#define __HtmlProcess_H__

#include <windows.h>
#include <string>
#include <functional>

#define SHMEM_HTML_NAME "Kila_Html_SharedMemory"
#define EVENT_EVENT_LISTEN "Kila_Event_Listen"
#define EVENT_HTML_KEEPALIVE "Kila_Event_KeepAlive"
#define EVENT_HTML_DESTROY "Kila_Event_Destroy"

//! 事件类型
struct EventType
{
    enum List
    {
        Init,
        Login,
        Close,
        Error,
        H5Skip
    };
};

//! 内存共享数据
struct HtmlShareData
{
    //! 传入的参数
    EventType::List event;
    int             type;  //! 0:登录 1:主播中心 2: 协议
    char            title[1024];
    char            logPath[1024];
    char            url[1024];
    char            uid[1024];
    char            loginToken[1024];
    int             x;
    int             y;
    int             width;
    int             height;
    int             version              = 0;
    bool            isChangeScrollBarCss = false;

    //! 传出的参数
    int     loginType;
    wchar_t txSign[2048];
    wchar_t wbToken[2048];
    wchar_t authToken[2048];
    wchar_t appleToken[2048];
    wchar_t outerId[2048];
    wchar_t h5Text[2048];
};

//! Html进程间处理
class CHtmlProcess
{
public:
    //! 构造函数
    CHtmlProcess(void);

    //! 析构函数
    ~CHtmlProcess(void);

    //! 开始进程处理
    int ReceiveProcess(void);

    //! 返回共享对象
    HtmlShareData* GetShareData(void)
    {
        return m_SharedComunicationData;
    }

    //! 激活事件
    void ActivateEvent(void)
    {
        SetEvent(m_EventListen);
    }

    //! 停止处理
    void StopProcess(void);

private:
    //! 心跳线程
    static void KeepaliveThreadProcProxy(void* param);
    void        KeepaliveThreadProc(void);

    //! 数据成员
    HANDLE         m_ShareComunication      = 0x0;
    HtmlShareData* m_SharedComunicationData = 0x0;

    //! 需要加载的url地址
    std::string m_URL            = "";
    HANDLE      m_EventListen    = 0x0;
    HANDLE      m_EventKeepalive = 0x0;
    HANDLE      m_EventDestroy   = 0x0;

    bool   m_Exiting         = false;
    HANDLE m_KeepaliveThread = 0x0;
};

#endif
