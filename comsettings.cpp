#include "comsettings.h"
#include "ui_comsettings.h"


COMSettings::COMSettings(QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::COMSettings)
{
    ui->setupUi(this);
    //Baud rates
    ui->comboBaud->addItem(QString::number(QSerialPort::Baud1200));
    ui->comboBaud->addItem(QString::number(QSerialPort::Baud2400));
    ui->comboBaud->addItem(QString::number(QSerialPort::Baud4800));
    ui->comboBaud->addItem(QString::number(QSerialPort::Baud9600));
    ui->comboBaud->addItem(QString::number(QSerialPort::Baud19200));
    ui->comboBaud->addItem(QString::number(QSerialPort::Baud38400));
    ui->comboBaud->addItem(QString::number(QSerialPort::Baud57600));
    ui->comboBaud->addItem(QString::number(QSerialPort::Baud115200));
    ui->comboBaud->setCurrentIndex(ui->comboBaud->findText(QString::number(static_cast<MainWindow*>(this->parent())->getCOMBaudRate())));
    //Data bits
    ui->comboDataBits->addItem(QString::number(QSerialPort::Data5));
    ui->comboDataBits->addItem(QString::number(QSerialPort::Data6));
    ui->comboDataBits->addItem(QString::number(QSerialPort::Data7));
    ui->comboDataBits->addItem(QString::number(QSerialPort::Data8));
    ui->comboDataBits->setCurrentIndex(ui->comboDataBits->findText(QString::number(static_cast<MainWindow*>(this->parent())->getCOMDataBits())));
    //Parity
    QStringList parityNames = {"Even", "Odd", "No"};
    ui->comboParity->addItems(parityNames);
    QSerialPort::Parity current_parity =static_cast<MainWindow*>(this->parent())->getCOMParity();
    switch(current_parity) {
        case QSerialPort::NoParity:
            ui->comboParity->setCurrentIndex(ui->comboParity->findText("No"));
            break;
        case QSerialPort::EvenParity:
            ui->comboParity->setCurrentIndex(ui->comboParity->findText("Even"));
            break;
        case QSerialPort::OddParity:
            ui->comboParity->setCurrentIndex(ui->comboParity->findText("Odd"));
            break;
    }
}

COMSettings::~COMSettings()
{
    delete ui;
}

void COMSettings::on_buttonBox_rejected() {
    this->close();
    return;
}

void COMSettings::on_buttonBox_accepted() {
    static_cast<MainWindow*>(this->parent())->setCOMBaudRate(QSerialPort::BaudRate(this->ui->comboBaud->currentText().toInt()));
    static_cast<MainWindow*>(this->parent())->setCOMDataBits(QSerialPort::DataBits(this->ui->comboDataBits->currentText().toInt()));
    QSerialPort::Parity parity = QSerialPort::NoParity;
    switch (this->ui->comboParity->currentIndex()) {
        case 0:
            parity = QSerialPort::EvenParity;
            break;
        case 1:
            parity = QSerialPort::OddParity;
            break;
        case 2:
            parity = QSerialPort::NoParity;
            break;
    }
    static_cast<MainWindow*>(this->parent())->setCOMParity(parity);
    return;
}
