#pragma once

#include <QWidget>
#include "ui_RWebPanelTitlebar.h"

class CRWebPanelTitlebar : public QWidget
{
    Q_OBJECT

public:
    CRWebPanelTitlebar(QWidget *parent = Q_NULLPTR, bool frontShow = false);
    ~CRWebPanelTitlebar();

    void showBackButton(bool show);
    void showFront(bool isShowFront);

Q_SIGNALS:
    void SigClose();
    void SigMinWindow();
    void SigShowFront();
    void SigBackWindow();

public slots:
    void OnBack();
    void OnFrontShowWnd();
    void OnMinWnd();
    void OnCloseWnd();

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

private:
    Ui::CRWebPanelTitlebar ui;

    QPoint   pressedPoint;
    bool     isMove;
    bool     m_isFront = false;
    QWidget *m_parent;
};
