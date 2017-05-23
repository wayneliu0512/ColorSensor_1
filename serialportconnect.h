#ifndef SERIALPORTCONNECT_H
#define SERIALPORTCONNECT_H
#include <QCoreApplication>
#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTextStream>
#include <QTimer>
#include <QSettings>
#include <QRegularExpression>
#include <QtGlobal>
#include <QtMath>
#include <QProcess>

#define PI 3.14159

class serialPortConnect : public QObject
{
    Q_OBJECT
public:

    explicit serialPortConnect(QSerialPort *serialPort ,QString portName = "COM3" ,QString color = "RED"
            ,int darkRange = 10 , int flashCount = 3,int timeout = 10 ,int mode = 2, int upperbound = 10
            ,int lowerbound = 0,bool flashSwitch = false ,QObject *parent = nullptr);

    ~serialPortConnect();

signals:

private slots:

    void handleReadyRead();
    void handleReadTimeout();
    void handleClockout();

private:

    void dataAyalsis();
    void dataAyalsisDark();
    void dataSeparate();
    QString colorString(QString &color);

    QSerialPort *m_serialPort;
    QTextStream m_standardOutput;
    QTimer      m_timer_read, m_timer_clock;
    QSettings   m_setting;

    QByteArray  m_readData;
    QString     m_dataString, m_portName, m_color;
    QStringList m_dataList;

    QRegularExpression m_rx;
    QRegularExpressionMatch m_match;

    int         m_passTime = 2 ,m_countDown ,m_mode ,m_flashTestCount = 3 ,m_colorDarkRange = 10 ,m_upperBound ,m_lowerBound;

    bool        m_testResult = true ,m_flashSwitch;

};

#endif // SERIALPORTCONNECT_H
