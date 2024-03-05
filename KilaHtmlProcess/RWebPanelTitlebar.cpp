#include "RWebPanelTitlebar.h"
#include <QMouseEvent>
CRWebPanelTitlebar::CRWebPanelTitlebar(QWidget *parent, bool frontShow)
    : QWidget(parent)
    , m_isFront(frontShow)
{
    ui.setupUi(this);
    setObjectName("RWebPanelTitlebar");

    m_parent = parent;
    isMove   = false;

    setAttribute(Qt::WA_StyledBackground);
    ui.toolButton_close->setCursor(Qt::PointingHandCursor);
    ui.toolButton_min->setCursor(Qt::PointingHandCursor);
    ui.toolButton_top->setCursor(Qt::PointingHandCursor);
    ui.toolButton_back->setCursor(Qt::PointingHandCursor);

    if (m_isFront)
    {
        ui.toolButton_top->setStyleSheet("border-image: url(:/KilaHtmlProcess/resource/skin/btn_show_front_checked.png);");
    }
    else
    {
        ui.toolButton_top->setStyleSheet("border-image: url(:/KilaHtmlProcess/resource/skin/btn_show_front.png);");
    }

    connect(ui.toolButton_close, SIGNAL(clicked()), this, SLOT(OnCloseWnd()));
    connect(ui.toolButton_min, SIGNAL(clicked()), this, SLOT(OnMinWnd()));
    connect(ui.toolButton_top, SIGNAL(clicked()), this, SLOT(OnFrontShowWnd()));
    connect(ui.toolButton_back, SIGNAL(clicked()), this, SLOT(OnBack()));
}

CRWebPanelTitlebar::~CRWebPanelTitlebar() {}

void CRWebPanelTitlebar::showBackButton(bool show)
{
    ui.toolButton_back->setVisible(show);
}

void CRWebPanelTitlebar::mousePressEvent(QMouseEvent *e)
{
    if (e->x() + 40 >= this->width())
    {
        e->ignore();
        return;
    }
    pressedPoint = e->pos();
    isMove       = true;

    return __super::mousePressEvent(e);
}

void CRWebPanelTitlebar::mouseMoveEvent(QMouseEvent *e)
{
    if ((e->buttons() & Qt::LeftButton) && isMove)
    {
        QWidget *parWidget = m_parent;
        if (NULL == parWidget)
        {
            e->ignore();
            return;
        }

        QPoint nowParPoint = parWidget->pos();
        nowParPoint.setX(nowParPoint.x() + e->x() - pressedPoint.x());
        nowParPoint.setY(nowParPoint.y() + e->y() - pressedPoint.y());
        parWidget->move(nowParPoint);
    }

    return __super::mouseMoveEvent(e);
}

void CRWebPanelTitlebar::mouseReleaseEvent(QMouseEvent *e)
{
    if (isMove) isMove = false;

    return __super::mouseReleaseEvent(e);
}

void CRWebPanelTitlebar::OnBack()
{
    emit SigBackWindow();
}

void CRWebPanelTitlebar::OnFrontShowWnd()
{
    m_isFront = !m_isFront;
    if (m_isFront)
    {
        ui.toolButton_top->setStyleSheet("border-image: url(:/KilaHtmlProcess/resource/skin/btn_show_front_checked.png);");
    }
    else
    {
        ui.toolButton_top->setStyleSheet("border-image: url(:/KilaHtmlProcess/resource/skin/btn_show_front.png);");
    }
    emit SigShowFront();
}

void CRWebPanelTitlebar::OnMinWnd()
{
    emit SigMinWindow();
}

void CRWebPanelTitlebar::OnCloseWnd()
{
    emit SigClose();
}

void CRWebPanelTitlebar::showFront(bool isShowFront)
{
    m_isFront = isShowFront;
    if (m_isFront)
    {
        ui.toolButton_top->setStyleSheet("border-image: url(:/KilaHtmlProcess/resource/skin/btn_show_front_checked.png);");
    }
    else
    {
        ui.toolButton_top->setStyleSheet("border-image: url(:/KilaHtmlProcess/resource/skin/btn_show_front.png);");
    }
}