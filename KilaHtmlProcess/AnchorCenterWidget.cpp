#include "AnchorCenterWidget.h"
#include <QWebEngineHistory>
#include "RWebPanelTitlebar.h"
#include "HtmlProcess.h"
#include "BaseLog.h"
#include "CommunicatH5.h"
#include <QWebEngineHttpRequest>
#include <windows.h>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QByteArray>

CAnchorCenterWidget::CAnchorCenterWidget(QWidget* parent, CHtmlProcess* process)
    : QWidget(parent)
    , m_bFrontShow(true)
{
    m_Process = process;
    ui.setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);

    m_pTitleBar = new CRWebPanelTitlebar(this, m_bFrontShow);
    m_pTitleBar->showBackButton(false);
    connect(m_pTitleBar, SIGNAL(SigMinWindow()), this, SLOT(showMinimized()));
    connect(m_pTitleBar, SIGNAL(SigClose()), this, SLOT(OnCloseWnd()));
    connect(m_pTitleBar, SIGNAL(SigShowFront()), this, SLOT(OnFrontShowWnd()));
    connect(m_pTitleBar, SIGNAL(SigBackWindow()), this, SLOT(OnBack()));

    ui.frame->layout()->addWidget(m_pTitleBar);

    m_webView                = new QWebEngineView(this);
    QWebEnginePage* page     = new QWebEnginePage(this);
    QWebChannel*    pChannel = new QWebChannel(this);
    CommunicatH5*   pCommH5  = new CommunicatH5();
    connect(pCommH5, &CommunicatH5::textReceived, this, &CAnchorCenterWidget::OnTextReceived);
    m_webView->setAttribute(Qt::WA_DeleteOnClose);
    m_webView->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    m_webView->settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, true);
    m_webView->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    pChannel->registerObject(QStringLiteral("channelObject"), pCommH5);
    m_webView->setPage(page);
    page->setWebChannel(pChannel);
    connect(m_webView, SIGNAL(loadFinished(bool)), this, SLOT(onWebviewLoadFinished(bool)));
    connect(m_webView, &QWebEngineView::urlChanged, [&](const QUrl& url) {
        bool isMp4 = url.url().endsWith(".mp4");
        if (isMp4)
        {
            QDesktopServices::openUrl(url);
            close();
            return;
        }
    });

    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, true);
    QNetworkProxyFactory::setUseSystemConfiguration(false);
    ui.frame_browser->layout()->addWidget(m_webView);

    ::SetWindowPos((HWND)this->winId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

CAnchorCenterWidget::~CAnchorCenterWidget()
{
    if (NULL != m_webView)
    {
        delete m_webView;
        m_webView = NULL;
    }
}

void CAnchorCenterWidget::Show()
{
    this->show();
    if (m_bFrontShow)
    {
        ::SetWindowPos((HWND)this->winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    else
    {
        ::SetWindowPos((HWND)this->winId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
}

void CAnchorCenterWidget::LoadUrl(QString url, QString uid, QString loginToken, int version)
{
    QUrl                  Qurl = QUrl(url);
    QWebEngineHttpRequest httpRequest(Qurl);
    httpRequest.setHeader("x-auth-token", loginToken.toUtf8());
    httpRequest.setHeader("uid", uid.toUtf8());
    httpRequest.setHeader("os", "windows");
    httpRequest.setHeader("vc", QString::number(version).toUtf8());
    m_webView->load(httpRequest);
}

void CAnchorCenterWidget::onWebviewLoadFinished(bool ok)
{
    m_pTitleBar->showBackButton(m_webView->history()->canGoBack());
}

void CAnchorCenterWidget::OnBack()
{
    if (m_webView->history()->canGoBack())
    {
        m_webView->history()->back();
    }
}

void CAnchorCenterWidget::OnFrontShowWnd()
{
    m_bFrontShow = !m_bFrontShow;
    if (m_bFrontShow)
    {
        ::SetWindowPos((HWND)this->winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    else
    {
        ::SetWindowPos((HWND)this->winId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
}

void CAnchorCenterWidget::OnMinWnd()
{
    showMinimized();
}

void CAnchorCenterWidget::OnCloseWnd()
{
    HtmlShareData* sharedData = m_Process->GetShareData();
    sharedData->event         = EventType::Close;
    m_Process->ActivateEvent();
    m_Process->StopProcess();

    //! ÊÍ·ÅÈÕÖ¾
    Log_Release();

    close();
}

void CAnchorCenterWidget::OnTextReceived(const QString& text)
{
    // 1:创建直播间预告弹窗 2:守护团详情弹窗 3:问题反馈弹窗 4:房间管理弹窗 5:私信弹窗
    // 将信息解析出来发送至主进程
    HtmlShareData* sharedData = m_Process->GetShareData();
    sharedData->event         = EventType::H5Skip;
    std::wstring temp         = text.toStdWString();
    wcscpy(sharedData->h5Text, temp.c_str());
    m_Process->ActivateEvent();
}

void CAnchorCenterWidget::SetShowFront(bool isShowFront)
{
    m_bFrontShow = isShowFront;
    m_pTitleBar->showFront(isShowFront);
    if (m_bFrontShow)
    {
        ::SetWindowPos((HWND)this->winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    else
    {
        ::SetWindowPos((HWND)this->winId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
}