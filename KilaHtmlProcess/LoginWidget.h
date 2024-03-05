#pragma once

#include "ui_LoginWidget.h"
#include "CommonWidget.h"
#include <QPushButton>
#include <QDateTime>
#include <QNetworkProxyFactory>
#include <QWebEngineView>
#include <QNetworkCookie>
#include <QWebEngineCookieStore>
#include <QWebEngineProfile>
#include <QDir>
#include <QDesktopServices>

//跟SDK 类型对应
enum ELoginType
{
    ELogin_Phone  = 0,
    ELogin_Weibo  = 1,
    ELogin_Weixin = 2,
    ELogin_QQ     = 3,
    ELogin_Apple  = 19,
    ELogin_Unknow,
};

#define HONGDOU_HELP_DOCUMENT_URL L"https://pikapika.live/rule/pikaPrivacyProtocol"

class MyWebPage : public QWebEnginePage
{
    Q_OBJECT
public:
    MyWebPage(QObject *parent = 0)
        : QWebEnginePage(parent)
    {
    }

    bool acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool)
    {
        if (type == QWebEnginePage::NavigationTypeLinkClicked)
        {
            if (url.url() == QString::fromWCharArray(HONGDOU_HELP_DOCUMENT_URL))
            {
                QDesktopServices::openUrl(url);
                return false;
            }
        }
        return true;
    }

    virtual bool certificateError(const QWebEngineCertificateError &certificateError)
    {
        return true;
    }

signals:
    void linkClicked(const QUrl &);
};

class CHtmlProcess;

class CLoginWidget : public CCommonWidget
{
    Q_OBJECT

public:
    CLoginWidget(QWidget *parent, CHtmlProcess *process, CommonWndFlags WndFlag = enum_UXApplication);
    ~CLoginWidget();

public:
    void LoadUrl(QString url, QString uid, QString loginToken, bool isChangeScrollbarCss = false);
    void Create(const QString &title);
    void Destroy();

protected slots:
    void Close();

signals:
    void quitLogin();
    void loginSucc(ELoginType type, QString txSign, QString wbToken, QString token, QString outerID,
                   QDateTime expiredDateTime);

public slots:
    void onWebviewLoadStart();
    void onWebviewLoadFinished(bool ok);

private slots:
    void handleCookieAdded(const QNetworkCookie &cookie);

private:
    void clearBrowserCache();
    void changeHtmlCss();  //更新html的样式，关闭html 超出显示会出现滚动条，导致隐私协议界面双重滚动条的问题

private:
    QWebEngineView *        m_webView;
    QWebEngineCookieStore * m_store;
    QVector<QNetworkCookie> m_cookies;
    QString                 m_visitedUrl[2];
    QPushButton *           m_btnQuit;
    MyWebPage *             m_page = nullptr;

    ELoginType m_loginType;

    QString m_wbToken;
    QString m_wxToken;
    QString m_phoneToken;
    QString m_qqToken;
    QString m_authToken;
    QString m_txSign;

    CHtmlProcess *m_Process;

private:
    Ui::CLoginWidget ui;
};
