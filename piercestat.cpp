#include "piercestat.h"
#include <iostream>

Command::Command(QString description, int number)
{
    this->description = description;
    this->number = number;
}

QString Command::getDescription() const
{
    return this->description;
}

int Command::getNumber() const
{
    return this->number;
}

AbstractParameter::AbstractParameter(QString description, int number, bool readOnly = false) : Command(description, number)
{
    this->readOnly = readOnly;
}

AbstractParameter::~AbstractParameter() {}

template<> void Parameter<double>::setValueFromString(QString stringValue)
{
    this->value = stringValue.toDouble();
    return;
}

template<> void Parameter<int>::setValueFromString(QString stringValue)
{
    this->value = stringValue.toInt();
    return;
}

template<> void Parameter<bool>::setValueFromString(QString stringValue)
{
    if(stringValue == "true" || stringValue == "1") {
        this->value = true;
    } else if(stringValue == "false" || stringValue == "0") {
        this->value = false;
    }
    return;
}

PierceStat::PierceStat(QObject *parent) : QObject(parent)
{
    serial = new QSerialPort();
    timeout = 100;
    //Default settings
    baud = QSerialPort::Baud115200;
    dataBits = QSerialPort::Data8;
    parity = QSerialPort::NoParity;
    //Command and parameters initialization
    //Commands
    commandsTable["Get Parameters List"] = new Command("Get Parameters List", 100);
    commandsTable["Save Flash"] = new Command("Save Flash", 110);
    //Heater Voltage
    parametersTable["Set Heater Voltage"] = new Parameter<double>("Heater Set Voltage", 150, 12.0, false);
    parametersTable["Meas. Heater Voltage"] = new Parameter<double>("Heater Measured Voltage", 202, 0.0, true);
    parametersTable["Min. Heater Voltage Alarm"] = new Parameter<double>("Min. Heater Voltage", 151, 3.9, false);
    parametersTable["Max. Heater Voltage Alarm"] = new Parameter<double>("Max. Heater Voltage", 152, 24.1, false);
    parametersTable["Heater Voltage On"] = new Parameter<bool>("Heater DC/DC converter On/Off", 153, false, false);
    parametersTable["Heater Voltage ADC Coeff."] = new Parameter<double>("ADC Coeff for Heater Voltage", 203, 0.0116, false);
    //Input Voltage
    parametersTable["Meas. Input Voltage"] = new Parameter<double>("Measured Input Voltage", 200, 0, true);
    parametersTable["Input Voltage ADC Coeff."] = new Parameter<double>("ADC Coeff for Input Voltage", 201, 0.0116, false);
    parametersTable["Min. Input Voltage Alarm"] = new Parameter<double>("Min. Input Voltage", 204, 22, false);
    parametersTable["Max. Input Voltage Alarm"] = new Parameter<double>("Max. Input Voltage", 205, 26, false);
    //Temperatures
    parametersTable["Temp1"] = new Parameter<double>("Temperature 1", 230, 0.0, true);
    parametersTable["Temp1 ADC Coeff."] = new Parameter<double>("Temperature 1 ADC Coeff.", 234, 1.0, false);
    parametersTable["Temp1 Min. Alarm"] = new Parameter<double>("Temperature 1 Min. Alarm", 238, -10.0, false);
    parametersTable["Temp1 Max. Alarm"] = new Parameter<double>("Temperature 1 Max. Alarm", 239, -10.0, false);
    parametersTable["Temp2"] = new Parameter<double>("Temperature 2", 231, 0.0, true);
    parametersTable["Temp2 ADC Coeff."] = new Parameter<double>("Temperature 2 ADC Coeff.", 235, 1.0, false);
    parametersTable["Temp2 Min. Alarm"] = new Parameter<double>("Temperature 2 Min. Alarm", 240, -10.0, false);
    parametersTable["Temp2 Max. Alarm"] = new Parameter<double>("Temperature 2 Max. Alarm", 241, -10.0, false);
    parametersTable["Temp3"] = new Parameter<double>("Temperature 3", 232, 0.0, true);
    parametersTable["Temp3 ADC Coeff."] = new Parameter<double>("Temperature 3 ADC Coeff.", 236, 1.0, false);
    parametersTable["Temp3 Min. Alarm"] = new Parameter<double>("Temperature 3 Min. Alarm", 242, -10.0, false);
    parametersTable["Temp3 Max. Alarm"] = new Parameter<double>("Temperature 3 Max. Alarm", 243, -10.0, false);
    parametersTable["Temp4"] = new Parameter<double>("Temperature 4", 233, 0.0, true);
    parametersTable["Temp4 ADC Coeff."] = new Parameter<double>("Temperature 4 ADC Coeff.", 237, 1.0, false);
    parametersTable["Temp4 Min. Alarm"] = new Parameter<double>("Temperature 4 Min. Alarm", 244, -10.0, false);
    parametersTable["Temp4 Max. Alarm"] = new Parameter<double>("Temperature 4 Max. Alarm", 245, -10.0, false);
    //Channel 1
    parametersTable["I_1A measure"] = new Parameter<double>("I_1A measure", 210, 0.0, true);
    parametersTable["I_1B measure"] = new Parameter<double>("I_1B measure", 211, 0.0, true);
    parametersTable["I_1A ADC Coeff."] = new Parameter<double>("I_1A ADC Coeff.", 214, 0.1, false);
    parametersTable["I_1B ADC Coeff."] = new Parameter<double>("I_1B ADC Coeff.", 215, 0.1, false);
    parametersTable["I_1A Max. Alarm"] = new Parameter<double>("I_1A Max. Alarm", 218, 10.0, false);
    parametersTable["I_1B Max. Alarm"] = new Parameter<double>("I_1B Max. Alarm", 219, 10.0, false);
    parametersTable["Ch1 Peltier Load"] = new Parameter<bool>("Channel 1 Peltier Load", 301, false, false); //false - resistor, true - Peltier
    parametersTable["Ch1 Const I mode"] = new Parameter<bool>("Channel 1 const I mode", 303, false, false); //false - PID, true - const I
    parametersTable["Ch1 On"] = new Parameter<bool>("Channel 1 ON", 305, false, false);
    parametersTable["Ch1 Source"] = new Parameter<int>("Channel 1 source", 307, 4, false);
    parametersTable["Ch1 P Temp"] = new Parameter<double>("Channel 1 PID P Coeff. Temp", 309, 0.0, false);
    parametersTable["Ch1 I Temp"] = new Parameter<double>("Channel 1 PID I Coeff. Temp", 311, 0.0, false);
    parametersTable["Ch1 D Temp"] = new Parameter<double>("Channel 1 PID D Coeff. Temp", 313, 0.0, false);
    parametersTable["Ch1 P Freq"] = new Parameter<double>("Channel 1 PID P Coeff. Freq", 315, 0.0, false);
    parametersTable["Ch1 I Freq"] = new Parameter<double>("Channel 1 PID I Coeff. Freq", 317, 0.0, false);
    parametersTable["Ch1 D Freq"] = new Parameter<double>("Channel 1 PID D Coeff. Freq", 319, 0.0, false);
    parametersTable["Ch1 Set Temp"] = new Parameter<double>("Channel 1 Set Temperature", 321, 0.0, false);
    parametersTable["Ch1 Set Current"] = new Parameter<double>("Channel 1 Set Current", 330, 0.0, false);
    parametersTable["gate1A On"] = new Parameter<bool>("gate1A On Off", 401, false, true);
    parametersTable["gate1B On"] = new Parameter<bool>("gate1B On Off", 402, false, true);
    parametersTable["Ch1A High Gate"] = new Parameter<bool>("Ch1A High Gate", 405, false, true);
    parametersTable["Ch1B High Gate"] = new Parameter<bool>("Ch1B High Gate", 406, false, true);
    parametersTable["Ch1 PWM DC"] = new Parameter<double>("Channel 1 PWM Duty Cycle (signed)", 409, 0.0, true);
    parametersTable["Ch1 Polarity"] = new Parameter<bool>("Channel 1 Polarity", 411, true, false);
    //Channel 2
    parametersTable["I_2A measure"] = new Parameter<double>("I_2A measure", 212, 0.0, true);
    parametersTable["I_2B measure"] = new Parameter<double>("I_2B measure", 213, 0.0, true);
    parametersTable["I_2A ADC Coeff."] = new Parameter<double>("I_2A ADC Coeff.", 216, 0.1, false);
    parametersTable["I_2B ADC Coeff."] = new Parameter<double>("I_2B ADC Coeff.", 217, 0.1, false);
    parametersTable["I_2A Max. Alarm"] = new Parameter<double>("I_2A Max. Alarm", 220, 10.0, false);
    parametersTable["I_2B Max. Alarm"] = new Parameter<double>("I_2B Max. Alarm", 221, 10.0, false);
    parametersTable["Ch2 Peltier Load"] = new Parameter<bool>("Channel 2 Peltier Load", 302, false, false); //false - resistor, true - Peltier
    parametersTable["Ch2 Const I mode"] = new Parameter<bool>("Channel 2 const I mode", 304, false, false); //false - PID, true - const I
    parametersTable["Ch2 On"] = new Parameter<bool>("Channel 2 ON", 306, false, false);
    parametersTable["Ch2 Source"] = new Parameter<int>("Channel 2 source", 308, 4, false);
    parametersTable["Ch2 P Temp"] = new Parameter<double>("Channel 2 PID P Coeff. Temp", 310, 0.0, false);
    parametersTable["Ch2 I Temp"] = new Parameter<double>("Channel 2 PID I Coeff. Temp", 312, 0.0, false);
    parametersTable["Ch2 D Temp"] = new Parameter<double>("Channel 2 PID D Coeff. Temp", 314, 0.0, false);
    parametersTable["Ch2 P Freq"] = new Parameter<double>("Channel 2 PID P Coeff. Freq", 316, 0.0, false);
    parametersTable["Ch2 I Freq"] = new Parameter<double>("Channel 2 PID I Coeff. Freq", 318, 0.0, false);
    parametersTable["Ch2 D Freq"] = new Parameter<double>("Channel 2 PID D Coeff. Freq", 320, 0.0, false);
    parametersTable["Ch2 Set Temp"] = new Parameter<double>("Channel 2 Set Temperature", 322, 0.0, false);
    parametersTable["Ch2 Set Current"] = new Parameter<double>("Channel 2 Set Current", 331, 0.0, false);
    parametersTable["gate2A On"] = new Parameter<bool>("gate2A On Off", 403, false, true);
    parametersTable["gate2B On"] = new Parameter<bool>("gate2B On Off", 404, false, true);
    parametersTable["Ch2A High Gate"] = new Parameter<bool>("Ch2A High Gate", 407, false, true);
    parametersTable["Ch2B High Gate"] = new Parameter<bool>("Ch2B High Gate", 408, false, true);
    parametersTable["Ch2 PWM DC"] = new Parameter<double>("Channel 2 PWM Duty Cycle (signed)", 410, 0.0, true);
    parametersTable["Ch2 Polarity"] = new Parameter<bool>("Channel 2 Polarity", 412, true, false);
    //Frequency
    parametersTable["Frequency"] = new Parameter<double>("Pierce frequency", 500, 0.0, false);
    parametersTable["Set Frequency"] = new Parameter<double>("Set Pierce frequency", 501, 0.0, false);
    //Alarms
    //parametersTable["Alarms"] = new Parameter<int>("Alarms vector", 700, 0, true);
    //parametersTable["Alarms Masks"] = new Parameter<int>("Alarm masks vector", 800, 0, false);

}

PierceStat::~PierceStat()
{
    if(serial->isOpen()) {
        serial->close();
    }
    delete serial;

    QHash<QString, Command*>::iterator iterCommands = commandsTable.begin();
    while (iterCommands != commandsTable.end()) {
        delete iterCommands.value();
        ++iterCommands;
    }
    QHash<QString, AbstractParameter*>::iterator iterParams = parametersTable.begin();
    while (iterParams != parametersTable.end()) {
        delete iterParams.value();
        ++iterParams;
    }
}

void PierceStat::sendCommand(QString commandName)
{
    serial->write((QString("$%1?\n").arg(commandsTable[commandName]->getNumber())).toUtf8());
    getResponse();
    return;
}

AbstractParameter* PierceStat::getParameter(QString parameterName)
{
    serial->write((QString("$%1?\n").arg(parametersTable.value(parameterName)->getNumber())).toUtf8());
    QString response = getResponse();
    parametersTable[parameterName]->setValueFromString(response.split(':')[1].trimmed());
    return parametersTable[parameterName];
}

void PierceStat::setParameter(QString parameterName, QString stringValue)
{
    serial->write((QString("$%1:%2\n").arg(parametersTable[parameterName]->getNumber()).arg(stringValue)).toUtf8());
    QString response = getResponse();
    if(response.split(':')[1].trimmed() == "Y") {
        parametersTable[parameterName]->setValueFromString(stringValue);
    }
    return;
}

AbstractParameter* PierceStat::getStoredParameter(QString parameterName)
{
    return parametersTable[parameterName];
}

QString PierceStat::getResponse()
{
    QByteArray buffer;
    while(serial->waitForReadyRead(timeout)) {
        while(serial->bytesAvailable() > 0) {
            buffer += serial->readAll();
            if(buffer.contains('\n')) {
                break;
            }
        }
    }
    return QString(buffer);
}

void PierceStat::setPortName(QString name)
{
    serial->setPortName(name);
    return;
}

QString PierceStat::getPortName() const
{
    return serial->portName();
}

void PierceStat::setBaud(int baud)
{
    this->baud = baud;
    return;
}

void PierceStat::setDataBits(QSerialPort::DataBits dataBits)
{
    this->dataBits = dataBits;
    return;
}

void PierceStat::setParity(QSerialPort::Parity parity)
{
    this->parity = parity;
    return;
}

int PierceStat::getBaud() const
{
    return serial->baudRate();
}

QSerialPort::DataBits PierceStat::getDataBits() const
{
    return serial->dataBits();
}

QSerialPort::Parity PierceStat::getParity() const
{
    return serial->parity();
}

QString PierceStat::getPortErrorString() const
{
    return serial->errorString();
}

bool PierceStat::open()
{
    if (!serial->open(QIODevice::ReadWrite)) { // Если попытка открыть порт для ввода\вывода не получилось
        return false;
    }
    serial->setBaudRate(baud);
    serial->setDataBits(dataBits);
    serial->setParity(parity);
    QHash<QString, AbstractParameter*>::iterator iter = parametersTable.begin();
    while (iter != parametersTable.end()) {
        getParameter(iter.key());
        ++iter;
    }
    return true;
}

void PierceStat::close()
{
    serial->close();
    return;
}
