#include "mainwindow.h"
#include "ui_mainwindow.h"


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
    on_actionCOMUPD_triggered(); // Обновить список доступных портов

    plots = {ui->plotFreq, ui->plotTemp, ui->plotDer};
    plotLabels = {ui->labelFreq, ui->labelDer, ui->labelTemperature};
    foreach(QCustomPlot* p, plots) {
        p->installEventFilter(this);
        p->addGraph();
        p->setInteraction(QCP::iRangeDrag,true);
        p->setInteraction(QCP::iRangeZoom,true);
    }

    foreach(QLabel* l, plotLabels) {
        l->setVisible(false);
    }

    plotCoordinatesLabel = new QLabel(this);
    pierceStat = new PierceStat(this);
    elapsedTimer = new QElapsedTimer();
    timer = new QTimer(this);
    channelSourcesValues = new QVector<QVector<double>>(5);

    freqMax = 0;
    freqMin = 0;
    prevTime = 0;
    prevFreq = 0;
    fixedPlot = false;
    pollingInterval = 1000;
    timeScale = 1000;
    channelLoads = QStringList({"Resistor", "Peltier"});
    channelModes = QStringList({"PID", "Constant Current"});
    channelSources = QStringList({"Temperature 1", "Temperature 2", "Temperature 3", "Temperature 4", "Frequency"});

    ui->comboBoxChannelsCoop->addItems({"Independent", "Cooperative"});
    ui->comboBoxChannelsCoop->setCurrentIndex(0);

    ui->comboBoxCh1Load->addItems(channelLoads);
    ui->comboBoxCh2Load->addItems(channelLoads);

    ui->comboBoxCh1Mode->addItems(channelModes);
    ui->comboBoxCh2Mode->addItems(channelModes);

    ui->comboBoxCh1Source->addItems(channelSources);
    ui->comboBoxCh2Source->addItems(channelSources);

    for(int i = 0; i < channelSources.size() - 1; i++) {
        ui->comboBoxPlotTemp->addItem(channelSources[i]);
    }
    ui->comboBoxPlotTemp->setCurrentIndex(0);



    connect(timer,SIGNAL(timeout()),this,SLOT(getReadings()));
    connect(this,SIGNAL(updatePlots(QVector<double>&)),this, SLOT(on_updatePlots(QVector<double>&)));
    connect(ui->pushButtonCh1Enable, SIGNAL(clicked()), this, SLOT(on_channelEnable()));
    connect(ui->pushButtonCh2Enable, SIGNAL(clicked()), this, SLOT(on_channelEnable()));
    //connect(ui->comboBoxCh1Load, SIGNAL(currentIndexChanged(int)), this, SLOT(on_channelLoadChanged(int)));
    //connect(ui->comboBoxCh2Load, SIGNAL(currentIndexChanged(int)), this, SLOT(on_channelLoadChanged(int)));
    //connect(ui->comboBoxCh1Mode, SIGNAL(currentIndexChanged(int)), this, SLOT(on_channelModeChanged(int)));
    //connect(ui->comboBoxCh2Mode, SIGNAL(currentIndexChanged(int)), this, SLOT(on_channelModeChanged(int)));
    //connect(ui->comboBoxCh1Source, SIGNAL(currentIndexChanged(int)), this, SLOT(on_channelSourceChanged(int)));
    //connect(ui->comboBoxCh2Source, SIGNAL(currentIndexChanged(int)), this, SLOT(on_channelSourceChanged(int)));
    //connect(ui->doubleSpinBoxCh1Setpoint, SIGNAL(valueChanged(double)), this, SLOT(on_channelSetpointChanged(double)));
    //connect(ui->doubleSpinBoxCh2Setpoint, SIGNAL(valueChanged(double)), this, SLOT(on_channelSetpointChanged(double)));
    //connect(ui->doubleSpinBoxCh1P, SIGNAL(valueChanged(double)), this, SLOT(on_channelPChanged(double)));
    //connect(ui->doubleSpinBoxCh2P, SIGNAL(valueChanged(double)), this, SLOT(on_channelPChanged(double)));
    //connect(ui->doubleSpinBoxCh1I, SIGNAL(valueChanged(double)), this, SLOT(on_channelIChanged(double)));
    //connect(ui->doubleSpinBoxCh2I, SIGNAL(valueChanged(double)), this, SLOT(on_channelIChanged(double)));
    //connect(ui->doubleSpinBoxCh1D, SIGNAL(valueChanged(double)), this, SLOT(on_channelDChanged(double)));
    //connect(ui->doubleSpinBoxCh2D, SIGNAL(valueChanged(double)), this, SLOT(on_channelDChanged(double)));
    foreach(QCustomPlot* p, plots) {
        connect(p, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(on_mouseMove(QMouseEvent*)));
    }

}

MainWindow::~MainWindow()
{
    delete elapsedTimer;
    delete channelSourcesValues;
    delete ui;
}

//CLASS METHODS

void MainWindow::getReadings()
{
    QVector<double> res;
    for(int i = 0; i < 4; i++) {
        res.append(static_cast<Parameter<double>*>(pierceStat->getParameter(QString("Temp%1").arg(i + 1)))->getValue());
    }
    res.append(static_cast<Parameter<double>*>(pierceStat->getParameter("Frequency"))->getValue());

    emit updatePlots(res);

    //QVector<double> meas;

    //meas.append(static_cast<Parameter<double>*>(pierceStat->getParameter("Meas. Heater Voltage"))->getValue());
    //emit updateMeasurements(meas);

    return;
}

void MainWindow::setCOMBaudRate(int baud) {
    pierceStat->setBaud(baud);
    return;
}

void MainWindow::setCOMDataBits(QSerialPort::DataBits dataBits) {
    pierceStat->setDataBits(dataBits);
    return;
}

void MainWindow::setCOMParity(QSerialPort::Parity parity) {
    pierceStat->setParity(parity);
    return;
}

int MainWindow::getCOMBaudRate() const {
    return pierceStat->getBaud();
}

QSerialPort::DataBits MainWindow::getCOMDataBits() const {
    return pierceStat->getDataBits();
}

QSerialPort::Parity MainWindow::getCOMParity() const {
    return pierceStat->getParity();
}

//SLOTS
void MainWindow::on_pushButtonAction_clicked()
{
    elapsedTimer->start();
    if (ui->pushButtonAction->text() == "Start") {
        foreach(QVector<double> v, *channelSourcesValues) {
            v.clear();
        }
        freqDer.clear();
        freqMax = 0;
        freqMin = 0;
        pierceStat->setPortName(ui->comboBoxCOM->currentText()); // Указание имени порта
        if (!pierceStat->open()) { // Если попытка открыть порт для ввода\вывода не получилось
            QMessageBox::critical(this,tr("Error"), pierceStat->getPortErrorString());
            return;
        }
        timer->start(pollingInterval);
        ui->pushButtonAction->setText("Stop"); // Перевести кнопку в режим "Отключение"
        foreach(QLabel* l, plotLabels) {
            l->setVisible(true);
        }
        QSignalBlocker(ui->pushButtonCh1Enable);
        if(static_cast<Parameter<bool>*>(pierceStat->getParameter("Ch1 On"))->getValue()) {
            ui->pushButtonCh1Enable->setText("Disable");
        } else {
            ui->pushButtonCh1Enable->setText("Enable");
        }
        QSignalBlocker(ui->pushButtonCh2Enable);
        if(static_cast<Parameter<bool>*>(pierceStat->getParameter("Ch2 On"))->getValue()) {
            ui->pushButtonCh2Enable->setText("Disable");
        } else {
            ui->pushButtonCh2Enable->setText("Enable");
        }
        QSignalBlocker(ui->comboBoxCh1Load);
        if(static_cast<Parameter<bool>*>(pierceStat->getStoredParameter("Ch1 Peltier Load"))->getValue()) {
            ui->comboBoxCh1Load->setCurrentText("Peltier");
        } else {
            ui->comboBoxCh1Load->setCurrentText("Resistor");
        }
        QSignalBlocker(ui->comboBoxCh2Load);
        if(static_cast<Parameter<bool>*>(pierceStat->getStoredParameter("Ch2 Peltier Load"))->getValue()) {
            ui->comboBoxCh2Load->setCurrentText("Peltier");
        } else {
            ui->comboBoxCh2Load->setCurrentText("Resistor");
        }
        QSignalBlocker(ui->comboBoxCh1Mode);
        if(static_cast<Parameter<bool>*>(pierceStat->getStoredParameter("Ch1 Const I mode"))->getValue()) {
            ui->comboBoxCh1Mode->setCurrentText("Constant Current");
        } else {
            ui->comboBoxCh1Mode->setCurrentText("PID");
        }
        QSignalBlocker(ui->comboBoxCh2Mode);
        if(static_cast<Parameter<bool>*>(pierceStat->getStoredParameter("Ch2 Const I mode"))->getValue()) {
            ui->comboBoxCh2Mode->setCurrentText("Constant Current");
        } else {
            ui->comboBoxCh2Mode->setCurrentText("PID");
        }
        QSignalBlocker(ui->comboBoxCh1Source);
        ui->comboBoxCh1Source->setCurrentIndex(static_cast<Parameter<int>*>(pierceStat->getStoredParameter("Ch1 Source"))->getValue());
        QSignalBlocker(ui->comboBoxCh2Source);
        ui->comboBoxCh2Source->setCurrentIndex(static_cast<Parameter<int>*>(pierceStat->getStoredParameter("Ch2 Source"))->getValue());

        QSignalBlocker(ui->doubleSpinBoxCh1Setpoint);
        QSignalBlocker(ui->doubleSpinBoxCh1P);
        QSignalBlocker(ui->doubleSpinBoxCh1I);
        QSignalBlocker(ui->doubleSpinBoxCh1D);
        if(channelSources[ui->comboBoxCh1Source->currentIndex()] == "Frequency") {
            ui->doubleSpinBoxCh1Setpoint->setValue(static_cast<Parameter<double>*>(pierceStat->getStoredParameter("Set Frequency"))->getValue());
            ui->doubleSpinBoxCh1P->setValue(static_cast<Parameter<double>*>(pierceStat->getStoredParameter("Ch1 P Freq"))->getValue());
            ui->doubleSpinBoxCh1I->setValue(static_cast<Parameter<double>*>(pierceStat->getStoredParameter("Ch1 I Freq"))->getValue());
            ui->doubleSpinBoxCh1D->setValue(static_cast<Parameter<double>*>(pierceStat->getStoredParameter("Ch1 D Freq"))->getValue());
        } else {
            ui->doubleSpinBoxCh1Setpoint->setValue(static_cast<Parameter<double>*>(pierceStat->getStoredParameter("Ch1 Set Temp"))->getValue());
            ui->doubleSpinBoxCh1P->setValue(static_cast<Parameter<double>*>(pierceStat->getStoredParameter("Ch1 P Temp"))->getValue());
            ui->doubleSpinBoxCh1I->setValue(static_cast<Parameter<double>*>(pierceStat->getStoredParameter("Ch1 I Temp"))->getValue());
            ui->doubleSpinBoxCh1D->setValue(static_cast<Parameter<double>*>(pierceStat->getStoredParameter("Ch1 D Temp"))->getValue());
        }
        QSignalBlocker(ui->doubleSpinBoxCh2Setpoint);
        QSignalBlocker(ui->doubleSpinBoxCh2P);
        QSignalBlocker(ui->doubleSpinBoxCh2I);
        QSignalBlocker(ui->doubleSpinBoxCh2D);
        if(channelSources[ui->comboBoxCh1Source->currentIndex()] == "Frequency") {
            ui->doubleSpinBoxCh2Setpoint->setValue(static_cast<Parameter<double>*>(pierceStat->getStoredParameter("Set Frequency"))->getValue());
            ui->doubleSpinBoxCh2P->setValue(static_cast<Parameter<double>*>(pierceStat->getStoredParameter("Ch2 P Freq"))->getValue());
            ui->doubleSpinBoxCh2I->setValue(static_cast<Parameter<double>*>(pierceStat->getStoredParameter("Ch2 I Freq"))->getValue());
            ui->doubleSpinBoxCh2D->setValue(static_cast<Parameter<double>*>(pierceStat->getStoredParameter("Ch2 D Freq"))->getValue());
        } else {
            ui->doubleSpinBoxCh2Setpoint->setValue(static_cast<Parameter<double>*>(pierceStat->getStoredParameter("Ch2 Set Temp"))->getValue());
            ui->doubleSpinBoxCh2P->setValue(static_cast<Parameter<double>*>(pierceStat->getStoredParameter("Ch2 P Temp"))->getValue());
            ui->doubleSpinBoxCh2I->setValue(static_cast<Parameter<double>*>(pierceStat->getStoredParameter("Ch2 I Temp"))->getValue());
            ui->doubleSpinBoxCh2D->setValue(static_cast<Parameter<double>*>(pierceStat->getStoredParameter("Ch2 D Temp"))->getValue());
        }

    } else {
        timer->stop();
        foreach(QVector<double> v, *channelSourcesValues) {
            v.clear();
        }
        pierceStat->close(); // Закрыть открытый порт
        ui->pushButtonAction->setText("Start"); // Перевести кнопку в режим "Подключение"
        foreach(QLabel* l, plotLabels) {
            l->setVisible(false);
        }
    }
    // Блокировка или разблокировка переключателей настроек порта
    ui->comboBoxCOM->setEnabled(ui->pushButtonAction->text() == "Start");
    ui->actionCOMSettings->setEnabled(ui->pushButtonAction->text() == "Start");
    ui->groupBoxCh1->setEnabled(ui->pushButtonAction->text() == "Stop");
    ui->groupBoxCh2->setEnabled(ui->pushButtonAction->text() == "Stop");

    return;

}

void MainWindow::on_channelEnable()
{
    QString name = sender() == ui->pushButtonCh1Enable ? "Ch1 On" : "Ch2 On";
    if(qobject_cast<QPushButton*>(sender())->text() == "Enable") {
        pierceStat->setParameter(name, "true");
        qobject_cast<QPushButton*>(sender())->setText("Disable");
    } else if(qobject_cast<QPushButton*>(sender())->text() == "Disable") {
        pierceStat->setParameter(name, "false");
        qobject_cast<QPushButton*>(sender())->setText("Enable");
    }
    return;
}

void MainWindow::on_channelLoadChanged(int index)
{
    QString paramName = sender() == ui->comboBoxCh1Load ? "Ch1 Peltier Load" : "Ch2 Peltier Load";
    pierceStat->setParameter(paramName, channelLoads[index] == "Peltier" ? "true" : "false");
    return;
}

void MainWindow::on_channelModeChanged(int index)
{
    QString paramName = sender() == ui->comboBoxCh1Mode ? "Ch1 Const I mode" : "Ch2 Const I mode";
    bool isPID = channelModes[index] == "PID";
    pierceStat->setParameter(paramName, isPID ? "false" : "true");
    QLabel* labelP = sender() == ui->comboBoxCh1Mode ? ui->labelCh1P : ui->labelCh2P;
    labelP->setVisible(isPID);
    QLabel* labelI = sender() == ui->comboBoxCh1Mode ? ui->labelCh1I : ui->labelCh2I;
    labelI->setVisible(isPID);
    QLabel* labelD = sender() == ui->comboBoxCh1Mode ? ui->labelCh1D : ui->labelCh2D;
    labelD->setVisible(isPID);
    QLabel* labelSetpoint = sender() == ui->comboBoxCh1Mode ? ui->labelCh1Setpoint : ui->labelCh2Setpoint;
    labelSetpoint->setText(isPID ? "Setpoint" : "Set Current");
    QDoubleSpinBox* doubleSpinBoxP = sender() == ui->comboBoxCh1Mode ? ui->doubleSpinBoxCh1P : ui->doubleSpinBoxCh2P;
    doubleSpinBoxP->setVisible(isPID);
    QDoubleSpinBox* doubleSpinBoxI = sender() == ui->comboBoxCh1Mode ? ui->doubleSpinBoxCh1I : ui->doubleSpinBoxCh2I;
    doubleSpinBoxI->setVisible(isPID);
    QDoubleSpinBox* doubleSpinBoxD = sender() == ui->comboBoxCh1Mode ? ui->doubleSpinBoxCh1D : ui->doubleSpinBoxCh2D;
    doubleSpinBoxD->setVisible(isPID);
    QComboBox* comboBoxSource = sender() == ui->comboBoxCh1Mode ? ui->comboBoxCh1Source : ui->comboBoxCh2Source;
    comboBoxSource->setVisible(isPID);
    return;
}

void MainWindow::on_channelSourceChanged(int index)
{
    QString channelName = sender() == ui->comboBoxCh1Mode ? "Ch1" : "Ch2";
    QString paramName = QString("%1 Source").arg(channelName);
    bool isFreq = channelSources[index] == "Frequency";
    pierceStat->setParameter(paramName, QString::number(index));
    QDoubleSpinBox* doubleSpinBoxP = channelName == "Ch1" ? ui->doubleSpinBoxCh1P : ui->doubleSpinBoxCh2P;
    const QSignalBlocker blockerP = QSignalBlocker(doubleSpinBoxP);
    QString getPParamName = QString("%1 P %2").arg(channelName).arg(isFreq ? "Freq" : "Temp");
    doubleSpinBoxP->setValue(static_cast<Parameter<double>*>(pierceStat->getParameter(getPParamName))->getValue());
    QDoubleSpinBox* doubleSpinBoxI = channelName == "Ch1" ? ui->doubleSpinBoxCh1I : ui->doubleSpinBoxCh2I;
    const QSignalBlocker blockerI = QSignalBlocker(doubleSpinBoxI);
    QString getIParamName = QString("%1 I %2").arg(channelName).arg(isFreq ? "Freq" : "Temp");
    doubleSpinBoxI->setValue(static_cast<Parameter<double>*>(pierceStat->getParameter(getIParamName))->getValue());
    QDoubleSpinBox* doubleSpinBoxD = channelName == "Ch1" ? ui->doubleSpinBoxCh1D : ui->doubleSpinBoxCh2D;
    const QSignalBlocker blockerD = QSignalBlocker(doubleSpinBoxD);
    QString getDParamName = QString("%1 D %2").arg(channelName).arg(isFreq ? "Freq" : "Temp");
    doubleSpinBoxD->setValue(static_cast<Parameter<double>*>(pierceStat->getParameter(getDParamName))->getValue());
    QDoubleSpinBox* doubleSpinBoxSetpoint = channelName == "Ch1" ? ui->doubleSpinBoxCh1Setpoint : ui->doubleSpinBoxCh2Setpoint;
    const QSignalBlocker blockerSetpoint = QSignalBlocker(doubleSpinBoxSetpoint);
    QString getSetpointParamName;
    if(isFreq) {
        getSetpointParamName = "Set Frequency";
    } else {
        getSetpointParamName = QString("%1 Set Temp").arg(channelName);
    }
    doubleSpinBoxSetpoint->setValue(static_cast<Parameter<double>*>(pierceStat->getParameter(getSetpointParamName))->getValue());
    return;
}

void MainWindow::on_channelSetpointChanged(double value)
{
    QString channelName = sender() == ui->doubleSpinBoxCh1Setpoint ? "Ch1" : "Ch2";
    QComboBox* comboBoxMode = channelName == "Ch1" ? ui->comboBoxCh1Mode : ui->comboBoxCh2Mode;
    bool isConstantCurrent = channelModes[comboBoxMode->currentIndex()] == "Constant Current";
    QString paramName;
    if(isConstantCurrent) {
        paramName = QString("%1 Set Current").arg(channelName);
    } else {
        QComboBox* comboBoxSource = channelName == "Ch1" ? ui->comboBoxCh1Source : ui->comboBoxCh2Source;
        bool isFreq = channelSources[comboBoxSource->currentIndex()] == "Frequency";
        if(isFreq) {
            paramName = "Set Frequency";
        } else {
            paramName = QString("%1 Set Temp").arg(channelName);
        }
    }
    pierceStat->setParameter(paramName, QString::number(value));
    return;
}

void MainWindow::on_channelPChanged(double value)
{
    QString channelName = sender() == ui->doubleSpinBoxCh1P ? "Ch1" : "Ch2";
    QComboBox* comboBoxSource = channelName == "Ch1" ? ui->comboBoxCh1Source : ui->comboBoxCh2Source;
    bool isFreq = channelSources[comboBoxSource->currentIndex()] == "Frequency";
    QString paramName = QString("%1 P %2").arg(channelName).arg(isFreq ? "Freq" : "Temp");
    pierceStat->setParameter(paramName, QString::number(value));
    return;
}

void MainWindow::on_channelIChanged(double value)
{
    QString channelName = sender() == ui->doubleSpinBoxCh1I ? "Ch1" : "Ch2";
    QComboBox* comboBoxSource = channelName == "Ch1" ? ui->comboBoxCh1Source : ui->comboBoxCh2Source;
    bool isFreq = channelSources[comboBoxSource->currentIndex()] == "Frequency";
    QString paramName = QString("%1 I %2").arg(channelName).arg(isFreq ? "Freq" : "Temp");
    pierceStat->setParameter(paramName, QString::number(value));
    return;
}

void MainWindow::on_channelDChanged(double value)
{
    QString channelName = sender() == ui->doubleSpinBoxCh1D ? "Ch1" : "Ch2";
    QComboBox* comboBoxSource = channelName == "Ch1" ? ui->comboBoxCh1Source : ui->comboBoxCh2Source;
    bool isFreq = channelSources[comboBoxSource->currentIndex()] == "Frequency";
    QString paramName = QString("%1 D %2").arg(channelName).arg(isFreq ? "Freq" : "Temp");
    pierceStat->setParameter(paramName, QString::number(value));
    return;
}

void MainWindow::on_updatePlots(QVector<double>& values)
{
    //values[0 - 3] - temperatures, values[4] - frequency
    int time = elapsedTimer->elapsed();
    //reset occured
    if(time < prevTime) {
        times.clear();
        foreach(QVector<double> v, *channelSourcesValues) {
            v.clear();
        }
        freqDer.clear();
        freqMin = 0;
        freqMax = 0;
        prevFreq = 0;
        prevTime = 0;
    }
    //for nice plotting of frequency
    if(freqMax == 0 || values[4] > freqMax) freqMax = values[4];
    if(freqMin == 0 || values[4] < freqMin) freqMin = values[4];

    times.append(time / timeScale);
    for(int i = 0; i < 5; i++) {
        (*channelSourcesValues)[i].append(values[i]);
    }
    ui->labelFreq->setText(QString("Time: %1 s Frequency: %2 Hz").arg(int(time / timeScale)).arg(values[4], 0,'g',7));
    ui->labelTemperature->setText(QString("Temp1: %1 Temp2: %2 Temp3: %3 Temp4: %4").arg(values[0], 0,'g',7).arg(values[1], 0,'g',7).arg(values[2], 0,'g',7).arg(values[3], 0,'g',7));
    //if we have more than 1 point for frequency
    if((*channelSourcesValues)[4].size() > 1) {
        double d = timeScale * (values[4] - prevFreq) / (time - prevTime);
        ui->labelDer->setText(QString("Derevative: %1 Hz/s").arg(d, 0, 'g', 7));
        freqDer.append(d);
    }
    prevTime = time;
    prevFreq = values[4];

    if(!fixedPlot) {
        ui->plotFreq->graph(0)->setData(times, (*channelSourcesValues)[4]);
        ui->plotDer->graph(0)->setData(times, freqDer);
        ui->plotTemp->graph(0)->setData(times, (*channelSourcesValues)[ui->comboBoxPlotTemp->currentIndex()]);
        ui->plotFreq->xAxis->setRange(times.first(), times.last());
        ui->plotFreq->yAxis->setRange(freqMin, freqMax);
        for(int i = 0; i < plots.size(); i++) {
            plots[i]->rescaleAxes();
        }
        foreach(QCustomPlot* p, plots) {
            p->replot();
        }
    }
    return;
}

void MainWindow::on_updateMeasurements(QVector<double>& meas)
{
    ui->lcdHeaterVoltage->display(meas[0]);
    return;
}

void MainWindow::on_checkBoxShowDer_stateChanged(int state) {
    switch(state) {
        case Qt::CheckState::Checked:
            ui->plotDer->setVisible(true);
            if(!freqDer.empty()) {
                ui->labelDer->setVisible(true);
            }
            break;
        case Qt::CheckState::Unchecked:
            ui->plotDer->setVisible(false);
            ui->labelDer->setVisible(false);
            break;
    }
    return;
}

void MainWindow::on_checkBoxPlotTemperature_stateChanged(int state) {
    switch(state) {
        case Qt::CheckState::Checked:
            ui->plotTemp->setVisible(true);
            //DANGEROUS
            if(!channelSourcesValues[ui->comboBoxPlotTemp->currentIndex()].empty()) {
                ui->labelDer->setVisible(true);
            }
            break;
        case Qt::CheckState::Unchecked:
            ui->plotTemp->setVisible(false);
            ui->labelTemperature->setVisible(false);
            break;
    }
    return;
}

void MainWindow::on_mouseMove(QMouseEvent *event)
{
    QCustomPlot* plot = qobject_cast<QCustomPlot*>(sender());
    double time = plot->xAxis->QCPAxis::pixelToCoord(event->x());
    double value = plot->yAxis->QCPAxis::pixelToCoord(event->y());
    QString status_str;
    if(plot == ui->plotFreq) {
        status_str = QString("time:%1 freq:%2");
    } else if(plot == ui->plotDer) {
        status_str = QString("time:%1 der:%2");
    } else if(plot == ui->plotTemp) {
        status_str = QString("time:%1 temp:%2");
    }
    plotCoordinatesLabel->setText(status_str.arg(time).arg(value,0,'g',7));
    return;
}

bool MainWindow::eventFilter(QObject *target, QEvent *event)
{
    bool condition = false;
    foreach(QCustomPlot* p, plots) {
        condition = condition || (target == p);
        if(condition) {
            break;
        }
    }
    if(condition) {
        if(event->type() == QEvent::Enter) {
            statusBar()->addWidget(plotCoordinatesLabel);
            plotCoordinatesLabel->show();
        } else if(event->type() == QEvent::Leave) {
            statusBar()->removeWidget(plotCoordinatesLabel);
        }
    }
    return false;
}

void MainWindow::on_actionCOMUPD_triggered()
{
    ui->comboBoxCOM->clear(); // Стереть все имевшиеся старые порты

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) { // Добавление доступных в системе портов
        ui->comboBoxCOM->addItem(info.portName());
    }

    return;
}

void MainWindow::on_actionCOMSettings_triggered()
{
    COMSettings *dialog = new COMSettings(this);
    dialog->show();
    return;
}

void MainWindow::on_actionPierceStatSettings_triggered()
{
    PierceStatSettings *dialog = new PierceStatSettings(this);
    dialog->show();
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
            out << times[i] << ",";
            foreach(const QVector<double>& v, *channelSourcesValues) {
                out << v[i] << ",";
            }
            if(i > 1) { // 1 points is skipped in the derevative vector
                out << freqDer[i];
            }
            out << endl;
        }
        file.close();
    }
    return;
}

