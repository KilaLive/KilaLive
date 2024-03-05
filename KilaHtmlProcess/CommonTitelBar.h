#ifndef CCOMMONTITELBAR_H
#define CCOMMONTITELBAR_H
#include <QWidget>

class QLabel;
class QToolButton;
class CCommonTitelBar : public QWidget
{
    Q_OBJECT

public:
    CCommonTitelBar(QWidget* parent = 0);
    ~CCommonTitelBar();

Q_SIGNALS:
    void showMin();
    void showMax();
    void dialogQuit();

public:
    void setSysBtnHide(bool bMinBtn = false, bool bMaxBtn = false, bool bCloseBtn = false);
    void setTitle(const QString& strTitle);

    QToolButton* getMinBtn();
    QToolButton* getMaxBtn();
    QToolButton* getCloseBtn();

protected:
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseDoubleClickEvent(QMouseEvent*);

private:
    int          iSysBtnNum;
    QPoint       pressedPoint;
    bool         isMove;
    QToolButton* btnMin;
    QToolButton* btnMax;
    QToolButton* btnClose;
    QLabel*      labTitle;
};

#endif  // CCOMMONTITELBAR_H
