# MyWebServer

## 项目参考

[linyacool/WebServer: A C++ High Performance Web Server (github.com)](https://github.com/linyacool/WebServer)

[cacay/MemoryPool: An easy to use and efficient memory pool allocator written in C++. (github.com)](https://github.com/cacay/MemoryPool)

Linux高性能服务器编程，游双著.

## 项目介绍：

### Linux下c++轻量级Web服务器。

技术特点：

* 参照muduo，使用双缓冲技术实现了Log系统,支持日志回滚

* 使用小根堆+unordered_map实现定时器队列，在此基础上进一步实现了长连接的处理

* 使用RAII机制封装锁，让线程更安全

* 采用Reactor模式+EPOLL(ET)非阻塞IO

* 利用哈希桶的思想构建内存池，提高服务器性能。

* 加入缓存机制(LFU)，借鉴LFU-Aging，解决缓存污染问题。

* 使用基于状态机的HTTP请求解析，较为优雅

* 使用了智能指针、bind、function、右值引用等一些c++11的新特性

  



## 服务器处理的基本流程

可以看出这是一种主从reactor多线程模式的服务器.（reactor模式也称为事件驱动）参考自muduo

![image-20231120152409377](C:\Users\16537\AppData\Roaming\Typora\typora-user-images\image-20231120152409377.png)



![image-20231120161827751](C:\Users\16537\AppData\Roaming\Typora\typora-user-images\image-20231120161827751.png)

主从多线程Reactor模式的服务器可以参考GitHub：[yuesong-feng/30dayMakeCppServer: 30天自制C++服务器，包含教程和源代码 (github.com)](https://github.com/yuesong-feng/30dayMakeCppServer)

这位大佬的项目学完可以看muduo库源码



现在给出框架


![image-20231120152409377](https://github.com/yzhao200910/MyWebServer/assets/128422499/cbfc65b1-d2d1-465e-ada0-167b645f0d90)



poll：30天自制服务器上有，这里拿Epoll；

## 日志系统：参考与muduo的日志系统

![image-20231120183122237](C:\Users\16537\AppData\Roaming\Typora\typora-user-images\image-20231120183122237.png)

服务器的⽇志系统是⼀个多⽣产者，单消费者的任务场景：多⽣产者负责把⽇志写⼊缓冲区，单消费者负责把缓冲

区中数据写⼊⽂件。如果只⽤⼀个缓冲区，不光要同步各个⽣产者,还要同步⽣产者和消费者。⽽且最重要的是需要

保证⽣产者与消费者的并发，也就是前端不断写⽇志到缓冲区的同时，后端可以把缓冲区写⼊⽂件。

分析各个日志相关文件：

我个人理解日志的核心是Impl类和Logstream类

Impl利用Logger构造函数的初始化列表，初始化，将时间信息存到 LogStream 的缓冲区⾥，在我们使用LOG时实际用的impl_.stream_的重载<<写入数据，也是到LogStream缓存区中   

LogStream类的实现：封装一块缓冲区，和运算符的重载

很多运算符充当的角色，都是将整型，浮点型转换成字符型，只有参数是字符型才调用下面函数，去将数据拷贝进缓冲区



AsyncLogging 类的实现:这个类中有fflush之类的函数，那么就是将之前LogStream写进AsyncLogging的缓冲区中，使用双缓冲区技术

currentBuffer_和nextBuffer_

双缓存技术的原理:两块缓存区，A与B,数据先写到A，此时B有数据就写入磁盘，没有等待A满了，两者相互交替

使用此技术的好处：两个缓冲区的好处时不用等写入磁盘的时间，数据写入A，也只会等A满了，才会与B交互，相当与批量处理，减少线程被唤醒的次数

## 内存池的实现

### 内存池参考来自于：

[cacay/MemoryPool: An easy to use and efficient memory pool allocator written in C++. (github.com)](https://github.com/cacay/MemoryPool)

### 设计内存池得原因：

通常使用malloc或者new进行动态内存管理得运行速度慢，有一定得内存开销，如果是单纯对大内存得对象分配堆区利用new或者malloc还是很可以的，但如果需要存储许多小对象，不停的去调用new/malloc，无疑时间和内存开销是高性能程序无法接受的。

### 内存池设计的思想：

内存池分配大块内存，并将内存分割成小块。每次请求内存时，都会返回其中一小块，而不是调用操作系统或堆分配器。只有事先知道对象的大小，才能使用内存池；这个是原理

根据原理怎么去设计呢，首先是我们得得到一个大内存，再将大内存分成等分大小的一块块小内存，每次为小内存的对象分配内存是前面大内存被分成小内存，我们将大内存称之为内存池，因为大内存是所有小内存的集合

最后只想那块被分成的一个个小内存（才疏学浅，表达不出来）

比如：一个memeory pool的大小是4096bytes, 没一个小块被等分的切割成8bytes.那所有小块的总和是4096

![image-20231127214347210](C:\Users\16537\AppData\Roaming\Typora\typora-user-images\image-20231127214347210.png)
