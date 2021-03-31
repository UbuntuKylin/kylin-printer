#ifndef __UKUI_PRINTER_H__
#define __UKUI_PRINTER_H__

#include <vector>
#include <string>

#include <cups/cups.h>

//Set properties enable,shared
enum class ukuiPrinterProperties : int
{
    UKUI_PRINTER_PROPERTY_START = 0, //NONE
    UKUI_PRINTER_NAME,               //string
    UKUI_PRINTER_DESCRIPTION,        //string
    UKUI_PRINTER_LOCATION,           //string
    UKUI_PRINTER_DRV_PPD,            //string
    UKUI_PRINTER_ENABLE,             //"true"/"false"
    UKUI_PRINTER_SHARED,             //"true"/"false"
};

struct ukuiUsbPrinter
{
    std::string name;//Destination name;
    std::string uri;//Printer Uri : like usb://xxxx/xxxx?serial=xxxx
    std::string prodect;//Printer Model
    std::string vendor;//Printer Manufactor 
    std::string ppdName;//PPD name in lpinfo -m

};


class ukuiPrinter
{
    //Meyers singleton
public:
    static ukuiPrinter &getInstance()
    {
        static ukuiPrinter instance;
        return instance;
    }

    bool connect();
    bool disconnect();
    //Set ppd and some properties to a specified printer
    bool addPrinter(const std::string &usbUri,
                    const std::string &printerName,
                    const std::string &ppdName,
                    const std::string &make_and_model);
    //Remove specified printer
    bool removePrinter(const std::string &printerName);
    bool printTestPage(const std::string &printerName, const std::string &fileName);
    void enumPrinters(std::vector<ukuiUsbPrinter> &printers);

    bool setOption(ukuiPrinterProperties type, std::string value);
    bool savePrinters();
    void setHostAndPort(std::string host, int port);

private:
    ukuiPrinter(/* args */);
    ukuiPrinter(ukuiPrinter const &);
    ukuiPrinter &operator=(ukuiPrinter const &);
    ~ukuiPrinter();

    void generateDefaultOptions();
private:
    ukuiPrinter *m_instance;
    std::string m_host;

    http_t* m_http;
    int m_port;
};
#endif //__UKUI_PRINTER_H__