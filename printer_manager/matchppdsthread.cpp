#include "matchppdsthread.h"

MatchPPDsThread::MatchPPDsThread(QObject *parent) : QObject(parent)
{
}

void MatchPPDsThread::initPPDMatch(QString bandName, QString printerName, myMap data, int type)
{
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
        qDebug() << "查询的打印机厂商名为" << printerBandName;
        qDebug() << "查询的打印机型号名为" << printerModelName;
    }

    if (originData.find(printerBandName) != originData.end())
    {
        qDebug() << printerBandName << "厂商已存在于总map中";
        auto &mfg = originData[printerBandName];
        qDebug() << printerBandName << "对应的型号map大小为" << mfg.size();

        matchResult = eactMatch2(printerModelName, mfg, type);

        emit matchResultSignal(matchResult);
    }
    else
    {
        qDebug() << printerBandName << "厂商尚未存在于总map中";
        qDebug() << "目前总map中共有：" << originData.size() << "种厂商";
        emit matchFailed();
    }
}

QStringList MatchPPDsThread::eactMatch1(QString printerModel, QMap<QString, PPDsAndAttr> map)
{

    QMap<int, QString> tempPPDs;
    qDebug() << printerModel;
    QStringList printerModelList = printerModel.split(' ');
    if (printerModelList.size() <= 2)
    {
        emit matchFailed();
        return printerModelList;
    }
    tempPPDs.clear();
    QStringList ret;
    ret.clear();
    foreach (auto keyPPDs, map.keys())
    {
        qDebug() << keyPPDs << "对应的make和model为：" << map[keyPPDs].ppd_make_and_model;
        int i = 0;
        foreach (auto modelName, printerModelList)
        {
            if (modelName.isEmpty())
            {
                continue;
            }
            if (map[keyPPDs].ppd_make_and_model.contains(modelName, Qt::CaseInsensitive))
            {
                i++;
            }
        }
        if (i != 0)
        {
            if (tempPPDs.find(i) == tempPPDs.end())
            {
                tempPPDs.insert(i, keyPPDs);
            }
        }
    }
    if (tempPPDs.size())
    {

        QMap<int, QString>::iterator iter;
        iter = tempPPDs.end();
        for (int n = 0; n < 3; n++)
        {
            if (iter != tempPPDs.begin())
            {
                iter--;
                qDebug() << iter.value();
                ret.append(iter.value());
            }
            else
            {
                break;
            }
        }
        qDebug() << ret;
    }
    else
    {
        qDebug() << "没找到，还得模糊匹配";
    }
    return ret;
}

QPair<QMap<int, QStringList>, bool> MatchPPDsThread::eactMatch2(QString printerModel, QMap<QString, PPDsAndAttr> map, int type)
{
    exactMatchFlag = false;
    QMap<int, QString> tempPPDs;
    tempPPDs.clear();

    qDebug() << printerModel;

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
            //qInfo() << "字符串中的空格，不处理！";
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
                qDebug() << "找到通用免驱文件，不返回";
                qDebug() << "当前比对PPD文件名为：" << map[keyPPDs].ppdname;
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
                    qDebug() << "找到和" << printerModel << "精准匹配的PPD文件" << keyPPDs;
                    qDebug() << "该PPD文件的型号内容参数原字串为：" << map[keyPPDs].ppd_make_and_model;
                    qDebug() << "该PPD文件的型号内容参数处理字串为：" << handledPPDMakeAndModel;
                    qDebug() << "匹配字符串为：" << currentStr;
                    qDebug() << "匹配位数为：" << i;
                    qDebug() << "********************************************************************";

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
    //    QPair<QMap<int,QStringList>,bool> tempResult = {};
    if (!exactMatchFlag)
    {
        qDebug() << "没找到和" << printerModel << "精准匹配的PPD文件";
        foreach (auto it, ret.keys())
        {
            qDebug() << "找到匹配" << it << "个字符的PPD文件" << ret[it].size() << "个";
        }
        return QPair<QMap<int, QStringList>, bool>(ret, false);
    }
    else
    {
        qDebug() << "找到了和" << printerModel << "精准匹配的PPD文件";
        qDebug() << "完全匹配的PPD文件名字为：" << ret[tempPrinterModel.size()];
        return QPair<QMap<int, QStringList>, bool>(ret, true);
    }
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
