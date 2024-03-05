#include "CommunicatH5.h"

CommunicatH5::CommunicatH5(QObject *parent)
	: QObject(parent)
{}

CommunicatH5::~CommunicatH5()
{}

void CommunicatH5::ReceiveH5Text(const QString& text)
{
    emit textReceived(text);
}


