---
export_on_save:
html: true
---
<font face="Sarasa Mono HC">

# snmpwalk 获取设备信息

## 这是什么？
snmpwalk 是一个 SNMP 应用程序，它使用 SNMP GETNEXT 请求来查询网络实体以获取信息树。 

## 命令
`snmpwalk -v 2c -c public $host $oid`

## 打印机设备相关oid查询:

| description | object Name | OID | value in Brother DCP-7195DW |
| :-----   | :-----   | :----- | :----- |
| ip |  ipAdEntAddr | 1.3.6.1.2.1.4.20.1.1 | iso.3.6.1.2.1.4.20.1.1.192.168.17.90 = IpAddress: 192.168.17.90 |
| netmask | ipAdEntNetMask | 1.3.6.1.2.1.4.20.1.3 | iso.3.6.1.2.1.4.20.1.3.192.168.17.90 = IpAddress: 255.255.255.0 |
| network node name | sysName | 1.3.6.1.2.1.1.5 | iso.3.6.1.2.1.1.5.0 = STRING: "PRINTERFORTEST" |
| vendor name | prtOutputVendorName | 1.3.6.1.2.1.43.9.2.1.8 | iso.3.6.1.2.1.43.9.2.1.8.1.1 = STRING: "Brother" |
| vendor and model | hrDeviceDescr | 1.3.6.1.2.1.25.3.2.1.3| iso.3.6.1.2.1.25.3.2.1.3.1 = STRING: "Brother DCP-7195DW" |
| serial | prtGeneralSerialNumber | 1.3.6.1.2.1.43.5.1.1.17 | iso.3.6.1.2.1.43.5.1.1.17.1 = STRING: "E78133F0N443169" |

## 关于打印机的其他oid信息获取：
oid 1.3.6.1.4.1.x标识了打印机的厂商，可以在iana.org查询，例如Brother打印机的 编号就是2435。
即1.3.6.1.4.1.2435下的可以由Brother任意编辑,其他信息可以从这里面获取。

## 参考文档及网址：
https://oid-info.com/get/1.3.6.1.4.1
https://www.iana.org/assignments/enterprise-numbers/enterprise-numbers
https://bestmonitoringtools.com/mibdb/mibdb_search.php  
https://iphostmonitor.com/mib/Printer-MIB.html
https://www.rfc-editor.org/rfc/rfc1213.html
https://www.rfc-editor.org/rfc/rfc1514.html
https://www.rfc-editor.org/rfc/rfc1907.html
https://www.rfc-editor.org/rfc/rfc2011.html

</font>