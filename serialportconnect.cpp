#include "serialportconnect.h"

serialPortConnect::serialPortConnect(QSerialPort *serialPort ,QString portName ,QString color ,int darkRange ,int flashCount ,int timeout ,int mode ,int upperbound ,int lowerbound ,bool flashSwitch ,QObject *parent) : QObject(parent)
    , m_serialPort(serialPort)
    , m_portName(portName)
    , m_color(color)
    , m_colorDarkRange(darkRange)
    , m_flashTestCount(flashCount)
    , m_countDown(timeout)
    , m_mode(mode)
    , m_upperBound(upperbound)
    , m_lowerBound(lowerbound)
    , m_flashSwitch(flashSwitch)
    , m_standardOutput(stdout)
    , m_rx("^(s;[0-9]{1,4};[0-9]{1,4};[0-9]{1,4};e)$")
    , m_setting("Setting.ini", QSettings::IniFormat)
{

    m_serialPort->setPortName(m_portName);
    m_serialPort->setBaudRate(QSerialPort::Baud115200);
    m_serialPort->open(QIODevice::ReadWrite);

    connect(m_serialPort, &QSerialPort::readyRead, this, &serialPortConnect::handleReadyRead);
    connect(&m_timer_read, &QTimer::timeout, this, &serialPortConnect::handleReadTimeout);
    connect(&m_timer_clock, &QTimer::timeout, this, &serialPortConnect::handleClockout);


    m_timer_clock.start(1000);
    m_timer_read.start(260);//220
}

void serialPortConnect::handleReadyRead()
{
    m_readData.append(m_serialPort->readAll());
    if(!m_timer_read.isActive())
    m_timer_read.start(260);
}

void serialPortConnect::handleClockout()
{
    if(m_countDown == 0)
    {
        m_standardOutput << "TimesUp , TEST FAIL" << endl;
        return QCoreApplication::exit(1);
    }
    m_standardOutput << "CountDown:***********************************" << m_countDown--<<" Second"<<endl;
}

void serialPortConnect::handleReadTimeout()
{
    if(m_readData.isEmpty())
    {
        m_standardOutput << "-" << endl;
        //return QCoreApplication::exit(1);
    }else
    {
        m_standardOutput << "Success for read." << endl
                         << "Content : " << m_readData << endl
                         << "-----------" << endl;
        dataSeparate();

        if(m_dataList.length() - 1 < m_passTime)
        {
            m_standardOutput << "data Overflow on m_passTime" << endl;
            //return QCoreApplication::exit(1);
        }else
        {
            if(!(m_mode == 1 || m_mode == 2))
            {
                m_standardOutput << "Mode Setting error." << endl;
                return QCoreApplication::exit(1);

            }else if(m_flashSwitch)
            {
                switch(m_flashTestCount % 2)
                {
                case 1:
                    dataAyalsis();
                    break;
                case 0:
                    dataAyalsisDark();
                    break;
                }
                if(m_flashTestCount == 0)
                {
                    m_standardOutput << colorString(m_color) << " Flashing LED test success---------------------\nPASS" << endl;
                    return QCoreApplication::quit();
                }
            }else
            {
                dataAyalsis();
                if(m_flashTestCount == 0)
                {
                    m_standardOutput << colorString(m_color) <<" LED test success---------------------\nPASS" << endl;
                    return QCoreApplication::quit();
                }
            }
        }
    }
}

void serialPortConnect::dataSeparate()
{
    m_dataString = m_readData.data();
    m_readData.clear();
    m_dataList = m_dataString.split('\n');
}

void serialPortConnect::dataAyalsisDark()
{
    for(int count = 1; count<=m_passTime; count++)
    {
        m_match = m_rx.match(m_dataList[count]);

        if(!m_match.hasMatch())
        {
            m_standardOutput << " Dataform not match. Please try again." << endl;
            m_testResult = false;
            break;
        }
        QString string = m_dataList[count];
        QStringList color= string.split(';');
        QString red = color[1] ,green = color[2] ,blue = color[3];

        if((red.toInt() + green.toInt() + blue.toInt())/3 >= m_colorDarkRange)
        {
            m_standardOutput << "Dark test FAILED." << endl;
            m_testResult = false;
            break;
        }
    }
    if(m_testResult == true)
    {
        m_standardOutput << "---------------\nDarkPhase PASS" <<endl;
        m_flashTestCount--;
    }else
    {
        m_standardOutput << "---------------\nDarkPhase FAIL" <<endl;
        m_testResult = true;
    }
}


void serialPortConnect::dataAyalsis()
{
    for(int count = 1 ; count <= m_passTime; count++)
    {
        double y ,x;
        m_match = m_rx.match(m_dataList[count]);

        if(!m_match.hasMatch())
        {
            m_standardOutput << " Dataform not match. Please try again." << endl;
            m_testResult = false;
        }
        QString string = m_dataList[count];
        QStringList color= string.split(';');
        QString r = color[1] ,g = color[2] ,b = color[3];

        x = (r.toInt() + g.toInt() + b.toInt())/ 3;
        y = 0.2126 * r.toInt() + 0.7152 * g.toInt() + 0.0722 * b.toInt();
        y = qAtan2((r.toInt() - y) / 1.5748 , (b.toInt() - y) / 1.8556) * 180.0 / PI;
                if(x <= m_colorDarkRange)
                {
                    m_standardOutput << "FAILED: Not Bright enought: " << x <<endl;
                    m_testResult = false;
                    break;
                }else if( m_color == "R")
                {
                    if(y >= 105 || y <= 75 )
                    {
                        m_standardOutput << "Red color FAILED: " << y << endl;
                        m_testResult = false;
                        break;
                    }
                }else if(m_color == "RY")
                {
                    if(y <= 105 || y >= 135)
                    {
                        m_standardOutput << "Orange color FAILED: " << y  <<endl;
                        m_testResult = false;
                        break;
                    }
                }else if(m_color == "Y")
                {
                    if(y <= 135 || y >= 165)
                    {
                        m_standardOutput << "Yellow color FAILED: " << y <<endl;
                        m_testResult = false;
                        break;
                    }
                }else if(m_color == "YG")
                {
                    if(!(y >= 165 || y <= -165))
                    {
                        m_standardOutput << "Yellow Green color FAILED: " << y <<endl;
                        m_testResult = false;
                        break;
                    }
                }else if(m_color == "G")
                {
                    if(y <= -165 || y >= -135)
                    {
                        m_standardOutput << "Green color FAILED: " << y <<endl;
                        m_testResult = false;
                        break;
                    }
                }else if(m_color == "GC")
                {
                    if(y <= -135 || y >= -105)
                    {
                        m_standardOutput << "Green Cyan color FAILED: " << y <<endl;
                        m_testResult = false;
                        break;
                    }
                }else if(m_color == "C")
                {
                    if(y <= -105 || y >= -75)
                    {
                        m_standardOutput << "Cyan color FAILED: "  << y <<endl;
                        m_testResult = false;
                        break;
                    }
                }else if(m_color == "CB")
                {
                    if(y <= -75 || y >= -45)
                    {
                        m_standardOutput << "Cyan Blue color FAILED: "  << y <<endl;
                        m_testResult = false;
                        break;
                    }
                }else if(m_color == "B")
                {
                    if(y <= -45 || y >= -15)
                    {
                        m_standardOutput << "Blue color FAILED: "  << y <<endl;
                        m_testResult = false;
                        break;
                    }
                }else if(m_color == "BP")
                {
                    if(y <= -15 || y >= 15)
                    {
                        m_standardOutput << "Blue Pink color FAILED: " << y  <<endl;
                        m_testResult = false;
                        break;
                    }
                }else if(m_color == "P")
                {
                    if(y <= 15 || y >= 45)
                    {
                        m_standardOutput << "Pink color FAILED: "  << y <<endl;
                        m_testResult = false;
                        break;
                    }
                }else if(m_color == "PR")
                {
                    if(y <= 45 || y >= 75)
                    {
                        m_standardOutput << "Pink Red color FAILED: "  << y <<endl;
                        m_testResult = false;
                        break;
                    }
                }else if(m_mode == 2 && m_color == "M")
                {
                    if(!(y <= m_upperBound && y >= m_lowerBound))
                    {
                        m_standardOutput << "Custom color FAILED: "  << y <<endl;
                        m_testResult = false;
                        break;
                    }
                }

    }
    if(m_testResult == true)
    {
        if(m_flashSwitch)
        {
            m_standardOutput << "---------------\nLightPhase PASS" <<endl;            
        }else{
            m_standardOutput << "---------------\nPASS" <<endl;
        }
        m_flashTestCount--;
    }else
    {
        if(m_flashSwitch)
        {
            m_standardOutput << "---------------\nLightPhase FAIL" <<endl;            
        }else{
            m_standardOutput << "---------------\nFAIL" <<endl;
        }
        m_testResult = true;
    }

}

serialPortConnect::~serialPortConnect()
{

}

QString serialPortConnect::colorString(QString &color)
{
    if(color == "R"){
        return QString("Red");
    }else if(color == "RY"){
        return QString("Red Yellow");
    }else if(color == "Y"){
        return QString("Yellow");
    }else if(color == "YG"){
        return QString("Yellow Green");
    }else if(color == "G"){
        return QString("Green");
    }else if(color == "GC"){
        return QString("Green Cyan");
    }else if(color == "C"){
        return QString("Cyan");
    }else if(color == "CB"){
        return QString("Cyan Blue");
    }else if(color == "B"){
        return QString("Blue");
    }else if(color == "BP"){
        return QString("Blue Pink");
    }else if(color == "P"){
        return QString("Pink");
    }else if(color == "PR"){
        return QString("Pink Red");
    }else if(color == "M"){
        return QString("Custom Color");
    }
}
