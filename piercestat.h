#ifndef PIERCESTAT_H
#define PIERCESTAT_H

#include <QObject>
#include <QSerialPort>
#include <QHash>

class Command {
private:
    int number;
    QString description;
public:
    Command(QString, int);
    int getNumber() const;
    QString getDescription() const;
};

class AbstractParameter : public Command {
private:
    bool readOnly;
public:
    AbstractParameter(QString, int, bool);
    virtual ~AbstractParameter();
    virtual void setValueFromString(QString) = 0;
};


template <typename T>
class Parameter : public AbstractParameter {
private:
    T value;
public:
    Parameter(QString description, int number, T value, bool readOnly = false) : AbstractParameter(description, number, readOnly)
    {
        this->value = value;
    }
    void setValue(T value)
    {
        this->value = value;
        return;
    }
    T getValue() const
    {
        return this->value;
    }
    bool isReadOnly() const
    {
        return this->readOnly;
    }
    void setValueFromString(QString stringValue);

};

class PierceStat : public QObject
{
    Q_OBJECT
private:
    QSerialPort *serial;
    int baud;
    QSerialPort::DataBits dataBits;
    QSerialPort::Parity parity;
    QByteArray receiveBuffer;
    QHash<QString, AbstractParameter*> parametersTable;
    QHash<QString, Command*> commandsTable;
    int timeout;
public:
    explicit PierceStat(QObject *parent = nullptr);
    ~PierceStat();
    void setPortName(QString);
    void setBaud(int);
    void setDataBits(QSerialPort::DataBits);
    void setParity(QSerialPort::Parity);
    int getBaud() const;
    QSerialPort::DataBits getDataBits() const;
    QSerialPort::Parity getParity() const;
    QString getPortName() const;
    bool open();
    void close();
    QString getPortErrorString() const;
    void sendCommand(QString);
    void setParameter(QString, QString);
    AbstractParameter* getParameter(QString);
    AbstractParameter* getStoredParameter(QString);
    QString getResponse();

signals:

};

#endif // PIERCESTAT_H
