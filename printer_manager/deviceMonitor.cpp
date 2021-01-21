#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h>
#include <unistd.h>
#include <regex>
#include <iostream>

#include <sys/inotify.h>

#include <cups/cups.h>

#include <QDebug>
#include <QString>
#include <QProcess>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QThread>
#include <QObject>
#include <QMetaType>
#include <QUrl>
#include <QSysInfo>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QEventLoop>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>

#include "deviceMonitor.h"

#define UEVENT_BUFFER_SIZE 2048

static int init_hotplug_sock(void)
{
    struct sockaddr_nl snl;
    const int buffersize = 16 * 1024 * 1024;
    int retval;
    memset(&snl, 0x00, sizeof(struct sockaddr_nl));
    snl.nl_family = AF_NETLINK;
    snl.nl_pid = getpid();
    snl.nl_groups = 1;
    int hotplug_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (hotplug_sock == -1)
    {
        printf("error getting socket: %s", strerror(errno));
        return -1;
    }
    /* set receive buffersize */
    setsockopt(hotplug_sock, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, sizeof(buffersize));
    retval = bind(hotplug_sock, (struct sockaddr *) &snl, sizeof(struct sockaddr_nl));
    if (retval < 0) {
        printf("bind failed: %s", strerror(errno));
        close(hotplug_sock);
        hotplug_sock = -1;
        return -1;
    }
    return hotplug_sock;
}

bool AddWatch(const QString &path)
{
    qDebug() << path;
    qDebug() << "im in AddWatch!";
    unsigned char buf[1024] = {0};
    struct inotify_event *event = NULL;
    int fd = inotify_init();
    int wd = inotify_add_watch(fd, path.toStdString().c_str(), IN_ALL_EVENTS);
    while(1) {
        fd_set fds;   
        FD_ZERO(&fds);                
        FD_SET(fd, &fds);
        if (select(fd + 1, &fds, NULL, NULL, NULL) > 0)                //监控fd的事件。当有事件发生时，返回值>0
        {   
            qDebug() << 123;
            int len, index = 0;   
            while (((len = read(fd, &buf, sizeof(buf))) < 0) && (errno == EINTR));       //没有读取到事件。
            while (index < len) {   
                    event = (struct inotify_event *)(buf + index);                      
                    // printf("event->mask: 0x%08x\n", event->mask);   
                    // printf("event->name: %s\n", event->name);                                       //获取事件。
                    qDebug() << "get!";
                    index += sizeof(struct inotify_event) + event->len;             //移动index指向下一个事件。
                    inotify_rm_watch(fd, wd);
                    return true;
            }   
        }   
    }
    inotify_rm_watch(fd, wd);
    return false;
}

QString getRetFromCommand(const QStringList &command)
{
    QProcess proc;
    QStringList options;
    options << "-c"<< command.join(" ");
    proc.closeWriteChannel();
    proc.start("bash", options);
    if (!proc.waitForStarted()) {
        ;
    }
    
    while (false == proc.waitForFinished())
    {
        ;
    }
    QString res = QString(proc.readAll());
    proc.close();
    if(res.right(1) == "\n")
        res.chop(1);
    return res;
}

QString getDeivceTypeFromPath(const QString &path)
{
    QString res;
    QString bInterfaceClass;
    QStringList bInterfaceClassPathList = getRetFromCommand(QStringList{"find", path ,"-name", "bInterfaceClass"}).split("\n");
    for (int i = 0; i < bInterfaceClassPathList.size(); i++) {
        bInterfaceClass = getRetFromCommand(QStringList{"cat", bInterfaceClassPathList.at(i)});
        if (bInterfaceClass == "00" || bInterfaceClass == "ff"){
            continue;
        }
        res = bInterfaceClass;
    }
    return res;
}


// DeviceInformation start
DeviceInformation::DeviceInformation()
{

}

DeviceInformation::DeviceInformation(const QString &qstr)
{
    if (qstr == "test") {
        this->devicePath   = "test_devicePath"; 
        this->deviceType   = "test_deviceType"; 
        this->busNumber    = "test_busNumber";
        this->deviceNumber = "test_deviceNumber";
        this->VID          = "test_VID";
        this->PID          = "test_PID";
        this->vendor       = "test_vendor";   
        this->model        = "test_model";     
        this->serial       = "test_serial"; 
    }
}

QString getPackageNameFromHttp(const DeviceInformation &device)
{
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
        #define __SYSTEM_VERSION__ "V10Professional"
    #else
        #define __SYSTEM_VERSION__ "V10"
    #endif

    QString arch = QSysInfo::currentCpuArchitecture();
    if (arch.contains("86")) {
        arch = "amd64";
    }
    else if (arch.contains("arm")) {
        arch = "arm64";
    }
    else if (arch.contains("mips")) {
        arch = "mips64el";
    }
    // https://api.kylinos.cn/api/v1/getprinterdrive?
    // systemVersion=V10
    // &framework=arm64
    // &pid=00a5
    // &vid=04f9
    // &product=Brother
    // &model=HL-3190CDW
    QString httpRequest = QString ( QString("https://api.kylinos.cn/api/v1/getprinterdrive")
                                  + "?" + QString("systemVersion=") + QString(__SYSTEM_VERSION__)
                                  + "&" + QString("framework=")     + arch
                                  + "&" + QString("pid=")           + device.PID
                                  + "&" + QString("vid=")           + device.VID
                                  + "&" + QString("product=")       + device.vendor
                                  + "&" + QString("model=")         + device.model
                                  );
    QNetworkAccessManager manager;
    QNetworkRequest netRequest;
    QNetworkReply *netReply;
    QEventLoop loop;

    // httpRequest = "https://api.kylinos.cn/api/v1/getprinterdrive?systemVersion=V10&framework=arm64&pid=00a5&vid=04f9&product=Brother&model=HL-3190CDW";
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    netRequest.setUrl(QUrl(httpRequest));
    netReply = manager.get(netRequest);

    QObject::connect(netReply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if (netReply->error() != QNetworkReply::NoError) {
        return "";
    }

    QByteArray strRateAll = netReply->readAll();
    qDebug() << strRateAll;
    if (strRateAll == "") {
        return "";
    }
    QJsonParseError jsonParserError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(strRateAll, &jsonParserError );

    if ( jsonDocument.isNull() || jsonParserError.error != QJsonParseError::NoError ) {
        qDebug () << "json解析失败";
        return "";
    }
    else {
        qDebug() << "json解析成功!";
    }
    QStringList ans;
    if (jsonDocument.isObject()) {
        QJsonObject jsonObject = jsonDocument.object();
        if ( jsonObject.contains("data")
            && jsonObject.value("data").isArray() ) {

            QJsonArray jsonArray = jsonObject.value("data").toArray();
            for ( int i = 0; i < jsonArray.size(); i++) {
                if (jsonArray.at(i).isString()) {
                    ans.append(jsonArray.at(i).toString());
                }
            }
        }
    }
    // qDebug()<< ans;
    
    return ans.join(",");
}

QDebug operator << (QDebug debug, const DeviceInformation &debugInfo)
{
    debug.noquote();
    QString info = QString  ( QString("\n")
                            + QString("+++++++++++++++++++++++++++++++++\n")
                            + QString("name         is: ") + debugInfo.name      + QString('\n')
                            + QString("sysPath      is: ") + debugInfo.sysPath      + QString('\n')
                            + QString("devicePath   is: ") + debugInfo.devicePath   + QString("\n")
                            + QString("deviceType   is: ") + debugInfo.deviceType   + QString("\n")
                            + QString("busNumber    is: ") + debugInfo.busNumber    + QString("\n")
                            + QString("deviceNumber is: ") + debugInfo.deviceNumber + QString("\n")
                            + QString("VID          is: ") + debugInfo.VID          + QString("\n")
                            + QString("PID          is: ") + debugInfo.PID          + QString("\n")
                            + QString("vendor       is: ") + debugInfo.vendor       + QString("\n")
                            + QString("model        is: ") + debugInfo.model        + QString("\n")
                            + QString("serial       is: ") + debugInfo.serial       + QString("\n")
                            + QString("uri          is: ") + debugInfo.uri          + QString("\n")
                            + QString("packageName  is: ") + debugInfo.packageName  + QString("\n")
                            + QString("makeAndModel is: ") + debugInfo.makeAndModel + QString("\n")
                            + QString("+++++++++++++++++++++++++++++++++\n")
                            );
    debug << info;
    return debug;
}
// DeviceInformation end








// DeviceMonitor start
DeviceMonitor *DeviceMonitor::getInstance()
{   

    static DeviceMonitor *instance = nullptr;
    static QThread *thread;
    if (nullptr == instance) {
        instance = new DeviceMonitor();
        thread = new QThread();
        instance->setFindPrinterOnly = true;
        instance->moveToThread(thread);
        connect(instance, SIGNAL(start()),
                instance, SLOT(detectStart()));
        thread->start();
    }
    return instance;
}

DeviceMonitor::~DeviceMonitor()
{

}



DeviceMonitor::DeviceMonitor()
{

}

void DeviceMonitor::detectStart()
{
    // qDebug() << "DeviceMonitor::detectStart";
    // qDebug() << QThread::currentThreadId();
    // usbDeivceAdd("bind@/devices/pci0000:00/0000:00:16.0/0000:0b:00.0/usb3/3-2");
    int hotplug_sock = init_hotplug_sock();
    while(1)
    {
        char buf[UEVENT_BUFFER_SIZE*2] = {0};
        // recv 会等待usb信号连入
        recv(hotplug_sock, &buf, sizeof(buf), 0);
        this->usbDeviceIdentify(QString(buf));
    }
}

bool DeviceMonitor::usbDeviceIdentify(const QString &qstr)
{
    // qDebug() << qstr;
    // qDebug() << "DeviceMonitor::usbDeviceIdentify";
    // bind@/devices/pci0000:00/0000:00:11.0/0000:02:01.0/usb1/1-1
    if (  qstr.indexOf("bind") == 0
      &&  qstr.contains("pci") 
      && !qstr.right( qstr.size() -1 - qstr.lastIndexOf('/') ).contains(":") 
      && !qstr.right( qstr.size() -1 - qstr.lastIndexOf('/') ).contains(".") 
    ) {

        this->usbDeivceAdd(qstr);
        return true;
    }
    // unbind@/devices/pci0000:00/0000:00:11.0/0000:02:01.0/usb1/1-1
    if (  qstr.contains("unbind")
      &&  qstr.contains("pci") 
      && !qstr.right( qstr.size() -1 - qstr.lastIndexOf('/') ).contains(":") 
      && !qstr.right( qstr.size() -1 - qstr.lastIndexOf('/') ).contains(".")
    ) {

        this->usbDeivceRemove(qstr);
        return true;
    }
    return false;
}



bool DeviceMonitor::usbDeivceAdd(const QString &qstr)
{
    QString path = QString("/sys") + qstr.right(qstr.size() - 1 - qstr.indexOf('@'));
    QString deviceType = getDeivceTypeFromPath(path);

    // 打印机设备类型为07
    if (this->setFindPrinterOnly == true) {
        if (deviceType != "07")
            return false;
    }
    // 判断是否重复
    if (this->m_mpDeviceInformationMap.contains(path)) {
        return false;
    }

    DeviceInformation newDevice;
    
    newDevice.sysPath      = path;
    newDevice.devicePath   = "/dev/" + getRetFromCommand(QStringList{"cat", path + "/uevent", "|" , "grep", "DEVNAME"}).remove("DEVNAME=");
    newDevice.deviceType   = deviceType;
    newDevice.busNumber    = getRetFromCommand(QStringList{"cat", path + "/busnum"});
    newDevice.deviceNumber = getRetFromCommand(QStringList{"cat", path + "/devnum"});
    newDevice.VID          = getRetFromCommand(QStringList{"cat", path + "/idVendor"});
    newDevice.PID          = getRetFromCommand(QStringList{"cat", path + "/idProduct"});
    newDevice.vendor       = getRetFromCommand(QStringList{"cat", path + "/manufacturer"});
    newDevice.model        = getRetFromCommand(QStringList{"cat", path + "/product"});
    newDevice.serial       = getRetFromCommand(QStringList{"cat", path + "/serial"});

    // direct usb://Cumtenn/CTP-2200N?serial=0123
    newDevice.uri = getRetFromCommand(QStringList{"lpinfo", "-v", "|" , "grep", "usb://"}).remove("direct ");
    // newDevice.uri = "usb://Cumtenn/CTP-2200N%20series?serial=0123";
    if (!newDevice.uri.contains("usb")) {
        newDevice.uri.clear();
    }
    if (newDevice.uri.size()) {
        QString tempUri = QUrl(newDevice.uri).toString();
        tempUri.remove("usb://");
        newDevice.vendor = tempUri.left(tempUri.indexOf("/"));
        tempUri.remove( tempUri.left( tempUri.indexOf("/") + 1 ) );

        //TODO: model带空格 只取第一个
        newDevice.model = tempUri.left(tempUri.indexOf("?")).split(" ").at(0);
        tempUri.remove( tempUri.left( tempUri.indexOf("?") + 1 ) );
        tempUri.remove("serial=");                
        newDevice.serial = tempUri;
    }
    this->m_mpDeviceInformationMap.insert(path, newDevice);
    qDebug() << "find a new painter!";
    // qDebug() << newDevice;
    emit this->findUsbConnect(newDevice);
    return true;
}

bool DeviceMonitor::usbDeivceRemove(const QString &qstr)
{
    QString path = QString("/sys") + qstr.right(qstr.size() - 1 - qstr.indexOf('@'));
    if (!this->m_mpDeviceInformationMap.contains(path)) {
        return false;
    }
    qDebug() << "remove a painter!";
    this->findUsbDisconnect(this->m_mpDeviceInformationMap[path]);
    this->m_mpDeviceInformationMap.remove(path);
    return true;
}

// 调试模式
void DeviceMonitor::setTestModelEnabled()
{
    QTimer timer(this);
    QObject::connect(&timer, SIGNAL(timeout()   ),
                      this,  SLOT  (sendTestSignals()));
    timer.start(5000);
}

void DeviceMonitor::sendTestSignals()
{
    static int flag = 0;
    static DeviceInformation test("test");
    if (!flag) {
        qDebug() << "a test new USB device add!";
        emit this->findUsbConnect(test);
    }
    else {
        qDebug() << "a test USB device removed!";
        emit this->findUsbDisconnect(test);
    }

    flag = ~flag;
}





QString getUriHead(const QString &uri)
{
    if (!uri.size())
        return "";
    QString head = uri.left(uri.indexOf(":"));
    if (head.size())
        return head;
    return "";
}

cups_dest_t *getDestWithURI(const char *uri)
{
    qDebug() << "im in getDestWithURI";
    qDebug() << uri;
    int i;
    cups_dest_t *dests, *dest;
    const char *value;
    int num_dests = cupsGetDests(&dests);

    for (i = num_dests, dest = dests; i > 0; i --, dest ++) {
        if (dest->num_options) {
            value = cupsGetOption("device-uri", dest->num_options, dest->options);
            if ( value != NULL && strcmp(uri, value) == 0) {
                return dest;
            }
        }
    }
    return NULL;
}

DeviceInformation getDeviceInformationFromDest(cups_dest_t *dest)
{
    DeviceInformation deviceInfo;
    if (dest->name != NULL)
        deviceInfo.name = dest->name;
    if (dest->num_options) {
        deviceInfo.makeAndModel = cupsGetOption("printer-make-and-model", dest->num_options, dest->options);
        deviceInfo.uri = cupsGetOption("device-uri", dest->num_options, dest->options);
    }
    QString tempUri = deviceInfo.uri;
    if (tempUri.contains("ipp") && tempUri.contains("%20")) {
        tempUri.replace(tempUri.indexOf("%20"),3, "/");
        // tempUri[tempUri.indexOf("%20")] = "/";
    }
    tempUri = QUrl(tempUri).toString();
    QString head = getUriHead(tempUri);

    if (head == "usb") {
        tempUri.remove("usb://");
        deviceInfo.vendor = tempUri.left(tempUri.indexOf("/"));
        tempUri.remove( tempUri.left( tempUri.indexOf("/") + 1 ) );

        //TODO: model带空格 只取第一个
        deviceInfo.model = tempUri.left(tempUri.indexOf("?")).split(" ").at(0);
        tempUri.remove( tempUri.left( tempUri.indexOf("?") + 1 ) );
        tempUri.remove("serial=");
        if (tempUri.contains("&"))
            deviceInfo.serial = tempUri.left(tempUri.indexOf("&"));
        else
            deviceInfo.serial = tempUri;
    }
    else if (head == "ipp" || head == "ipps") {
        tempUri.remove("ipp://");
        deviceInfo.vendor = tempUri.left(tempUri.indexOf("/"));
        tempUri.remove( tempUri.left(tempUri.indexOf("/") + 1) );
        tempUri = tempUri.left(tempUri.indexOf("."));
        deviceInfo.model = tempUri.split(" ").at(0);
    }
    return deviceInfo;
}

QList<DeviceInformation> DeviceMonitor::getAllPrinterWithPDD(const bool usbConnectOnly)
{
    QList<DeviceInformation> res;
    int i;
    cups_dest_t *dests, *dest;
    int num_dests = cupsGetDests(&dests);
    qDebug() << num_dests;
    for (i = num_dests, dest = dests; i > 0; i --, dest ++)
    {
        if (dest->num_options) {
            QString uri = cupsGetOption("device-uri", dest->num_options, dest->options);
            if (usbConnectOnly == true && !uri.contains("usb://")) {
                continue;
            }
            QString value = cupsGetOption("printer-make-and-model", dest->num_options, dest->options);
            if (value.size() && value != "Remote Printer") {
                DeviceInformation device = getDeviceInformationFromDest(dest);
                res.append(device);
            }
        }
    }
    return res;
}


QList<DeviceInformation> DeviceMonitor::getAllPrinterConnected()
{   
    QList<DeviceInformation> res;
    QStringList uriList;
    uriList.append( getRetFromCommand( QStringList{"lpinfo", "-v", "|" , "grep", "-e", "\'usb://\'", "-e", "\'ipp://\'"})
                    .remove("direct ")
                    .remove("network ")
                    .split("\n") 
                  );

    for (int i = 0; i < uriList.size(); i++) {
        // qDebug () << i;
        // printf("%s\n",uriList.at(i).toStdString().c_str());
        // qDebug() << uriList.at(i).toStdString().c_str();
        cups_dest_t *dest = getDestWithURI(uriList.at(i).toStdString().c_str());

        if (dest == NULL) {
            qDebug() << i << "is not find";
            continue;
        }
        DeviceInformation device = getDeviceInformationFromDest(dest);
        res.append(device);
    }
    return res;
}
