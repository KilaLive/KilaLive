#include "CommonTitelBar.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QApplication>
#include <QMessageBox>
#include <QMouseEvent>
#include <QWidget>
#include <QToolButton>

CCommonTitelBar::CCommonTitelBar(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground);
    labTitle = new QLabel;
    labTitle->setObjectName("commonTitle");
    btnMin = new QToolButton(this);
    btnMin->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    btnMin->setObjectName(QString::fromLocal8Bit("SystemMinButton"));
    btnMin->setToolTip(tr("Minimize"));
    connect(btnMin, SIGNAL(clicked()), this, SIGNAL(showMin()));

    btnMax = new QToolButton(this);
    btnMax->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    btnMax->setObjectName(QString::fromLocal8Bit("SystemMaxButton"));
    btnMax->setToolTip(tr("Maximize"));
    connect(btnMax, SIGNAL(clicked()), this, SIGNAL(showMax()));

    btnClose = new QToolButton(this);
    btnClose->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    btnClose->setObjectName(QString::fromUtf8("SystemCloseButton"));
    btnClose->setToolTip(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SIGNAL(dialogQuit()));

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(labTitle, 0, Qt::AlignVCenter);
    labTitle->setContentsMargins(10, 0, 0, 0);
    mainLayout->addStretch();

    mainLayout->addWidget(btnMin);
    mainLayout->addWidget(btnMax);
    mainLayout->addWidget(btnClose);

    mainLayout->setSpacing(3);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);
    isMove     = false;
    iSysBtnNum = 3;
}

CCommonTitelBar::~CCommonTitelBar() {}

void CCommonTitelBar::setSysBtnHide(bool bMinBtn, bool bMaxBtn, bool bCloseBtn)
{
    if (bMinBtn)
    {
        if (!btnMin->isHidden())
        {
            iSysBtnNum = iSysBtnNum - 1;
            btnMin->hide();
        }
    }
    else
    {
        if (btnMin->isHidden())
        {
            iSysBtnNum = iSysBtnNum + 1;
            btnMin->show();
        }
    }
    if (bMaxBtn)
    {
        if (!btnMax->isHidden())
        {
            iSysBtnNum = iSysBtnNum - 1;
            btnMax->hide();
        }
    }
    else
    {
        if (btnMax->isHidden())
        {
            iSysBtnNum = iSysBtnNum + 1;
            btnMax->show();
        }
    }
    if (bCloseBtn)
    {
        if (!btnClose->isHidden())
        {
            iSysBtnNum = iSysBtnNum - 1;
            btnClose->hide();
        }
    }
    else
    {
        if (btnClose->isHidden())
        {
            iSysBtnNum = iSysBtnNum + 1;
            btnClose->show();
        }
    }
}

void CCommonTitelBar::mousePressEvent(QMouseEvent *e)
{
    //如果点在按钮区域  不会移动 立即返回
    if (e->x() + (39 * iSysBtnNum) >= this->width())
    {
        e->ignore();
        return;
    }
    pressedPoint = e->pos();
    isMove       = true;

    return __super::mousePressEvent(e);
}
void CCommonTitelBar::mouseMoveEvent(QMouseEvent *e)
{
    if ((e->buttons() & Qt::LeftButton) && isMove)
    {
        QWidget *parWidget = parentWidget();
        if (parWidget == NULL)
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
void CCommonTitelBar::mouseReleaseEvent(QMouseEvent *e)
{
    if (isMove) isMove = false;

    return __super::mouseReleaseEvent(e);
}
void CCommonTitelBar::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (!btnMax->isHidden())
    {
        if (e->x() + 109 >= this->width())
        {
            e->ignore();
            return;
        }

        emit showMax();
    }

    return __super::mouseDoubleClickEvent(e);
}

void CCommonTitelBar::setTitle(const QString &strTitle)
{
    labTitle->setText(strTitle);
}

QToolButton *CCommonTitelBar::getMinBtn()
{
    return btnMin;
}

QToolButton *CCommonTitelBar::getMaxBtn()
{
    return btnMax;
}

QToolButton *CCommonTitelBar::getCloseBtn()
{
    return btnClose;
}
