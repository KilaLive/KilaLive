#include "CommonWidget.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QStyleOption>
#include <windows.h>

CCommonWidget::CCommonWidget(QWidget* parent, CommonWndFlags WndFlag, Qt::WindowFlags f)
    : QWidget(parent, f | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::Dialog)
{
    m_lWndFlag         = WndFlag;
    m_iRoundCornerSize = 6;
    m_pTitleBar        = NULL;
    setAttribute(Qt::WA_StyledBackground);
    m_pParent   = parent;
    m_pTitleBar = new CCommonTitelBar(this);
    m_pTitleBar->setFixedHeight(46);

    CrateSystemButton(WndFlag);
}

CCommonWidget::~CCommonWidget()
{
    if (NULL != m_pTitleBar)
    {
        delete m_pTitleBar;
        m_pTitleBar = NULL;
    }
}

//在指定区创建按钮
void CCommonWidget::CrateSystemButton(CommonWndFlags WndFlag)
{
    bool bShowMin   = false;
    bool bShowMax   = false;
    bool bShowClose = false;
    bShowMin        = WndFlag & enum_UXWndMinimizeButtonHint;
    bShowMax        = WndFlag & enum_UXWndMaximizeButtonHint;
    bShowClose      = WndFlag & enum_UXWndCloseButtonHint;

    if (NULL != m_pTitleBar)
    {
        m_pTitleBar->setSysBtnHide(!bShowMin, !bShowMax, !bShowClose);
        connect(m_pTitleBar->getCloseBtn(), SIGNAL(clicked()), this, SIGNAL(OnClose()));
        connect(m_pTitleBar->getMaxBtn(), SIGNAL(clicked()), this, SLOT(OnMaxClick()));
        connect(m_pTitleBar->getMinBtn(), SIGNAL(clicked()), this, SLOT(OnMinClick()));
    }
}

void CCommonWidget::CrateSystemTitle(QLayout* layout)
{
    if (NULL != m_pTitleBar) layout->addWidget(m_pTitleBar);
}

//设置标题高度
void CCommonWidget::SetTitlebarHeight(int aiHeight)
{
    if (NULL != m_pTitleBar) m_pTitleBar->setFixedHeight(aiHeight);
}

//设置标题
void CCommonWidget::SetTitle(const QString& strTitle)
{
    if (m_pTitleBar)
    {
        m_pTitleBar->setTitle(strTitle);
    }
}

//设置系统任务栏标题
void CCommonWidget::SetTaskBarTitle(const QString& strTitle)
{
    setWindowTitle(strTitle);
}

//设置圆角
void CCommonWidget::SetRoundCorner(int iSize)
{
    if (iSize < 0) return;

    m_iRoundCornerSize = iSize;
    if (iSize > 0)
    {
        MakeRoundCorner();
    }
}

//窗口居中
void CCommonWidget::CenterWindow(QWidget* parent)
{
    int x = 0;
    int y = 0;
    if (NULL == parent)
    {
        const QRect rect = QApplication::desktop()->availableGeometry();
        x                = rect.left() + (rect.width() - width()) / 2;
        y                = rect.top() + (rect.height() - height()) / 2;
    }
    else
    {
        QPoint point(0, 0);
        point = parent->mapToGlobal(point);
        x     = point.x() + (parent->width() - width()) / 2;
        y     = point.y() + (parent->height() - height()) / 2;
    }

    //! 为了不遮挡登录窗口，错开一点位置
    move(x, y);
}

//设置最前
void CCommonWidget::Foreground(bool abShowFirst /* = true*/)
{
    if (abShowFirst)
    {
        show();
    }
    SwitchToThisWindow((HWND)winId(), TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCommonWidget::OnMaxClick()
{
    showMaximized();
}

void CCommonWidget::OnMinClick()
{
    showMinimized();
}

void CCommonWidget::resizeEvent(QResizeEvent* event)
{
    MakeRoundCorner();
}

void CCommonWidget::MakeRoundCorner()
{
    // if(m_iRoundCornerSize> 0)
    {
        RECT  rect, wndRect;
        POINT point = {0, 0};
        ::GetClientRect((HWND)winId(), &rect);
        ::ClientToScreen((HWND)winId(), &point);
        ::GetWindowRect((HWND)winId(), &wndRect);

        HRGN hRgn =
            ::CreateRoundRectRgn(point.x - wndRect.left, point.y - wndRect.top, rect.right + point.x - wndRect.left + 1,
                                 rect.bottom + point.y - wndRect.top + 1, m_iRoundCornerSize, m_iRoundCornerSize);
        if (hRgn)
        {
            if (::SetWindowRgn((HWND)winId(), hRgn, TRUE) == 0)
            {
                ::DeleteObject(hRgn);
            }
        }
    }
}

void CCommonWidget::Show(bool abShowMode)
{
    if (abShowMode) setWindowModality(Qt::ApplicationModal);

    if (m_pParent) CenterWindow(m_pParent);

    if (isMinimized())
    {
        showNormal();
    }
    else
    {
        QWidget::show();
    }
}

void CCommonWidget::SlotOnClose()
{
    close();
}

void CCommonWidget::showEvent(QShowEvent* event)
{
    if (isModal())
    {
        // CSingleton<WidgetShowHideObject>::Instance()->PostShowEvent(true);
    }
}

void CCommonWidget::hideEvent(QHideEvent* event)
{
    if (isModal())
    {
        // CSingleton<WidgetShowHideObject>::Instance()->PostShowEvent(false);
    }
}
