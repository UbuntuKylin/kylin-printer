#ifndef NETWORKDEVICEMONITOR_H
#define NETWORKDEVICEMONITOR_H

#include <QObject>
#include "deviceInformation.h"


// NetworkDeviceMonitor 用法：
// NetworkDeviceMonitor *p = NetworkDeviceMonitor::getInstance();

// 设置是否发送测试信息 默认为false
// 如果修改为true 发送一条测试信息
// p->sendTestinfo = false;
// connect (p,     SIGNAL(findNetworkConnect(DeviceInformation &)   ),
//           类名,  SLOT (要触发的槽  ));

// p->start();

class NetworkDeviceMonitor : public QObject
{
    Q_OBJECT

protected:
    QMap<QString, DeviceInformation> m_mpDeviceInformationMap;


public:
    // 设置是否发送测试信息 默认为false
    // 如果修改为true 发送一条测试信息
    bool sendTestinfo;
    static NetworkDeviceMonitor *getInstance();
    ~NetworkDeviceMonitor();

private:
    NetworkDeviceMonitor(/* args */);

protected Q_SLOTS:
    void detectStart       ();

Q_SIGNALS:
    void start();
    void findNetworkConnect (DeviceInformation &);
};






#endif // NETWORKDEVICEMONITOR_H