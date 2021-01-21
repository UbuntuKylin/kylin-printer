#ifndef __UKUI_APT_H__
#define __UKUI_APT_H__

//Use QObject
#include <QObject>
#include <QDebug>
//Use libqapt as apt interface
#include <qapt/backend.h>
#include <qapt/debfile.h>
#include <qapt/package.h>
#include <qapt/transaction.h>



enum class ukuiAptError : int
{
    UKUI_APT_SUCCESS = 0,
    UKUI_APT_CANNOT_ACCESS_SERVER,
    UKUI_APT_NO_SUCH_PACKAGE,
    UKUI_APT_INSTALL_FAIL,
};

enum class ukuiInstallStatus : int
{
    UKUI_INSTALL_START = 0,
    UKUI_INSTALL_IN_PROGRESS,
    UKUI_INSTALL_SUCCESS,
    UKUI_INSTALL_FAIL,
};

class ukuiApt : public QObject
{
    Q_OBJECT
public:
    ukuiApt();
    explicit ukuiApt(QString debName);
    ~ukuiApt();



    //Apt
    bool isPackageInSourceList(QString pkgName);
    //
    bool getDependencies(QString pkgName, QList<QString> &dependPkgs);
    //
    bool installStatus(QString debName, int &installStatus);
    void updatePkgCache();
    bool setDebName(QString debName);

    bool preInstall();
    bool install();
    void postInstall();
    void getStatus()
    {
        if (m_trans != nullptr)
            qDebug() << m_trans->status();
    }

private:
    /* data */
    //QString m_debName;
    QApt::Backend *m_backend;
    QApt::DebFile *m_debFile;
    QApt::Transaction *m_trans;
    QApt::TransactionStatus m_installStatus;

private:
    bool initial();

Q_SIGNALS:
    void reportInstallStatus(ukuiInstallStatus status);
    void alreadyInstallSignal();                            //发送已经安装的信号
private Q_SLOTS:
    void onInstallStatusChanged(QApt::TransactionStatus status);
    void onInstallErrorOccured(QApt::ErrorCode status);
    void onPackageStatusChanged();
    void onProgressChanged(int progress);
    void onFinished(QApt::ExitStatus exitStatus);
};

//Add install local package and remote package with DBus KylinUpdateManager
class PrinterUtility
{
public:
    PrinterUtility() {}
    ~PrinterUtility() {}
    //bool getDebNames(struct PrinterInfo printer, QStringList &debNames);
    void installPackage(QString packageName);
    void installLocalDeb(QString debFilePath);

};



#endif //__UKUI_APT_H__
