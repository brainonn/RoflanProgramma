#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "comsettings.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    if(!QDir(QCoreApplication::applicationDirPath() + "/data").exists()) {
        QDir().mkdir(QCoreApplication::applicationDirPath() + "/data");
    }
    this->setWindowTitle(QCoreApplication::applicationName());

    ui->pushButtonAction->setText("Start"); // Кнопка работает в режиме "Подключить"

    ui->plot->installEventFilter(this);
    ui->derPlot->installEventFilter(this);

    ui->plot->addGraph();
    ui->derPlot->addGraph();
    ui->plot->setInteraction(QCP::iRangeDrag,true);
    ui->plot->setInteraction(QCP::iRangeZoom,true);
    ui->derPlot->setInteraction(QCP::iRangeDrag,true);
    ui->derPlot->setInteraction(QCP::iRangeZoom,true);
    ui->labelFreq->setVisible(false);
    ui->labelDer->setVisible(false);

    plotCoordinatesLabel = new QLabel(this);

    on_actionCOMUPD_triggered(); // Обновить список доступных портов

    serial = new QSerialPort();  // Создать новый объект класса "SerialPort"

    freqMax = 0;
    freqMin = 0;
    prevTime = 0;
    prevFreq = 0;
    fixedPlot = false;
    timeScale = 1000;


    connect(serial,SIGNAL(readyRead()),this,SLOT(getResponse()));
    connect(this,SIGNAL(updatePlot(double,double)),this, SLOT(onUpdatePlot(double,double)));
    connect(ui->plot, SIGNAL(mouseMove(QMouseEvent*)),this,SLOT(onMouseMove(QMouseEvent*)));
    connect(ui->derPlot, SIGNAL(mouseMove(QMouseEvent*)),this,SLOT(onMouseMove(QMouseEvent*)));
}

MainWindow::~MainWindow()
{
    if(serial->isOpen()) {
        serial->close();
    }
    delete serial; // Удалить ненужные переменные
    delete ui;
}

void MainWindow::on_actionCOMUPD_triggered()
{
    ui->comboBoxCOM->clear(); // Стереть все имевшиеся старые порты

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) { // Добавление доступных в системе портов
        ui->comboBoxCOM->addItem(info.portName());
    }

    return;
}

//CLASS METHODS
void MainWindow::getResponse()
{
    QByteArray tmp;
    tmp = serial->readAll();
    buffer += tmp;
    if (tmp.contains("\n")) {
        QList<QByteArray> res = buffer.split(' ');
        if(res.length() == 2) {
            emit(updatePlot(res[0].toDouble(),res[1].toDouble()));
        }
        buffer.clear();
    }
    return;
}

void MainWindow::setCOMBaudRate(int baud) {
    this->baud = baud;
    return;
}

void MainWindow::setCOMDataBits(QSerialPort::DataBits dataBits) {
    this->dataBits = dataBits;
    return;
}

void MainWindow::setCOMParity(QSerialPort::Parity parity) {
    this->parity = parity;
    return;
}

int MainWindow::getCOMBaudRate() {
    return this->baud;
}

QSerialPort::DataBits MainWindow::getCOMDataBits() {
    return this->dataBits;
}

QSerialPort::Parity MainWindow::getCOMParity() {
    return this->parity;
}

//SLOTS
void MainWindow::on_pushButtonAction_clicked()
{
    if (ui->pushButtonAction->text() == "Start") {
        times.clear();
        freqs.clear();
        freqMax = 0;
        freqMin = 0;
        serial->setPortName(ui->comboBoxCOM->currentText()); // Указание имени порта
        serial->setBaudRate(this->baud); // Указание частоты передачи порта
        serial->setDataBits(this->dataBits);
        serial->setParity(this->parity);
        if (!serial->open(QIODevice::ReadWrite)) { // Если попытка открыть порт для ввода\вывода не получилось
            QSerialPort::SerialPortError getError = QSerialPort::NoError; // Ошибка открытия порта
            serial->error(getError); // Получить номер ошибки
            QMessageBox::critical(this,tr("Error"), serial->errorString());
            return;
        }
        ui->pushButtonAction->setText("Stop"); // Перевести кнопку в режим "Отключение"
        ui->labelFreq->setVisible(true);
        ui->labelDer->setVisible(true);
    } else {
        this->serial->close(); // Закрыть открытый порт
        ui->pushButtonAction->setText("Start"); // Перевести кнопку в режим "Подключение"
        ui->labelFreq->setVisible(false);
        ui->labelDer->setVisible(false);

    }

    // Блокировка или разблокировка переключателей настроек порта
    ui->comboBoxCOM->setEnabled(ui->pushButtonAction->text() == "Start");
    ui->actionCOMSettings->setEnabled(ui->pushButtonAction->text() == "Start");

    return;

}

void MainWindow::on_updatePlot(double time, double freq)
{
    //reset occured
    if(time < prevTime) {
        times.clear();
        freqs.clear();
        der.clear();
        freqMin = 0;
        freqMax = 0;
        prevFreq = 0;
        prevTime = 0;
    }
    if(freqMax == 0 || freq > freqMax) freqMax = freq;
    if(freqMin == 0 || freq < freqMin) freqMin = freq;

    times.append(time / timeScale);
    freqs.append(freq);
    ui->labelFreq->setText(QString("Time: %1 s Frequency: %2 Hz").arg(int(time / timeScale)).arg(freq, 0,'g',7));
    if(freqs.size() > 1) {
        double d = timeScale * (freq - prevFreq) / (time - prevTime);
        ui->labelDer->setText(QString("Derevative: %1 Hz/s").arg(d, 0, 'g', 7));
        der.append(d);
    }
    prevTime = time;
    prevFreq = freq;
    if(!fixedPlot) {
        ui->plot->graph(0)->setData(times, freqs);
        ui->plot->xAxis->setRange(times.first(), times.last());
        ui->plot->yAxis->setRange(freqMin,freqMax);
        ui->derPlot->graph(0)->setData(times, der);
        ui->derPlot->rescaleAxes();
        ui->plot->replot();
        ui->derPlot->replot();
    }
    return;
}

void MainWindow::on_actionExport_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("Save file"),QCoreApplication::applicationDirPath() + "/data/" + QDateTime::currentDateTime().toString("yyyy_MM_dd--hh-mm-ss"),tr("Text files (*.txt)"));
    if(!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, tr("Unable to open file"),file.errorString());
        }
        QTextStream out(&file);
        out.setRealNumberNotation(QTextStream::FixedNotation);
        for(int i = 0; i < times.length(); i++) {
            if(i < 2) { // 2 points are skipped in the derevative vector
                out << times[i] << " " << freqs[i] << " " << endl;
            } else {
                out << times[i] << " " << freqs[i] << " " << der[i - 2] << endl;
            }
        }
        file.close();
    }
    return;
}

void MainWindow::on_actionFixPlot_triggered()
{
    fixedPlot = !fixedPlot;
    ui->actionFixPlot->setChecked(fixedPlot);
    if(fixedPlot) {
        fixedPlotLabel = new QLabel("Graph fixed");
        statusBar()->addPermanentWidget(fixedPlotLabel);
        fixedPlotLabel->show();
    } else {
        statusBar()->removeWidget(fixedPlotLabel);
        delete fixedPlotLabel;
    }
    return;
}

void MainWindow::on_checkBoxShowDer_stateChanged(int state) {
    switch(state) {
        case Qt::CheckState::Checked:
            ui->derPlot->setVisible(true);
            if(!der.empty()) {
                ui->labelDer->setVisible(true);
            }
            break;
        case Qt::CheckState::Unchecked:
            ui->derPlot->setVisible(false);
            ui->labelDer->setVisible(false);
            break;
    }
    return;
}

void MainWindow::on_mouseMove(QMouseEvent *event)
{
    QCustomPlot* plot = qobject_cast<QCustomPlot*>(sender());
    double time = plot->xAxis->QCPAxis::pixelToCoord(event->x());
    double freq = plot->yAxis->QCPAxis::pixelToCoord(event->y());
    QString status_str;
    if(plot == ui->plot) {
        status_str = QString("time:%1 freq:%2");
    } else {
        status_str = QString("time:%1 der:%2");
    }
    plotCoordinatesLabel->setText(status_str.arg(time).arg(freq,0,'g',7));
    return;
}

bool MainWindow::eventFilter(QObject *target, QEvent *event)
{
    if((target == ui->plot) || (target == ui->derPlot)) {
        if(event->type() == QEvent::Enter) {
            statusBar()->addWidget(plotCoordinatesLabel);
            plotCoordinatesLabel->show();
        } else if(event->type() == QEvent::Leave) {
            statusBar()->removeWidget(plotCoordinatesLabel);
        }
    }
    return false;
}

void MainWindow::on_actionCOMSettings_triggered()
{
    COMSettings *dialog = new COMSettings(this);
    dialog->show();
    return;
}
