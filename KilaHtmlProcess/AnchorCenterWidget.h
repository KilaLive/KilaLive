#pragma once

#include <QWidget>
#include "ui_AnchorCenterWidget.h"
#include <QWebEngineView>
#include <QWebEngineSettings>
#include <QWebEngineScript>
#include <QWebChannel>
#include <QWebEngineProfile>
#include <QNetworkProxyFactory>
#include <QNetworkCookie>
#include <QWebEngineCookieStore>
#include <QDir>
#include <QDesktopServices>

class CRWebPanelTitlebar;
class CHtmlProcess;

class CAnchorCenterWidget : public QWidget
{
    Q_OBJECT

public:
    CAnchorCenterWidget(QWidget* parent = Q_NULLPTR, CHtmlProcess* process = 0x0);
    ~CAnchorCenterWidget();

    void LoadUrl(QString url, QString uid, QString loginToken, int version = 0);

    void Show();

    void SetShowFront(bool isShowFront);

private:
public slots:
    void onWebviewLoadFinished(bool ok);

    void OnBack();

    void OnFrontShowWnd();

    void OnMinWnd();

    void OnCloseWnd();

    void OnTextReceived(const QString& text);

private:
    Ui::CAnchorCenterWidget ui;

private:
    QWebEngineView*     m_webView;
    CRWebPanelTitlebar* m_pTitleBar;
    bool                m_bFrontShow = false;
    CHtmlProcess*       m_Process;
};
