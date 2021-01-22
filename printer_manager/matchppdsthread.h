#ifndef MATCHPPDSTHREAD_H
#define MATCHPPDSTHREAD_H

#include <QObject>
#include "findppdsthread.h"

enum deviceType
{
    USB = 0,
    IP,
    SHARED,
    NONEPRINTER
};
typedef QPair<QMap<int, QStringList>, bool> resultPair;

class MatchPPDsThread : public QObject
{
    Q_OBJECT
public:
    explicit MatchPPDsThread(QObject *parent = nullptr);

signals:
    void matchFailed();
    void matchResultSignal(resultPair res);
public slots:
    void initPPDMatch(QString bandName, QString printerName, myMap data, int type);

private:
    QMap<QString, QMap<QString, PPDsAndAttr>> originData;

    QString printerBandName = nullptr;
    QString printerModelName = nullptr;

    QPair<QMap<int, QStringList>, bool> eactMatch(QString printerModel, QMap<QString, PPDsAndAttr> map, int type);
    QPair<QMap<int, QStringList>, bool> genericMatch(QString printerModel, QMap<QString, PPDsAndAttr> map, int type);

    bool exactMatchFlag = false;

    QString originStringHandle(QString originString);

    QPair<QMap<int, QStringList>, bool> matchResult;
};

#endif // MATCHPPDSTHREAD_H
