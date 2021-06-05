---
export_on_save:
html: true
---
<font face="Sarasa Mono HC">

# avahi-qt

## 这是什么？
一个使用Qt的方法封装的avahi-watch主事件循环。
使用此接口，可以方便的在Qt中调用avahi的方法获取网络设备信息。

## 为什么需要将源码移植到打印机项目中，而不是直接调用库？
参考这个连接
[874832](https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=874832)
这个bug中提到，kde社区由于要从qt4升级到qt5，而avahi中使用的是qt4，故而将avahi-qt从包列表中移除。
如果想在v10sp1中调用这个库，需要将avahi-qt从qt4升到qt5。这个工作现在已经完成了。

我们现在在可以直接在qt5中调用这个库了。

</font>