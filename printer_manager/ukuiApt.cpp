#include <QDebug>
#include <QProcess>
#include <QFileInfo>
#include <QtDBus>
#include "ukuiApt.h"

bool ukuiApt::initial()
{
    if (m_backend == nullptr)
    {
        m_backend = new QApt::Backend();
        if (m_backend == nullptr)
        {
            qCritical() << "Failed to alloc m_backend";
            return false;
        }

        if (!m_backend->init())
        {
            qCritical() << "Failed to init m_backend";
            return false;
        }
        connect(m_backend, SIGNAL(packageChanged()), this, SLOT(onPackageStatusChanged()));
    }
    QApt::FrontendCaps caps = (QApt::FrontendCaps)(QApt::DebconfCap);
    m_backend->setFrontendCaps(caps);
    return true;
}

ukuiApt::ukuiApt(QString debName) : m_backend(nullptr),
                                    m_debFile(nullptr),
                                    m_trans(nullptr),
                                    m_initSucceed(false)
{
    if (setDebName(debName) ==false)
        return ;

    if (initial() == false)
        return;
    
    m_initSucceed = true;
}

ukuiApt::~ukuiApt()
{
    qDebug() << "~ukuiApt()";
    if (m_debFile != nullptr)
        delete m_debFile;

    if (m_backend != nullptr)
        delete m_backend;

    if (m_trans != nullptr)
        delete m_trans;
}

void ukuiApt::checkDebianPackageValid(QString packageName,QString &outMsg,QString &errMsg)
{
    QProcess p;
    QString cmd = QString("dpkg-deb -W %1").arg(packageName);
    p.start("bash",QStringList()<<"-c" <<cmd);
    p.waitForFinished();
    outMsg = p.readAllStandardOutput();
    errMsg = p.readAllStandardError();
    qDebug()<<"Cmd:"<<cmd;
    qDebug()<<"Result"<<outMsg<<" Error:"<<errMsg;
}

bool ukuiApt::setDebName(QString debName)
{
    QString errMsg="";
    QString outMsg="";
    checkDebianPackageValid(debName,outMsg,errMsg);
    if (errMsg.compare("")!=0)
    {
        return false;
    }
    QFileInfo fi(debName);
    if (m_debFile == nullptr)
    {
        m_debFile = new QApt::DebFile(fi.absoluteFilePath());
    }else{
        qDebug() << "m_debFile is not null";
    }
    
    if (!m_debFile->isValid())
    {
        qDebug()<< "Package " << debName << " is not valid.";
        delete m_debFile;
        m_debFile = nullptr;
        return false;
    }
    
    return true;
}
//install dependencies
bool ukuiApt::preInstall()
{
    return true;
}

bool ukuiApt::install()
{
    if(m_initSucceed == false){
        qDebug() << "failed to initial apt...";
        return false;
    }
    QString name = m_debFile->packageName();
    qDebug() << "Package Name:" << name;
    QApt::Package *pkg = m_backend->package(m_debFile->packageName());
    if (!pkg)
    {
        return false;
    }
    int res = QApt::Package::compareVersion(m_debFile->version(), pkg->availableVersion());
    if (res != 0)
    {
        qDebug() << "Package version is different from available version";
        qDebug() << "PKG Version:" << m_debFile->version() << ", Availablev Vesrion" << pkg->availableVersion();
    }
    else
    {
        qDebug() << "PKG Version:" << m_debFile->version();
    }

    bool isInstalled = pkg->isInstalled();
    if (isInstalled == false)
    {
        pkg->setInstall();
        m_trans = m_backend->installFile(*m_debFile);
        qDebug() << "1Current status is" << m_trans->status();
        m_trans->setLocale(".UTF-8");
        connect(m_trans, &QApt::Transaction::statusChanged,
                this, &ukuiApt::onInstallStatusChanged);
        connect(m_trans, &QApt::Transaction::errorOccurred,
                this, &ukuiApt::onInstallErrorOccured);
        connect(m_trans, SIGNAL(progressChanged(int)),
                this, SLOT(onProgressChanged(int)));
        connect(m_trans, SIGNAL(finished(QApt::ExitStatus)),
                this, SLOT(onFinished(QApt::ExitStatus)));

        m_trans->run();
    }
    else
    {
        qDebug() << "Package:" << m_debFile->packageName() << " is already installed.";

        emit alreadyInstallSignal();
        return true;
    }
    return true;
}
void ukuiApt::postInstall()
{
}

void ukuiApt::onInstallStatusChanged(QApt::TransactionStatus status)
{
    qDebug() << "onInstallStatusChanged" << (int)status;
    switch (status)
    {
    case QApt::RunningStatus:
    case QApt::DownloadingStatus:
    {
        m_installStatus = status;
        //emit reportInstallStatus(ukuiInstallStatus::UKUI_INSTALL_IN_PROGRESS);
    }
    break;
    case QApt::FinishedStatus:
    {
        m_installStatus = status;
        //emit reportInstallStatus(ukuiInstallStatus::UKUI_INSTALL_SUCCESS);
    }
    default:
        break;
    }
}
void ukuiApt::onInstallErrorOccured(QApt::ErrorCode error)
{
    qDebug() << "onInstallStatusChanged" << (int)error;
    switch (error)
    {
    case QApt::AuthError:
    case QApt::LockError:
        break;
    default:
        break;
    }
}

void ukuiApt::onPackageStatusChanged()
{
    qDebug() << "onPackageStatusChanged";
}
void ukuiApt::onProgressChanged(int progress)
{
    qDebug() << "onProgressChanged :" << progress;
}

void ukuiApt::onFinished(QApt::ExitStatus exitStatus)
{
    qDebug() << "onFinished";
    emit reportInstallStatus(ukuiInstallStatus::UKUI_INSTALL_SUCCESS);
}

void PrinterUtility::installPackage(QString packageName)
{
    qDebug() << "Package:" << packageName;
    qDebug() << "installPackage 1:";
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface dbus_iface("cn.kylinos.KylinUpdateManager", "/cn/kylinos/KylinUpdateManager",
                              "cn.kylinos.KylinUpdateManager", bus);

    qDebug() << dbus_iface.call("install_and_upgrade", packageName);
    //dbus_iface.call("exit");
}

void PrinterUtility::installLocalDeb(QString debFilePath)
{
    qDebug() << "Package:" << debFilePath;
    qDebug() << "installLocalDeb 2:";
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface dbus_iface("cn.kylinos.KylinUpdateManager", "/cn/kylinos/KylinUpdateManager",
                              "cn.kylinos.KylinUpdateManager", bus);

    qDebug() << dbus_iface.call("install_debfile", debFilePath);
    //dbus_iface.call("exit");
}