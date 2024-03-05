#ifndef CCOMMONWIDGET_H
#define CCOMMONWIDGET_H

#include <QWidget>
#include <QLayout>
#include <QLabel>
#include <QToolButton>
#include "CommonTitelBar.h"

//窗口属性枚举
enum ENUM_CommonWndFlag
{
    enum_UXWndTitlebarHint       = 0x00000001,  // 支持标题栏
    enum_UXWndMinimizeButtonHint = 0x00000002,  // 支持最小化按钮
    enum_UXWndMaximizeButtonHint = 0x00000004,  // 支持最大化按钮
    enum_UXWndCloseButtonHint    = 0x00000008,  // 有关闭按钮
    enum_UXApplication           = enum_UXWndTitlebarHint | enum_UXWndCloseButtonHint
};

Q_DECLARE_FLAGS(CommonWndFlags, ENUM_CommonWndFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(CommonWndFlags)

class CCommonWidget : public QWidget
{
    Q_OBJECT

public:
    CCommonWidget(QWidget* parent, CommonWndFlags WndFlag, Qt::WindowFlags f = 0);
    virtual ~CCommonWidget();

    //在指定区创建按钮
    void CrateSystemButton(CommonWndFlags WndFlag);
    //在指定区创建标题栏
    void CrateSystemTitle(QLayout* layout);
    //设置标题高度
    void SetTitlebarHeight(int aiHeight);
    //设置标题
    void SetTitle(const QString& strTitle);
    //设置系统任务栏标题
    void SetTaskBarTitle(const QString& strTitle);
    //设置圆角
    void SetRoundCorner(int iSize);
    //窗口居中
    void CenterWindow(QWidget* parent = NULL);
    //设置最前
    void Foreground(bool abShowFirst = true);
    //显示
    void     Show(bool abShowModal = true);
    QWidget* GetTitleBar()
    {
        return m_pTitleBar;
    }

private:
    void MakeRoundCorner();

signals:
    void OnClose();

protected slots:
    void         OnMaxClick();
    void         OnMinClick();
    virtual void SlotOnClose();

protected:
    virtual void resizeEvent(QResizeEvent* event);
    virtual void showEvent(QShowEvent* event);
    virtual void hideEvent(QHideEvent* event);

private:
    CommonWndFlags   m_lWndFlag;
    int              m_iRoundCornerSize;
    CCommonTitelBar* m_pTitleBar;  //标题栏
    bool             m_bIsShowModal = false;
    QWidget*         m_pParent      = NULL;
};

#endif
