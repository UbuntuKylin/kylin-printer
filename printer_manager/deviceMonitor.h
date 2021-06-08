#ifndef __DEVICE_MONITOR__
#define __DEVICE_MONITOR__

#include <QObject>
#include <QMap>
#include <QTimer>
#include <QThread>
#include <QDebug>

#include "deviceInformation.h"

QString getRetFromCommand(const QStringList &command);


// DeviceMonitor 用法：
// DeviceMonitor *p = DeviceMonitor::getInstance();
// p->setFindPrinterOnly = false;
// connect (&p,     SIGNAL(findUsbConnect(DeviceInformation &)   ),
//           类名,  SLOT (要触发的槽  ));

// connect (&p,      SIGNAL(findUsbDisconnect(DeviceInformation &)   ),
//           类名,   SLOT (要触发的槽  ));
// p->start();
// 要注意的问题：不能使用lambda表达式，不然依然是在子线程中运行；
// qt 画ui只能在主线程中
class DeviceMonitor : public QObject
{
    Q_OBJECT

protected:
    QMap<QString, DeviceInformation> m_mpDeviceInformationMap;

public:
    // 设置是否需要只需要打印机 默认为true
    // 如果修改为false 则会识别所有接入的usb设备
    bool setFindPrinterOnly;
    static QList<DeviceInformation> getAllPrinterWithPDD(const bool usbConnectOnly);
    static QList<DeviceInformation> getAllPrinterConnected();
public:
    static DeviceMonitor *getInstance();
    ~DeviceMonitor();

private:
    DeviceMonitor();

protected Q_SLOTS:
    void detectStart       ();
protected:
    bool usbDeviceIdentify (const QString &);
    bool usbDeivceAdd      (const QString &);
    bool usbDeivceRemove   (const QString &);

Q_SIGNALS:
    void start();
    void findUsbConnect    (DeviceInformation &);
    void findUsbDisconnect (DeviceInformation &);



// 调试模式: 5s发送一次findUsbConnect信号 再5s发送一次findUsbDisConnect信号 以此类推
// 用法:
// p->setTestModelEnabled();
public:
    void setTestModelEnabled();
protected Q_SLOTS:
    void sendTestSignals();
};

#endif // __DEVICE_MONITOR__
