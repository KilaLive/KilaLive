#include "LoginWidget.h"
#include "AnchorCenterWidget.h"
#include "HtmlProcess.h"
#include <QtWidgets/QApplication>
#include <QTranslator>
#include <QDesktopWidget>
#include <QObject>
//! 进程处理对象
CHtmlProcess TheHtmlProcess;

double getSysDpi()
{
    double dDpi          = 1;
    HDC    desktopDc     = GetDC(NULL);
    float  horizontalDPI = GetDeviceCaps(desktopDc, LOGPIXELSX);
    float  verticalDPI   = GetDeviceCaps(desktopDc, LOGPIXELSY);

    float dpiA = (float)GetDeviceCaps(desktopDc, DESKTOPHORZRES) / (float)GetDeviceCaps(desktopDc, HORZRES);
    float dpiB = (float)GetDeviceCaps(desktopDc, LOGPIXELSX) / 0.96 / 100;
    ::ReleaseDC(NULL, desktopDc);
    if (dpiA == 1.0)
    {
        return dpiB;
    }
    else if (dpiB == 1.0)
    {
        return dpiA;
    }
    else if (dpiA == dpiB)
    {
        return dpiA;
    }
    else
    {
        return 1.0;
    }
}

//根据用户屏幕分辨率以及缩放倍数调整程序缩放因子
//默认为1.0，标准分辨率为1920*1080
QString GetScaleRate()
{
    HWND hwd    = ::GetDesktopWindow();
    HDC  hdc    = ::GetDC(hwd);
    int  width  = GetDeviceCaps(hdc, DESKTOPHORZRES);  // device max width
    int  height = GetDeviceCaps(hdc, DESKTOPVERTRES);  // device max height

    // int   cxScreen = GetSystemMetrics(SM_CXSCREEN);  // screen width
    // int   cyScreen = GetSystemMetrics(SM_CYSCREEN);  // screen height
    float xScale   = width / 1920.0;
    float yScale   = height / 1080.0;
    float minScale = xScale < yScale ? xScale : yScale;

    if (minScale <= 1.0)
    {
        minScale = 1.0;
    }
    else
    {
        if (width == 3840 && height == 2160)
        {
            // 4k：3840x2160
            minScale = 1.0 + (minScale - 1.0) * 0.5;
        }
        else if (width == 2560 && height == 1440)
        {
            // 2k：2560 x1440
            minScale = 1.0;
        }
        else
        {
            //产品定的其他尺寸照旧不映射
        }
    }

    double dpi = getSysDpi();
    //缩放因子小于125% 设置无效，未找到解决方案
    if (dpi < 1.26)
    {
        dpi = 1.0;
    }
    QString strScaleRate = QString::number(minScale * 1.0 / dpi);
    return strScaleRate;
}

int main(int argc, char* argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    qputenv("QT_SCALE_FACTOR", GetScaleRate().toUtf8());

    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/KilaHtmlProcess/resource/skin/logo.ico"));

    //! 加载翻译文件
    QTranslator* trans = new QTranslator;
    trans->load(":/translator/resource/translator/kilamain_zh.qm");

    qApp->installTranslator(trans);

    //! 加载样式表
    QString skinPath = QString(":/KilaHtmlProcess/resource/skin/ubuntuSkin.qss");
    QFile   file(skinPath);
    if (file.open(QFile::ReadOnly))
    {
        QString styleSheet = QLatin1String(file.readAll());
        qApp->setStyleSheet(styleSheet);
        file.close();
    }

    //! 返回进程间通信数据
    TheHtmlProcess.ReceiveProcess();
    HtmlShareData* sharedData = TheHtmlProcess.GetShareData();

    int result = 0;

    //! 登录
    if (sharedData->type == 0)
    {
        //! 创建显示窗口
        CLoginWidget* loginWidget = new CLoginWidget(0x0, &TheHtmlProcess);

        QString title = QString::fromStdString(std::string(sharedData->title));
        loginWidget->Create(title);
        loginWidget->move(sharedData->x, sharedData->y);
        loginWidget->Show();

        loginWidget->LoadUrl(QString::fromStdString(sharedData->url), QString::fromStdString(sharedData->uid),
                             QString::fromStdString(sharedData->loginToken));

        result = a.exec();

        //! 释放登录窗口
        delete loginWidget;
        loginWidget = 0x0;
    }
    //! 主播中心
    else if (sharedData->type == 1 || sharedData->type == 2)
    {
        CAnchorCenterWidget* anchorCenter = new CAnchorCenterWidget(nullptr, &TheHtmlProcess);
        if (sharedData->type == 1)
        {
            anchorCenter->setWindowTitle(QObject::tr("anchor center"));
        }
        else if (sharedData->type == 2)
        {
            anchorCenter->SetShowFront(false);
            anchorCenter->setWindowTitle(QObject::tr("daily gift rank"));
        }
        anchorCenter->move(sharedData->x, sharedData->y);
        anchorCenter->LoadUrl(QString::fromStdString(sharedData->url), QString::fromStdString(sharedData->uid),
                              QString::fromStdString(sharedData->loginToken), sharedData->version);
        anchorCenter->Show();
        anchorCenter->setFixedSize(sharedData->width, sharedData->height);

        result = a.exec();

        //! 释放登录窗口
        delete anchorCenter;
        anchorCenter = 0x0;
    }
    //! 协议展示窗口
    else if (sharedData->type == 3)
    {
        //! 创建显示窗口
        CLoginWidget* loginWidget = new CLoginWidget(0x0, &TheHtmlProcess);

        QString title = QString::fromStdString(std::string(sharedData->title));
        loginWidget->Create(title);
        loginWidget->move(sharedData->x, sharedData->y);
        loginWidget->setFixedSize(sharedData->width, sharedData->height);
        loginWidget->Show();

        loginWidget->LoadUrl(QString::fromStdString(sharedData->url), QString::fromStdString(sharedData->uid),
                             QString::fromStdString(sharedData->loginToken), sharedData->isChangeScrollBarCss);

        result = a.exec();

        //! 释放登录窗口
        delete loginWidget;
        loginWidget = 0x0;
    }

    return result;
}
