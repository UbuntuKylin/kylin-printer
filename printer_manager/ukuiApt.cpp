#include <QDebug>
#include <QFileInfo>
#include "ukuiApt.h"
#include "manualinstallwindow.h"


static void dgbPrintPackage(QApt::DebFile *debFile)
{
}

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

ukuiApt::ukuiApt(/* args */) : m_backend(nullptr),
                               m_debFile(nullptr),
                               m_trans(nullptr)
{

    if (initial() == false)
        return;
}

ukuiApt::ukuiApt(QString debName) : m_backend(nullptr),
                                    m_debFile(nullptr),
                                    m_trans(nullptr)
{
    ManualInstallWindow *manual = new ManualInstallWindow;
    connect(this ,&ukuiApt::alreadyInstallSignal,manual,&ManualInstallWindow::alreadyInstallSlot);

    if (initial() == false)
        return;

    setDebName(debName);
}

ukuiApt::~ukuiApt()
{
    if (m_debFile != nullptr)
        delete m_debFile;

    if (m_backend != nullptr)
        delete m_backend;

    if (m_trans != nullptr)
        delete m_trans;
}

bool ukuiApt::setDebName(QString debName)
{
    QFileInfo fi(debName);
    if (m_debFile == nullptr)
    {
        m_debFile = new QApt::DebFile(fi.absoluteFilePath());
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
        return false;
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
