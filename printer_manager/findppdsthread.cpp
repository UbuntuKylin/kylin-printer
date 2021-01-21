#include "findppdsthread.h"

FindPPDsThread::FindPPDsThread(http_t* httpConnection, QObject *parent) : QObject(parent),newHttp(httpConnection)
{

}

void FindPPDsThread::initPPDMapConstruct()
{
    originPPDs = getPPDsFromCUPS();
    qDebug() << "PPDs got!";
    emit gotAllHandledPPDs(originPPDs);
}

QMap<QString,QMap<QString,PPDsAndAttr>> FindPPDsThread::getPPDsFromCUPS()
{
    ppdRequest = ippNewRequest(CUPS_GET_PPDS);

    cupsAnswer = cupsDoRequest(newHttp, ppdRequest, "/admin/");

    if (!cupsAnswer || ippGetStatusCode(cupsAnswer) > IPP_OK_CONFLICT)
    {
        qDebug() << "cupsAnswer request failed!";
    }
    else
    {
        qDebug() << "ippGetStatusCode(cupsAnswer) result is :" << ippGetStatusCode(cupsAnswer) ;
    }

    int i = 0;

    QMap<QString,QMap<QString,PPDsAndAttr>> ret;
    for (attr = ippFirstAttribute(cupsAnswer); attr; attr = ippNextAttribute(cupsAnswer))
    {
        const char *name = nullptr;

        name = ippGetName(attr);
        if(!name)
        {
            continue;
        }
        if (!strcmp(name, "ppd-name") && ippGetValueTag(attr) == IPP_TAG_NAME)
        {
            QString ppdname = nullptr;
            QString ppd_make_and_model = nullptr;
            QString ppd_device_id = nullptr;
            QString ppd_make = nullptr;
            QString ppd_product = nullptr;
            QString keyMFG = nullptr;

            ppdname = ippGetString(attr, 0, nullptr);
            PPDsAndAttr a;
            a.ppdname = ppdname;


            for (; attr && ippGetGroupTag(attr) == IPP_TAG_PRINTER;attr = ippNextAttribute(cupsAnswer))
            {
                const char *attrName = nullptr;

                attrName = ippGetName(attr);
                if(!attrName)
                {
                    break;
                }

                if(attrName && !strcmp(attrName, "ppd-make-and-model"))
                {
                    ppd_make_and_model = QString::fromLocal8Bit((char *)ippGetString(attr, 0, nullptr));

                    QStringList temp = {};
                    temp = ppd_make_and_model.split(' ');
                    keyMFG = temp[0].toLower();
                    for (int i = 1; i < temp.size(); ++i)
                    {
                        a.ppd_make_and_model.append(temp[i].toLower());
                    }

                }
                if(attrName && !strcmp(attrName, "ppd-device-id"))
                {
                    ppd_device_id = QString::fromLocal8Bit((char *)ippGetString(attr, 0, nullptr));
                    a.ppd_device_id = ppd_device_id;
                }
                if(attrName && !strcmp(attrName, "ppd-make"))
                {
                    ppd_make = QString::fromLocal8Bit((char *)ippGetString(attr, 0, nullptr));
                    a.ppd_make = ppd_make;
                }
                if(attrName && !strcmp(attrName, "ppd-product"))
                {
                    ppd_product = QString::fromLocal8Bit((char *)ippGetString(attr, 0, nullptr));
                    a.ppd_product = ppd_product;
                }
            }
            if(ret.find(keyMFG) != ret.end())
            {
                auto &mfg = ret[keyMFG];

                mfg.insert(a.ppd_make_and_model,a);
            }
            else
            {
                QMap<QString,PPDsAndAttr> tempMap ;
                tempMap.insert(a.ppd_make_and_model,a);
                ret.insert(keyMFG,tempMap);
            }


        }
    }
    if (cupsAnswer!=nullptr)
    {
        ippDelete(cupsAnswer);
    }
    
    return ret;
}
