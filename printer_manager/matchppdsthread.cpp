#include <QThread>
#include "matchppdsthread.h"

MatchPPDsThread::MatchPPDsThread(QObject *parent) : QObject(parent)
{
    qDebug() << QString("MatchPPDsThread thread id:") << (QThread::currentThreadId());
}

void MatchPPDsThread::initPPDMatch(QString bandName, QString printerName, myMap data, int type)
{
    QMutexLocker lockData( &m_mutex);
    qDebug() << "In initPPDMatch!";
    originData = data;
    QStringList printerNameList = (bandName + ' ' + printerName).split(' ');
    printerBandName = nullptr;
    printerModelName = nullptr;

    printerBandName = bandName.toLower();
    if (!printerBandName.isEmpty())
    {
        for (int i = 1; i < printerNameList.size(); ++i)
        {
            QString tempLetterAndNum = nullptr;
            tempLetterAndNum = originStringHandle(printerNameList[i]);

            printerModelName.append(tempLetterAndNum);
            printerModelName.append(' ');
        }
    }

    if (originData.find(printerBandName) != originData.end())
    {
        auto &mfg = originData[printerBandName];

        matchResult = exactMatch(printerModelName, mfg, type);

        if(matchResult.first.size())
        {
            emit matchResultSignal(matchResult);
        }
        else
        {
            qDebug() << printerBandName << "厂商在该机器上已安装的驱动程序中没有能匹配当前打印机型号的驱动";
            auto &mfg = originData["generic"];
            qDebug() << "generic对应的map大小为" << mfg.size();

            matchResult = genericMatch(printerModelName, mfg, type);
            emit matchResultSignal(matchResult);
        }
    }
    else
    {
        qDebug() << printerBandName << "厂商尚未存在于总map中";
        auto &mfg = originData["generic"];
        qDebug() << "generic对应的map大小为" << mfg.size();

        matchResult = genericMatch(printerModelName, mfg, type);
        emit matchResultSignal(matchResult);
    }
}

QPair<QMap<int, QStringList>, bool> MatchPPDsThread::genericMatch(QString printerModel, QMap<QString, PPDsAndAttr> map, int type)
{

    QMap<int, QString> tempPPDs;
    tempPPDs.clear();

    QStringList printerModelList = printerModel.split(' ');
    if (printerModelList.size() <= 2)
    {
        emit matchFailed();
    }
    printerModelList.removeAt(0);

    printerModel.clear();

    foreach (QString it , printerModelList)
    {
        printerModel.append(it);
    }

    QString tempPrinterModel = nullptr;
    foreach (auto &letter, printerModel.toLower())
    {
        if (letter.isSpace())
        {
            continue;
        }
        else
        {
            tempPrinterModel.append(letter);
        }
    }

    QMap<int, QStringList> ret;
    ret.clear();
    int numOfMatchedPPD = 0;
    int size = tempPrinterModel.size();
    for (int i = size ; i > 2; i--)
    {
        QStringList tempMakeAndModel = {};
        QString currentStr = tempPrinterModel.left(i);

        foreach (auto keyPPDs, map.keys())
        {
            if (type == USB && map[keyPPDs].ppdname.contains("driverless"))
            {
                continue;
            }
            else
            {
                QString handledPPDMakeAndModel;
                handledPPDMakeAndModel = originStringHandle(keyPPDs);

                QString tempHandledPPDMakeAndModel = nullptr;

                foreach (auto &letter, handledPPDMakeAndModel)
                {
                    if (letter.isSpace())
                    {
                        continue;
                    }
                    else
                    {
                        tempHandledPPDMakeAndModel.append(letter);
                    }
                }
                handledPPDMakeAndModel.clear();
                handledPPDMakeAndModel = tempHandledPPDMakeAndModel;

                if (handledPPDMakeAndModel.contains(currentStr, Qt::CaseInsensitive))
                {
                    if (ret.find(i) == ret.end())
                    {
                        numOfMatchedPPD++;
                        tempMakeAndModel.append(map[keyPPDs].ppdname);
                        ret.insert((i), tempMakeAndModel);
                    }
                    else
                    {
                        numOfMatchedPPD++;
                        ret[i].append(map[keyPPDs].ppdname);
                    }
                }
                if(3 == numOfMatchedPPD)
                {
                    break;
                }
            }
        }

        if (3 == numOfMatchedPPD)
        {
            break;
        }
    }

    qDebug() << "没找到和" << printerModel << "匹配的PPD文件,使用通用文件";
    foreach (auto it, ret.keys())
    {
        qDebug() << "找到匹配" << it << "个字符的PPD文件" << ret[it].size() << "个";
    }

    if(ret.size() == 0)
    {
        qDebug() << "没找到和" << printerModel << "相近的通用驱动文件";
        if (ret.find(0) == ret.end())
        {
            QStringList noMatchGeneric;
            QMap<QString, PPDsAndAttr>::iterator it = map.begin();
            int i = 0;
            for(;it != map.end();it++)
            {
                if(i < 3)
                {
                    if(it.key().contains("text"))
                    {
                        noMatchGeneric.append(it.value().ppdname);
                        i++;
                    }

                }
                else
                {
                    break;
                }
            }

            ret.insert((0), noMatchGeneric);
        }
    }

    return QPair<QMap<int, QStringList>, bool>(ret, false);

}

QPair<QMap<int, QStringList>, bool> MatchPPDsThread::exactMatch(QString printerModel, QMap<QString, PPDsAndAttr> map, int type)
{
    exactMatchFlag = false;
    QMap<int, QString> tempPPDs;
    tempPPDs.clear();

    QStringList printerModelList = printerModel.split(' ');
    if (printerModelList.size() <= 2)
    {
        emit matchFailed();
    }
    QString tempPrinterModel = nullptr;
    foreach (auto &letter, printerModel.toLower())
    {
        if (letter.isSpace())
        {
            continue;
        }
        else
        {
            tempPrinterModel.append(letter);
        }
    }

    QMap<int, QStringList> ret;
    ret.clear();
    int numOfMatchedPPD = 0;
    int size = tempPrinterModel.size();
    for (int i = size ; i > 2; i--)
    {
        QStringList tempMakeAndModel = {};
        QString currentStr = tempPrinterModel.left(i);

        foreach (auto keyPPDs, map.keys())
        {

            if (type == USB && map[keyPPDs].ppdname.contains("driverless"))
            {
                continue;
            }
            else
            {
                QString handledPPDMakeAndModel;
                handledPPDMakeAndModel = originStringHandle(keyPPDs);

                QString tempHandledPPDMakeAndModel = nullptr;

                foreach (auto &letter, handledPPDMakeAndModel)
                {
                    if (letter.isSpace())
                    {
                        //qInfo() << "字符串中的空格，不处理！";
                    }
                    else
                    {
                        tempHandledPPDMakeAndModel.append(letter);
                    }
                }
                handledPPDMakeAndModel.clear();
                handledPPDMakeAndModel = tempHandledPPDMakeAndModel;

                if (handledPPDMakeAndModel.contains(currentStr, Qt::CaseInsensitive))
                {
                    if (i == size)
                    {
                        exactMatchFlag = true;
                        tempMakeAndModel.append(map[keyPPDs].ppdname);
                        ret.insert(size, tempMakeAndModel);
                        break;
                    }

                    if (ret.find(i) == ret.end())
                    {
                        numOfMatchedPPD++;
                        tempMakeAndModel.append(map[keyPPDs].ppdname);
                        ret.insert((i), tempMakeAndModel);
                    }
                    else
                    {
                        numOfMatchedPPD++;
                        ret[i].append(map[keyPPDs].ppdname);
                    }
                }
                if(3 == numOfMatchedPPD)
                {
                    break;
                }
            }
        }

        if (exactMatchFlag || (3 == numOfMatchedPPD))
        {
            break;
        }
    }
    if (!exactMatchFlag)
    {
        qDebug() << "没找到和" << printerModel << "精准匹配的PPD文件";
        foreach (auto it, ret.keys())
        {
            qDebug() << "找到匹配" << it << "个字符的PPD文件" << ret[it].size() << "个";
        }
        if(!ret.size())
        {
            ret.clear();
        }
    }
    else
    {
        qDebug() << "找到了和" << printerModel << "精准匹配的PPD文件";
        qDebug() << "完全匹配的PPD文件名字为：" << ret[tempPrinterModel.size()];
    }

    return QPair<QMap<int, QStringList>, bool>(ret, exactMatchFlag);
}

QString MatchPPDsThread::originStringHandle(QString originString)
{
    if (originString.isEmpty())
    {
        qDebug() << "当前处理字符串为空字符串";
        return originString;
    }
    QString tempHandledString = nullptr;
    foreach (auto &letter, originString.toLower())
    {
        if (!letter.isLetterOrNumber())
        {
            tempHandledString.append(' ');
        }
        else
        {
            tempHandledString.append(letter);
            if (letter.isLetter() && (*(&letter + 1)).isDigit())
            {
                tempHandledString.append(' ');
            }
            else if (letter.isDigit() && (*(&letter + 1)).isLetter())
            {
                tempHandledString.append(' ');
            }
        }
    }

    return tempHandledString;
}
