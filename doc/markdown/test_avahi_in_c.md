---
export_on_save:
html: true
---
<font face="Sarasa Mono HC">

# test_avahi_in_c

## 这是什么?
这是一个参考`https://avahi.org/doxygen/v0.7/html/`提供的例子编写的自动检测网络设备demo。；可以自动发现并获取网络中使用zeroconf协议的网络设备。

## 如何运行？
mkdir build  
cd build  
cmake ..  
make  
./NetworkPrinterDetect

## 为什么弃用了这个接口？
可以看一下文件夹中的代码：
为了实现获取到avahi扫描出的设备信息，
调用了一个事件主循环，
起了一个客户端，两个服务端，
每个服务都需要起一个回调函数，
而且调用这个方法的人，如果想即时处理收到的信息，还需要在写一层回调函数，实在不是一种优美的解决方案。

</font>