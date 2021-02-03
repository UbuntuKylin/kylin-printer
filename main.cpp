#include <QStandardPaths>
#include <QApplication>
#include <QObject>
#include <QScreen>
#include <fcntl.h>
#include <syslog.h>

#include <QDebug>
#include <QFile>
#include <QMutex>
#include <QDateTime>
#include <sys/inotify.h>

#include <QLocale>
#include <QStandardPaths>
#include <QLibraryInfo>
#include <QDir>


#include "popwindow.h"
#include "deviceMonitor.h"

#include <QFileSystemWatcher>
#include <QThread>
#include <QLabel>
#include <QMetaType>
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // 加锁
    static QMutex mutex;
    mutex.lock();

    QByteArray localMsg = msg.toLocal8Bit();

    QString strMsg("");
    switch (type)
    {
    case QtDebugMsg:
    default:
        strMsg = QString("Debug:");
        break;
    case QtWarningMsg:
        strMsg = QString("Warning:");
        break;
    case QtCriticalMsg:
        strMsg = QString("Critical:");
        break;
    case QtFatalMsg:
        strMsg = QString("Fatal:");
        break;
        
    }

    // 设置输出信息格式
    QString strDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");
    QString strMessage = QString(QString() +
                                 "Message:%1"
                                 // +"        File:%2\n"
                                 // +"        Line:%3\n"
                                 // +"        Function:%4\n"
                                 // +"        DateTime:%5"
                                 )
                             .arg(localMsg.constData());
    // .arg(context.file).arg(context.line).arg(context.function).arg(strDateTime);

    // 输出信息至文件中（读写、追加形式）
    QFile file("log.txt");
    file.open(QIODevice::ReadWrite | QIODevice::Append);
    QTextStream stream(&file);
    stream << strMessage << "\r\n";
    file.flush();
    file.close();

    // 解锁
    mutex.unlock();
}

int main(int argc, char *argv[])
{
    printf("Program start ...\n");

    qRegisterMetaType<DeviceInformation>("DeviceInformation");
    qRegisterMetaType<DeviceInformation>("DeviceInformation&");
    //    qInstallMessageHandler(myMessageOutput);
    QApplication app(argc, argv);
    QStringList homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    int fd = open(QString(homePath.at(0) + "/.config/kylin-printer%1.lock").arg(getenv("DISPLAY")).toUtf8().data(),
                  O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (fd < 0)
    {
        exit(1);
    }
    if (lockf(fd, F_TLOCK, 0))
    {
        syslog(LOG_ERR, "Can't lock single file, kylin-printer is already running!");
        exit(0);
    }


    PopWindow popWid;
//qDebug()<<DeviceMonitor::getAllPrinterConnected();
    return app.exec();
}
