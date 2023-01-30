#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTime>
#include <QDebug>
#include <QLabel>


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    QByteArray buffer;
    QSerialPort *serial; // SerialPort
    int baud = QSerialPort::Baud115200;
    QSerialPort::DataBits dataBits = QSerialPort::Data8;
    QSerialPort::Parity parity = QSerialPort::NoParity;
    QLabel *fixedPlotLabel;
    QLabel *plotCoordinatesLabel;
    QVector<double> times;
    QVector<double> freqs;
    QVector<double> der;
    double freqMax;
    double freqMin;
    double prevTime;
    double prevFreq;
    int timeScale;
    bool fixedPlot;
protected:
    bool eventFilter(QObject *target, QEvent *event);
public:
    explicit MainWindow(QWidget *parent = nullptr); // Конструктор класса
    ~MainWindow(); // Деструктор класса
    void setCOMBaudRate(int);
    void setCOMDataBits(QSerialPort::DataBits);
    void setCOMParity(QSerialPort::Parity);
    int getCOMBaudRate();
    QSerialPort::DataBits getCOMDataBits();
    QSerialPort::Parity getCOMParity();
private slots:
    void on_actionCOMUPD_triggered(); // Обновление списка портов
    void on_actionExport_triggered();
    void on_actionFixPlot_triggered();
    void on_actionCOMSettings_triggered();
    void getResponse();
    void on_pushButtonAction_clicked(); // Подключение и отключение
    void on_updatePlot(double, double);
    void on_mouseMove(QMouseEvent*);
    void on_checkBoxShowDer_stateChanged(int);

signals:
    void updatePlot(double, double) const;

private:
    Ui::MainWindow *ui; // Пользовательский интерфейс
};

#endif // MAINWINDOW_H
