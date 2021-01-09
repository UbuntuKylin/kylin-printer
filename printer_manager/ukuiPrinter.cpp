#include <iostream>
#include <unistd.h>
#include "cupsconnection4ppds.h"
#include "ukuiPrinter.h"

//#define g_cupsConnection (CupsConnection4PPDs::getInstance()) 
const char *baseIppUriFormat = "ipp://%s:%d/printers/";

//Printer Destination Callback from CUPS example.
static int enum_cb(void *user_data, unsigned flags, cups_dest_t *dest)
{
    int i; /* Looping var */
    (void)user_data;
    (void)flags;
    if (dest->instance)
        printf("%s%s/%s%s:\n", (flags & CUPS_DEST_FLAGS_REMOVED) ? "REMOVE " : "", dest->name, dest->instance, dest->is_default ? " (Default)" : "");
    else
        printf("%s%s%s:\n", (flags & CUPS_DEST_FLAGS_REMOVED) ? "REMOVE " : "", dest->name, dest->is_default ? " (Default)" : "");

    for (i = 0; i < dest->num_options; i++)
        printf("    %s=\"%s\"\n", dest->options[i].name, dest->options[i].value);

    return (1);
}
//Print destination properties
void printDest(cups_dest_t dest)
{
    std::cout << dest.name << std::endl;
    for (auto i = 0; i < dest.num_options; i++)
    {
        /* code */
        std::cout << "    " << dest.options[i].name << ":" << dest.options[i].value << std::endl;
    }
}
//canonicalize uri
static void construct_uri(char *buffer, size_t buflen, const char *base, const char *value)
{
    char *d = buffer;
    const unsigned char *s = (const unsigned char *)value;
    if (strlen(base) < buflen)
    {
        strcpy(buffer, base);
        d += strlen(base);
    }
    else
    {
        strncpy(buffer, base, buflen);
        d += buflen;
    }

    while (*s && d < buffer + buflen)
    {
        if (isalpha(*s) || isdigit(*s) || *s == '-')
            *d++ = *s++;
        else if (*s == ' ')
        {
            *d++ = '+';
            s++;
        }
        else
        {
            if (d + 2 < buffer + buflen)
            {
                *d++ = '%';
                *d++ = "0123456789ABCDEF"[((*s) & 0xf0) >> 4];
                *d++ = "0123456789ABCDEF"[((*s) & 0x0f)];
                s++;
            }
            else
            {
                break;
            }
        }
    }

    if (d < buffer + buflen)
        *d = '\0';
}

//Add Printer,use ipp request
bool ukuiPrinter::addPrinter(const std::string &usbUri, const std::string &printerName, const std::string &ppdName, const std::string &make_and_model)
{
    //
    ipp_t *request = ippNewRequest(CUPS_ADD_MODIFY_PRINTER);
    ipp_t *answer = nullptr;
    char uri[HTTP_MAX_URI];
    char baseUri[128];
    sprintf(baseUri, baseIppUriFormat, m_host.c_str(), m_port);
    construct_uri(uri, HTTP_MAX_URI, baseUri, printerName.c_str());
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", nullptr, uri);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME, "ppd-name", nullptr, ppdName.c_str());
    ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_URI, "device-uri", nullptr, usbUri.c_str());
    ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_TEXT, "printer-info", nullptr, printerName.c_str());
    std::cout << "Uri:" << uri << std::endl;
    std::cout << "Ppd:" << ppdName << std::endl;
    std::cout << "usbUri:" << usbUri << std::endl;
    answer = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/");
    if (answer == nullptr)
    {
        printf("cupsDoRequest Failed.\n");
        ipp_status_t err = cupsLastError();
        if (err == ipp_status_t::IPP_STATUS_ERROR_CUPS_AUTHENTICATION_CANCELED)
        {
            int ret = cupsDoAuthentication(m_http,"POST","/admin/");
            std::cout << ret <<std::endl;
        }
        
        
        std::cout << "ERROR:" << cupsLastErrorString() << std::endl;
        return false;
    }
    ipp_status_t status = ippGetStatusCode(answer);
    if (status > IPP_OK_CONFLICT)
    {
        printf("Error:%s\n", cupsLastErrorString());
        return false;
    }
    ippDelete(answer);
    answer = nullptr;

    request = ippNewRequest(IPP_ENABLE_PRINTER);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", nullptr, uri);
    answer = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/");
    if (answer != nullptr)
    {
        /* code */
        ippDelete(answer);
        answer = nullptr;
    }

    request = ippNewRequest(IPP_RESUME_PRINTER);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", nullptr, uri);
    answer = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/");
    if (answer != nullptr)
    {
        /* code */
        ippDelete(answer);
        answer = nullptr;
    }

    return true;
}

bool ukuiPrinter::removePrinter(const std::string &printerName)
{
    ipp_t *request = ippNewRequest(CUPS_DELETE_PRINTER);
    ipp_t *answer = nullptr;
    char uri[HTTP_MAX_URI];
    char baseUri[128];
    sprintf(baseUri, baseIppUriFormat, m_host.c_str(), m_port);
    construct_uri(uri, HTTP_MAX_URI, baseUri, printerName.c_str());
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", nullptr, uri);
    answer = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/");
    if (answer == nullptr)
    {
        printf("cupsDoRequest Failed.\n");
        return false;
    }
    ipp_status_t status = ippGetStatusCode(answer);
    if (status > IPP_OK_CONFLICT)
    {
        printf("Error:%s\n", cupsLastErrorString());
        ippDelete(answer);
        return false;
    }
    ippDelete(answer);
    return true;
}

bool ukuiPrinter::printTestPage(const std::string &printerName, const std::string &fileName)
{
    cups_dest_t *allDests = nullptr;
    cups_dest_t currentDest;
    int destCount;
    cups_file_t *fp = cupsFileOpen(fileName.c_str(), "r");
    if (fp == NULL)
    {
        /* code */
        printf("Failed to print test page: open printing file failed\n");
        return false;
    }
    destCount = cupsGetDests(&allDests);
    if (destCount == 0)
    {
        /* code */
        printf("No available printer.\n");
        return false;
    }
    for (size_t i = 0; i < destCount; i++)
    {
        /* code */
        printDest(allDests[i]);
        if (printerName.compare(allDests[i].name) == 0)
        {
            currentDest = allDests[i];
        }
    }
    printf("%s\n", printerName.c_str());

    http_t *http;        /* Connection to destination */
    cups_dinfo_t *dinfo; /* Destination info */
    unsigned dflags = 0;
    ssize_t bytes;
    char buffer[32768];
    http = cupsConnectDest(&currentDest, dflags, 30000, NULL, NULL, 0, NULL, NULL);
    if (http == NULL)
        return false;

    dinfo = cupsCopyDestInfo(http, &currentDest);
    if (dinfo == NULL)
        return false;

    const char *title;
    if ((title = strrchr(fileName.c_str(), '/')) != NULL)
        title++;
    else
        title = fileName.c_str();

    int job_id;
    int num_options = 0;
    cups_option_t *options = nullptr;
    if (cupsCreateDestJob(http, &currentDest,
                          dinfo,
                          &job_id,
                          title,
                          num_options,
                          options) > IPP_STATUS_OK_IGNORED_OR_SUBSTITUTED)
    {
        /* code */
        cupsFileClose(fp);
        return false;
    }
    if (cupsStartDestDocument(http, &currentDest,
                              dinfo, job_id, title,
                              CUPS_FORMAT_AUTO,
                              0,
                              NULL,
                              1) != HTTP_STATUS_CONTINUE)
    {
        printf("Unable to send document: %s\n", cupsLastErrorString());
        cupsFileClose(fp);
        return false;
    }

    while ((bytes = cupsFileRead(fp, buffer, sizeof(buffer))) > 0)
    {
        std::cout << "Cups File Read " << bytes << " bytes." << std::endl;
        if (cupsWriteRequestData(http, buffer, (size_t)bytes) != HTTP_STATUS_CONTINUE)
        {
            printf("Unable to write document data: %s\n", cupsLastErrorString());
            break;
        }
    }

    cupsFileClose(fp);

    if (cupsFinishDestDocument(http, const_cast<cups_dest_t *>(&currentDest), dinfo) >
        IPP_STATUS_OK_IGNORED_OR_SUBSTITUTED)
    {
        printf("Unable to send document: %s\n", cupsLastErrorString());
        return false;
    }
    puts("Job queued.");
printTestEnd:
    if (dinfo != nullptr)
    {
        /* code */
        cupsFreeDestInfo(dinfo);
    }

    if (allDests != nullptr)
    {
        /* code */
        cupsFreeDests(destCount, allDests);
    }
    httpClose(http);
    //Free
    return true;
}

void ukuiPrinter::enumPrinters(std::vector<ukuiUsbPrinter> &printers)
{
    //
    cups_dest_t *dests; /* Destinations */
    int num_dests = cupsGetDests(&dests);
    for (auto i = 0; i < num_dests; i++)
    {
        ukuiUsbPrinter printer;
        printer.name = dests[i].name;
        const char *deviceUri = cupsGetOption("printer-uri", dests[i].num_options, dests[i].options);
        if (deviceUri != nullptr)
        {
            printer.uri = deviceUri;
        }
        printers.push_back(printer);
    }
}

ukuiPrinter::ukuiPrinter() : m_host("127.0.0.1"),
                             m_port(631),
                             m_instance(nullptr),
                             m_http(nullptr)
{
    std::cout << "ukuiPrinter Create.." << std::endl;
}

ukuiPrinter::~ukuiPrinter(/* args */)
{
    std::cout << "ukuiPrinter destroy.." << std::endl;
}