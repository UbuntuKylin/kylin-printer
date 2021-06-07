---
export_on_save:
html: true
---
<font face="Sarasa Mono HC">

# udp 广播最小系统

## 这是什么?
这是一个测试udp广播功能的demo  
实现client无需知道server地址，只要保证在同一个网段下，可以将消息发送给server

## 如何运行?
`make`
`./server` 开启监听udp广播端口
`./client` 发送udp广播消息

* 每运行一次client,server端便会收到一条来自client的广播消息。
* 问题：
  * 如果没有收到消息，可能是`9999`端口没有打开，可以使用`iptables -A INPUT -p tcp --dport 9999 -j ACCEPT`打开；
  * v10sp1在第一次获取广播消息的时候，可能无法正确的获取到ip地址，下一次就会获取到正确的地址。（这个问题在ubuntu20.04上已经修复了）

## 原理简单分析
udp广播通信的原理很简单，只要向广播地址发送数据包，广播地址便会将消息发送给网络内的所有主机。  
广播地址可以通过ip和子网掩码计算得出：  
`broadcast = (ip & netmask)|~netmask`

## UDP Socket的使用过程：
* 初始化网络库
* 创建SOCK_DGRAM类型的Socket。
* 绑定套接字。
* 发送、接收数据。
* 销毁套接字。
* 释放网络库。 


</font>