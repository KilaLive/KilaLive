#pragma once

#include <QObject>

// qt端和HTML页面之间交互要用到的类
class CommunicatH5  : public QObject
{
	Q_OBJECT

public:
    explicit CommunicatH5(QObject *parent = nullptr);
	~CommunicatH5();

public slots:
    //在HTML端的JavaScript中调用
    void ReceiveH5Text(const QString &text);

signals:
    void textReceived(const QString &text);
};
