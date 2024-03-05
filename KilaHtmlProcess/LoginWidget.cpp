#include "LoginWidget.h"
#include "HtmlProcess.h"
#include "BaseLog.h"
#include <QWebEngineSettings>
#include "TWebEngineUrlRequestInterceptor.h"
#include <QWebEngineHttpRequest>
#include <QWebEngineHistory>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>

CLoginWidget::CLoginWidget(QWidget* parent, CHtmlProcess* process, CommonWndFlags WndFlag)
	: CCommonWidget(parent, WndFlag)
{
	m_Process = process;
	m_webView = NULL;
	ui.setupUi(this);
	TWebEngineUrlRequestInterceptor* webInterceptor = new TWebEngineUrlRequestInterceptor();
	QWebEngineProfile::defaultProfile()->setRequestInterceptor(webInterceptor);
}

CLoginWidget::~CLoginWidget()
{
	Destroy();
}

void CLoginWidget::Create(const QString& title)
{
	setObjectName("LoginWidget");

	//创建系统栏题栏等
	CrateSystemTitle(ui.TitleLayout);

	SetTitle(title);
	connect(this, SIGNAL(OnClose()), this, SLOT(Close()));
	m_webView = new QWebEngineView(this);

	m_page = new MyWebPage();
	m_webView->setPage(m_page);

	m_webView->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
	m_webView->settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, true);
	m_webView->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
	QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, true);

	ui.MainLayout->addWidget(m_webView);
	clearBrowserCache();
	connect(m_webView, SIGNAL(loadFinished(bool)), this, SLOT(onWebviewLoadFinished(bool)));
	connect(m_webView, SIGNAL(loadStarted()), this, SLOT(onWebviewLoadStart()));
	connect(m_store, &QWebEngineCookieStore::cookieAdded, this, &CLoginWidget::handleCookieAdded);
}

void CLoginWidget::Destroy()
{
	clearBrowserCache();
	if (m_webView)
	{
		m_webView->setPage(nullptr);
	}

	if (m_page)
	{
		delete m_page;
		m_page = nullptr;
	}

	if (NULL != m_webView)
	{
		delete m_webView;
		m_webView = NULL;
	}
}

void CLoginWidget::Close()
{
	HtmlShareData* sharedData = m_Process->GetShareData();
	sharedData->event = EventType::Close;
	m_Process->ActivateEvent();
	m_Process->StopProcess();

	//! 释放日志
	Log_Release();

	clearBrowserCache();
	close();
}

void CLoginWidget::LoadUrl(QString url, QString uid, QString loginToken, bool isChangeScrollbarCss)
{
	if (NULL != m_webView)
	{
		if (isChangeScrollbarCss)
		{
			changeHtmlCss();
		}

		m_webView->load(QUrl(url));
	}
}

void CLoginWidget::handleCookieAdded(const QNetworkCookie& cookie)
{
	m_cookies.append(cookie);
}

void CLoginWidget::clearBrowserCache()
{
	if (NULL == m_webView) return;

	if (m_webView->page() && m_webView->page()->profile())
	{
		m_webView->page()->profile()->clearHttpCache();
		m_store = m_webView->page()->profile()->cookieStore();
		if (m_store) m_store->deleteAllCookies();
		m_cookies.clear();

		QString cachePath = m_webView->page()->profile()->cachePath();
		QDir    cachePathDir(cachePath);
		if (cachePathDir.exists())
		{
			bool rlt = cachePathDir.rmdir(cachePath);
			if (!rlt)
			{
				qDebug() << tr("Delete Cache Directory Failed");
			}
		}
	}
}

void CLoginWidget::changeHtmlCss()
{
	if (m_webView == nullptr)
	{
		return;
	}

	QFile cssfile(":/KilaHtmlProcess/resource/css/loginWidget.css");
	if (cssfile.open(QFile::ReadOnly))
	{
		QTextStream   stream(&cssfile);
		const QString styleJS = QString::fromLatin1("(function() {"
			"    css = document.createElement('style');"
			"    css.type = 'text/css';"
			"    css.id = '%1';"
			"    document.head.appendChild(css);"
			"    css.innerText = '%2';"
			"})()\n")
			.arg("scrollbarStyle")
			.arg(stream.readAll().simplified());
		cssfile.close();
		QWebEngineScript script;
		script.setWorldId(QWebEngineScript::MainWorld);
		script.setSourceCode(styleJS);

		m_webView->page()->scripts().insert(script);
		m_webView->page()->runJavaScript(styleJS, QWebEngineScript::ApplicationWorld);
	}
}

void CLoginWidget::onWebviewLoadStart()
{
	// m_cookies.clear();
}

void CLoginWidget::onWebviewLoadFinished(bool ok)
{
	m_loginType = ELogin_Unknow;
	QString curUrl = m_webView->url().toString().toLower();
	if (!curUrl.contains("about:blank"))
	{
		m_visitedUrl[0] = m_visitedUrl[1];
		m_visitedUrl[1] = curUrl;
	}

	m_loginType = ELogin_Phone;

	Log_Info("Previous login url:%s", m_visitedUrl[0].toStdString().c_str());
	Log_Info("Current login url:%s", curUrl.toStdString().c_str());
	if (curUrl.contains("/11"))
	{
		if (m_visitedUrl[0].contains("api.weibo.com"))
		{
			m_loginType = ELogin_Weibo;
		}
		else if (m_visitedUrl[0].contains("graph.qq.com"))
		{
			m_loginType = ELogin_QQ;
		}
		else if (m_visitedUrl[0].contains("apple.com"))
		{
			m_loginType = ELogin_Apple;
		}

		if (m_loginType == ELogin_Unknow)
		{
			for (auto iter = m_cookies.rbegin(); iter != m_cookies.rend(); iter++)
			{
				QString cookieName = QString((*iter).name());
				if (cookieName.contains("wxtoken"))
				{
					m_loginType = ELogin_Weixin;
					break;
				}
				else if (cookieName.contains("phonetoken"))
				{
					m_loginType = ELogin_Phone;
					break;
				}
			}
		}

		QString strAppleToken = "";
		QString strOuterId = "";

		// x-auth-token
		QDateTime expirationDate;
		if (m_cookies.size() > 0)
		{
			expirationDate = (*(m_cookies.end() - 1)).expirationDate();
		}
		for (auto item : m_cookies)
		{
			QString cookieName = QString(item.name());
			QString cookieValue = QString(item.value());

			if (cookieName.contains("wbtoken"))
			{
				m_wbToken = cookieValue;
			}
			else if (cookieName.contains("wxtoken"))
			{
				m_wxToken = cookieValue;
			}
			else if (cookieName.contains("phonetoken"))
			{
				m_phoneToken = cookieValue;
			}
			else if (cookieName.contains("qqtoken"))
			{
				m_qqToken = cookieValue;
			}
			else if (cookieName.contains("x-auth-token"))
			{
				m_authToken = cookieValue;
			}
			else if (cookieName.contains("txSign"))
			{
				m_txSign = cookieValue;
			}
			else if (cookieName.contains("appletoken"))
			{
				strAppleToken = cookieValue;
			}
			else if (cookieName.contains("outerId"))
			{
				strOuterId = cookieValue;
			}
		}

		clearBrowserCache();

		if (m_loginType != ELogin_Unknow && m_loginType != ELogin_Apple)
		{
			// emit(loginSucc(m_loginType, m_txSign, m_wbToken, m_authToken,"", expirationDate));

			HtmlShareData* sharedData = m_Process->GetShareData();
			sharedData->event = EventType::Login;
			sharedData->loginType = m_loginType;

			std::wstring temp = m_txSign.toStdWString();
			wcscpy(sharedData->txSign, temp.c_str());

			temp = m_wbToken.toStdWString();
			wcscpy(sharedData->wbToken, temp.c_str());

			temp = m_authToken.toStdWString();
			wcscpy(sharedData->authToken, temp.c_str());

			m_Process->ActivateEvent();
			m_Process->StopProcess();

			//! 释放日志
			Log_Release();

			clearBrowserCache();
			close();

			return;
		}
		else if (m_loginType == ELogin_Apple)
		{
			// emit(loginSucc(m_loginType, m_txSign, m_wbToken, strAppleToken, strOuterId, expirationDate));

			HtmlShareData* sharedData = m_Process->GetShareData();
			sharedData->event = EventType::Login;
			sharedData->loginType = m_loginType;

			std::wstring temp = m_txSign.toStdWString();
			wcscpy(sharedData->txSign, temp.c_str());

			temp = m_wbToken.toStdWString();
			wcscpy(sharedData->wbToken, temp.c_str());

			temp = strAppleToken.toStdWString();
			wcscpy(sharedData->appleToken, temp.c_str());

			temp = strOuterId.toStdWString();
			wcscpy(sharedData->outerId, temp.c_str());

			m_Process->ActivateEvent();
			m_Process->StopProcess();

			//! 释放日志
			Log_Release();

			clearBrowserCache();
			close();

			return;
		}
	}
	//! 同意协议后跳转的url
	else if (curUrl.contains("/1"))
	{
		Log_Info("After agree the doc, close the window, url:%s", curUrl.toStdString().c_str());

		HtmlShareData* sharedData = m_Process->GetShareData();
		sharedData->event = EventType::Close;
		m_Process->ActivateEvent();
		m_Process->StopProcess();

		//! 释放日志
		Log_Release();

		clearBrowserCache();
		close();
	}
	else if (curUrl.contains("/0"))
	{
		//重试
		clearBrowserCache();
	}
}
