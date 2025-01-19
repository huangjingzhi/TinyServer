## TinyServer

### 简介
Linux下轻量级通用网络服务框架，在此框架下可以快速实现一个高并发网络服务。
- 网络服务基础框架：基于epoll + LT模式，one thread on loop 模型实现网络通用服务框架。
- 自定义网络服务：通过继承App类，重载虚函数接口实现自己的业务逻辑。
- 基于该网络服务框架，已经实现简单的http服务，tlv消息服务。

### 代码
---

> 网络服务基础框架

<div align=center><img src="./docs/readme/服务端框架.png" height="201"/> </div>

<div align=center><img src="./img/服务端框架.png" height="201"/> </div>

    网络服务框架由侦听线程和IO线程组构成。侦听线程处理新连接，将连接分配给线程组中的线程处理，IO线程组处理连接上的IO事件。
    IO线程：每个IO线程创建一个epoll实例，管理多个连接上的IO事件。数据流主要是由连接的读事件产生，通过应用层的业务逻辑解析数据流，产生业务消息，将业务消息再次通过业务逻辑处理，产生响应数据流，通过连接的写事件发送出去。

> 网络应用服务

- 网络应用服务解析数据流的逻辑通过重载CHannel类中的虚函数，即可自定义网络数据解析和发送逻辑。主要重载的函数如下：
```c++
class Channel
{
    ...
public:
    ...
    virtual ChannelHandleResult HandleSocketRead();  // 读取和解析网络数据接口，产生业务消息
    virtual ChannelHandleResult HandleSocketWrite(); // 网络数据发送接口
    virtual ChannelHandleResult HandleSocketError(); // 网络错误处理接口
};
```
- 网络应用服务处理业务逻辑通过重载App类中的虚函数，即可自定义业务消息处理逻辑。当连接上产生业务消息时，框架内会调用该函数处理业务消息。
```c++
class App
{
    ...
    virtual void Update(Channel *channel); // 业务消息处理接口
};
```

> 代码目录简介
```shell
.
├── src
│   ├── app         # 应用层代码
│   │   ├── http    # 应用层http服务代码
│   │   └── tlvmsg  # 应用层tlv消息服务代码
│   ├── commom      # 公共代码：定时器，日志，内存等
│   ├── kernel      # 网络服务核心代码
│   │   ├── App.cpp
│   │   ├── App.h
│   │   ├── Channel.cpp
│   │   ├── Channel.h
│   │   ├── ConnectManage.cpp
│   │   ├── ConnectManage.h
│   │   ├── NetIoManage.cpp
│   │   ├── NetIoManage.h
│   │   ├── NetIoWorker.cpp
│   │   └── NetIoWorker.h
└── webbench-1.5    # 压测工具 
```

### http服务
---
> 实现了简单的Http服务，支持响应视频，图片，文本等文件。

代码目录：src/app/http
- HttpChannel：继承自Channel类，实现了Http协议的解析和发送。并调用HttpServer重载App的接口，处理业务消息。
```c++
    virtual ChannelHandleResult HandleSocketRead();  // 重载：读取IO数据，传递给request，让request解析
    virtual ChannelHandleResult HandleSocketWrite(); // 重载：根据respond内容，发送数据
    virtual ChannelHandleResult HandleSocketError(); // 不重载
```
- HttpServer：继承自App类，实现了Http服务的业务逻辑。重载了App类的虚函数，处理Request请求，并产生Response响应。
```c++
    virtual void Update(Channel *channel); // 重载：根据request请求，执行对应的请求处理方式，构建response
```
- HttpRequest：Http请求类，解析Http请求报文。
- HttpResponse：Http响应类，生成Http响应报文，支持响应文本，图片，视频等文件。

### http服务效果：

>视频播放

<div align=center><img src="./img/video.gif" height="201"/> </div>

>图片
<div align=center><img src="./img/images4.gif" height="201"/> </div>

