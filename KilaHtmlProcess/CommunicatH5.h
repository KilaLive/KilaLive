#pragma once

#include <QObject>

// qt�˺�HTMLҳ��֮�佻��Ҫ�õ�����
class CommunicatH5  : public QObject
{
	Q_OBJECT

public:
    explicit CommunicatH5(QObject *parent = nullptr);
	~CommunicatH5();

public slots:
    //��HTML�˵�JavaScript�е���
    void ReceiveH5Text(const QString &text);

signals:
    void textReceived(const QString &text);
};
