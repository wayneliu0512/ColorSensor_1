#include "serialportconnect.h"
#include <QDebug>
#include <Windows.h>
#include <QDir>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTextStream standardOutput(stdout);
    QSerialPort serialPort;
    QString portName ,color ,upperbound ,lowerbound ,colorRanged, deviceName;
    int countDown ,mode = 2 ,colorDarkRange ,flashCount;
    bool flashSwitch = false;
    QSettings setting("Setting.ini", QSettings::IniFormat);

    if(a.arguments().size() < 2 || a.arguments().size() > 5)
    {
        standardOutput << "Invaild arguments avalible for read." << endl;
        standardOutput<<"====================================================="<<endl<<endl
                      <<"Options:"<<endl<<endl
                      <<"   Standard Mode: Test color which you typed."<<endl
                      <<"   Example:       \"/C\" + <Color> + <FlashSwitch>"<<endl
                      <<"   --------------------------------------------------"<<endl
                      <<"   Manual Mode:   Test the color range what you defined manually."<<endl
                      <<"   Example:       \"/M\" + <UpperBound> + <LowerBound> + <FlashSwitch>"<<endl
                      <<"   --------------------------------------------------"<<endl
                      <<"   information:   \"/?\""<<endl<<endl
                      <<"Commands:"<<endl<<endl
                      <<"   <Color>:       \"R\"(red), \"RY\"(red yellow/orange), \"Y\"(yellow),"<<endl
                      <<"                  \"YG\"(yellow green), \"G\"(green), \"GC\"(green cyan),"<<endl
                      <<"                  \"C\"(cyan), \"CB\"(cyan blue), \"B\"(Blue), \"BP\"(blue pink),"<<endl
                      <<"                  \"P\"(pink), \"PR\"(pink red)."<<endl
                      <<"   ---------------------------------------------------"<<endl
                      <<"   <UpperBound> & <LowerBound>:    \"180\" to \"-180\"."<<endl
                      <<"   ---------------------------------------------------"<<endl
                      <<"   <FlashSwitch>:       If test on flashing light , type \"F\" , if not , you can"<<endl
                      <<"                        just ignore this command."<<endl<<endl
                      <<"====================================================="<<endl;
        return 1;
    }

    QProcess::execute("cmd.exe /c pnputil.exe -i -a " + QDir::toNativeSeparators(QDir::currentPath())
                      + "\\CDM-2\\CDMv2.08.28Certified\\*.inf");

    deviceName = setting.value("Device/Name").toString();

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        if(info.description().contains(deviceName, Qt::CaseInsensitive))
        {
            portName = info.portName();
            standardOutput << "Find PortName: " <<portName << endl;
        }
    }

    if(portName.isEmpty())
    {
        standardOutput << "Can't found Arduino port." << endl;
        return 1;
    }

    QString argument1 = a.arguments().at(1);
    argument1 = argument1.toUpper();

    QString argument3 = a.arguments().last();
    argument3 = argument3.toUpper();

    if(argument1 == "/C")
    {
        if(argument3 == "F")
        {
            if(a.arguments().size() != 4)
            {
                standardOutput << "No Color arguments to read." << endl;
                return 1;
            }
            flashSwitch = true;
        }else
        {
            if(a.arguments().size() != 3)
            {
                standardOutput << "No Color arguments to read." << endl;
                return 1;
            }
        }
        QString argument2 = a.arguments().at(2);
        argument2 = argument2.toUpper();
        if(!(argument2 == "R" || argument2 == "RY" || argument2 == "Y" || argument2 == "YG"
           || argument2 == "G" || argument2 == "GC" || argument2 == "C"|| argument2 == "CB"
           || argument2 == "B"|| argument2 == "BP"|| argument2 == "P"|| argument2 == "PR"))
        {
            standardOutput << "Invaild Color arguments avalible for read." << endl;
            return 1;
        }
        color = argument2;
        mode = 1;
    }else if(argument1 == "/M")
    {
        if(argument3 == "F")
        {
            if(a.arguments().size() != 5)
            {
                standardOutput << "No Color arguments to read." << endl;
                return 1;
            }
            flashSwitch = true;
        }else
        {
            if(a.arguments().size() != 4)
            {
                standardOutput << "No Color range arguments to read." << endl;
                return 1;
            }
        }
        QString argument2 = a.arguments().at(2);
        argument2 = argument2.toUpper();
        upperbound = argument2;
        lowerbound = argument3;
        if(upperbound.toInt() < lowerbound.toInt())
        {
            standardOutput << "number1 must bigger than number2." << endl;
            return 1;
        }
        mode = 2;
    }else if(argument1 == "/?")
    {
        if(a.arguments().size() != 2)
        {
            standardOutput << "Invaild arguments avalible for read." << endl;
            return 1;
        }

        standardOutput << "Version:1.01 Build Date:2017/3/29 System:Windows" << endl;
        return 1;
    }else
    {
        standardOutput << "Invaild arguments avalible for read." << endl;
        return 1;
    }


    countDown = setting.value("SerialPort_Setting/Timeout_Sec").toInt();
    colorDarkRange = setting.value("Mode_Flash/ColorRange_Dark").toInt();

    if(colorDarkRange > 255 || colorDarkRange < 0)
    {
        standardOutput << "colorDark setting error" << endl;
        return 1;
    }

    flashCount = setting.value("Mode_Flash/FlashCount").toInt();


    serialPortConnect portconnect(&serialPort, portName, color, colorDarkRange
                                  , flashCount, countDown, mode ,upperbound.toInt()
                                  , lowerbound.toInt(), flashSwitch);

    return a.exec();
}
