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

#include <QSysInfo>

#include "deviceInformation.h"

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

QStringList getPackagesNameFromHttp(DeviceInformation &device)
{
    QStringList res;
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
                                  + "&" + QString("arch=")          + arch
                                  + "&" + QString("pid=")           + device.PID
                                  + "&" + QString("vid=")           + device.VID
                                  + "&" + QString("manufacter=")    + device.vendor
                                  + "&" + QString("model=")         + device.model
                                  );
    QNetworkAccessManager manager;
    QNetworkRequest netRequest;
    QNetworkReply *netReply;
    QEventLoop loop;

    // httpRequest = "https://api.kylinos.cn/api/v1/getprinterdrive?systemVersion=V10Professional&arch=amd64&manufacter=cumtenn";
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    netRequest.setUrl(QUrl(httpRequest));
    netReply = manager.get(netRequest);

    QObject::connect(netReply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if (netReply->error() != QNetworkReply::NoError) {
        return res;
    }

    QByteArray strRateAll = netReply->readAll();
    qDebug() << strRateAll;
    if (strRateAll == "") {
        return res;
    }
    QJsonParseError jsonParserError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(strRateAll, &jsonParserError );

    if ( jsonDocument.isNull() || jsonParserError.error != QJsonParseError::NoError ) {
        qDebug () << "json解析失败";
        return res;
    }
    else {
        qDebug() << "json解析成功!";
    }
    if (jsonDocument.isObject()) {
        QJsonObject jsonObject = jsonDocument.object();
        if ( jsonObject.contains("data")
            && jsonObject.value("data").isArray() ) {

            QJsonArray jsonArray = jsonObject.value("data").toArray();
            for ( int i = 0; i < jsonArray.size(); i++) {
                if (jsonArray.at(i).isString()) {
                    res.append(jsonArray.at(i).toString());
                }
            }
        }
    }
    device.packagesName = res;
    return res;
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
                            + QString("packageName  is: ") + debugInfo.packagesName.join(",") + QString("\n")
                            + QString("makeAndModel is: ") + debugInfo.makeAndModel + QString("\n")
                            + QString("+++++++++++++++++++++++++++++++++\n")
                            );
    debug << info;
    return debug;
}
// DeviceInformation end