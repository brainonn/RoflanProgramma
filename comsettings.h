#ifndef COMSETTINGS_H
#define COMSETTINGS_H

#include <QDialog>
#include <QSerialPort>
#include "mainwindow.h"

namespace Ui {
class COMSettings;
}

class COMSettings : public QDialog
{
    Q_OBJECT

public:
    explicit COMSettings(QWidget *parent = nullptr);
    ~COMSettings();

private:
    Ui::COMSettings *ui;
private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

};

#endif // COMSETTINGS_H
