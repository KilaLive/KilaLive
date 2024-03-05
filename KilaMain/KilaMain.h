#pragma once

#include <QtWidgets/QWidget>
#include "ui_KilaMain.h"

class KilaMain : public QWidget
{
    Q_OBJECT

public:
    KilaMain(QWidget *parent = nullptr);
    ~KilaMain();

private:
    Ui::KilaMainClass ui;
};
