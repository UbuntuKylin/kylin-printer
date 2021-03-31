#include <QTimer>
#include <QDateTime>
#include "cupsconnection4ppds.h"
#include "ukuiPrinter.h"

CupsConnection4PPDs::CupsConnection4PPDs()
{
    qDebug() << "CUPS开始建立" << QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss");
    server = cupsServer();

    ippPortNum = ippPort();

    encryptionType = cupsEncryption();

    int cancel = 0;
    ppdRequestConnection = httpConnect2(server, ippPortNum, nullptr, AF_UNSPEC, (http_encryption_t)encryptionType, 1, 30000, &cancel);

    if (!ppdRequestConnection)
    {
        throw std::runtime_error("Failed to construct ppdRequestConnection!");
    }
    else
    {
        qDebug() << "CUPS服务ppdRequestConnection建立成功!";
    }


    qDebug() << "CUPS建立完成" << QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss");
}

CupsConnection4PPDs *CupsConnection4PPDs::getInstance()
{
    static CupsConnection4PPDs *cupsInstance = nullptr;
    if (cupsInstance == nullptr)
    {
        try
        {
            cupsInstance = new CupsConnection4PPDs;
        }
        catch (const std::runtime_error &re)
        {
            qDebug() << "runtime_error:" << re.what();
        }
    }
    return cupsInstance;
}
