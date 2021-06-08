#ifndef DEVICEINFORMATION_H
#define DEVICEINFORMATION_H

#include <QString>
#include <QStringList>
#include <QDebug>

enum class ConnectType : int
{
    InfoFrom_Invalid = 0,
    InfoFrom_USB = 0, 
    InfoFrom_NETWORK, 
};

class DeviceInformation
{
public:
    ConnectType connectType;
    QString host;
    QString name;       // 打印机的名字
    QString sysPath;    // sys下的目录
    QString devicePath; // dev下的目录 绝对路径
    QString deviceType; // 设备种类 打印机为07
    QString busNumber; 
    QString deviceNumber;
    QString VID;       // usb vid
    QString PID;       // usb pid
    QString vendor;    // 供应商
    QString model;     // 型号
    QString serial;    // 序列号
    QString uri;            // 设备uri
    QStringList packagesName;    // 包名
    QString makeAndModel; // make-and-model
    QString uuid;
    
    DeviceInformation();
    DeviceInformation(const QString &);
    // QString name();
    friend QStringList getPackagesNameFromHttp(DeviceInformation &);
    // static QString getPackageNameFromHttp(const QString &);
    // qdebug 重定向
    friend QDebug operator << (QDebug debug, const DeviceInformation &);
};

#endif // DEVICEINFORMATION_H