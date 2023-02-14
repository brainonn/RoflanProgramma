#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QLabel>
#include <QElapsedTimer>
#include <QTimer>
#include <QSignalBlocker>
#include "qcustomplot.h"
#include "comsettings.h"
#include "piercestat.h"
#include "piercestatsettings.h"


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

protected:
    bool eventFilter(QObject *target, QEvent *event);
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setCOMBaudRate(int);
    void setCOMDataBits(QSerialPort::DataBits);
    void setCOMParity(QSerialPort::Parity);
    void setCOMPortName(QString);
    int getCOMBaudRate() const;
    QSerialPort::DataBits getCOMDataBits() const;
    QSerialPort::Parity getCOMParity() const;
private slots:
    void on_actionCOMUPD_triggered(); // Обновление списка портов
    void on_actionExport_triggered();
    void on_actionFixPlot_triggered();
    void on_actionCOMSettings_triggered();
    void on_actionPierceStatSettings_triggered();
    void on_pushButtonAction_clicked(); // Подключение и отключение
    void on_channelEnable();
    void on_channelLoadChanged(int);
    void on_channelModeChanged(int);
    void on_channelSourceChanged(int);
    void on_channelSetpointChanged(double);
    void on_channelPChanged(double);
    void on_channelIChanged(double);
    void on_channelDChanged(double);
    void on_updatePlots(QVector<double>&);
    void on_mouseMove(QMouseEvent*);
    void on_checkBoxShowDer_stateChanged(int);
    void on_checkBoxPlotTemperature_stateChanged(int);
    void getReadings();
    void on_updateMeasurements(QVector<double>&);

signals:
    void updatePlots(QVector<double>&) const;
    void updateMeasurements(QVector<double>&) const;

private:
    Ui::MainWindow *ui; // Пользовательский интерфейс
    QElapsedTimer *elapsedTimer;
    QTimer *timer;
    QLabel *fixedPlotLabel;
    QLabel *plotCoordinatesLabel;
    PierceStat *pierceStat;
    QStringList channelModes;
    QStringList channelLoads;
    QStringList channelSources;
    QVector<QCustomPlot*> plots;
    QVector<QLabel*> plotLabels;
    QVector<QVector<double>>* channelSourcesValues; //0 - 3 temps, 4 - freq
    QVector<double> times;
    QVector<double> freqDer;



    double freqMax;
    double freqMin;
    double prevTime;
    double prevFreq;
    int timeScale;
    bool fixedPlot;
    int pollingInterval;
};

#endif // MAINWINDOW_H
