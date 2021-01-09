#ifndef CUPSCONNECTION4PPDS_H
#define CUPSCONNECTION4PPDS_H

#include <sys/utsname.h>
#include <cups/cups.h>
#include <cups/http.h>
#include <QDebug>
#include <map>
#include <string>
#include <qstring.h>
#include <stdexcept>

class CupsConnection4PPDs
{
public:
    static CupsConnection4PPDs* getInstance();
    http_t *ppdRequestConnection;

protected:
    CupsConnection4PPDs();
private:
    const char *server;
    int ippPortNum;
    http_encryption_t encryptionType;


//    void CupsPpdsRequest();
};

#endif // CUPSCONNECTION4PPDS_H
