#pragma once
#include <QObject>
#include <QWebEngineView>
#include <QWebEngineUrlRequestInterceptor>
#include <QWebEngineUrlRequestInfo>
class TWebEngineUrlRequestInterceptor : public QWebEngineUrlRequestInterceptor
{
    // Q_OBJECT
public:
    TWebEngineUrlRequestInterceptor(QObject *parent = Q_NULLPTR);
    virtual void interceptRequest(QWebEngineUrlRequestInfo &info);
};