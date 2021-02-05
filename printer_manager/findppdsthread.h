#ifndef FINDPPDSTHREAD_H
#define FINDPPDSTHREAD_H

#include <QObject>
#include <QMetaType>
#include<QMutexLocker>
#include "cupsconnection4ppds.h"

template <typename>
struct TD;

class PPDsAndAttr
{
public:
    QString ppdname;
    QString ppd_make;
    QString ppd_product;
    QString ppd_make_and_model;
    QString ppd_device_id;
};
Q_DECLARE_METATYPE(PPDsAndAttr)

typedef QMap<QString,QMap<QString,PPDsAndAttr> > ppdPrinterMap;
class FindPPDsThread : public QObject
{
    Q_OBJECT
public:
    explicit FindPPDsThread(http_t* httpConnection, QObject *parent = nullptr);

signals:
    void gotAllHandledPPDs(ppdPrinterMap origin);


public slots:
    void initPPDMapConstruct();
    //void getPPDsByPrinter(QString printerName);

private:
    QMutex m_mutex;
    http_t* newHttp = nullptr;
    ipp_t* ppdRequest = nullptr;
    ipp_t* cupsAnswer = nullptr;
    ipp_attribute_t *attr = nullptr;

    QMap<QString,QMap<QString,PPDsAndAttr>> originPPDs;
    //std::multimap<QString,QMap<QString, PPDsAndAttr>> allHandledPPDs;

    QMap<QString,QMap<QString,PPDsAndAttr>> getPPDsFromCUPS();

    void handleOriginPPDs(QMap<QString, QStringList> tempMap);

};

#endif // FINDPPDSTHREAD_H
