#include "TWebEngineUrlRequestInterceptor.h"

#define PROJECT_KILAKILA

TWebEngineUrlRequestInterceptor::TWebEngineUrlRequestInterceptor(QObject *parent)
    : QWebEngineUrlRequestInterceptor(parent)
{
}

void TWebEngineUrlRequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo &info)
{
#ifdef PROJECT_KILAKILA
    info.setHttpHeader("Accept-Language", "zh");
#else
    info.setHttpHeader("Accept-Language", "ja");
#endif
}