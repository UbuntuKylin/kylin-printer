#include <QThread>

#include "networkDeviceMonitor.h"
#include "deviceInformation.h"

NetworkDeviceMonitor *NetworkDeviceMonitor::getInstance()
{
    static NetworkDeviceMonitor *instance = nullptr;
    static QThread *thread;
    if (nullptr == instance) {
        instance = new NetworkDeviceMonitor();
        thread = new QThread();
        instance->sendTestinfo = false;
        instance->moveToThread(thread);
        connect(instance, SIGNAL(start()),
                instance, SLOT(detectStart()));
        thread->start();
    }
    return instance;
}

NetworkDeviceMonitor::NetworkDeviceMonitor()
{

}

NetworkDeviceMonitor::~NetworkDeviceMonitor()
{

}

void NetworkDeviceMonitor::detectStart()
{
    qDebug () <<"detectStart "<< QThread::currentThread();
    if (this->sendTestinfo == true){
        DeviceInformation test;
        test.host = "192.168.17.90";
        test.connectType = ConnectType::InfoFrom_NETWORK;
        test.uri = "socket://192.168.17.90:9100";
        test.name = "PRINTERFORTEST";
        test.model = "Brother";
        test.vendor = "DCP-7195DW";
        test.makeAndModel = "Brother DCP-7195DW";
        test.uuid = "e3248000-80ce-11db-8000-3c2af4f3d95f";
        test.serial = "E78133F0N443169";
        emit this->findNetworkConnect(test);
    }


    return;
}