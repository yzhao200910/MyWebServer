# MyWebServer

## 项目参考

[linyacool/WebServer: A C++ High Performance Web Server (github.com)](https://github.com/linyacool/WebServer)

[cacay/MemoryPool: An easy to use and efficient memory pool allocator written in C++. (github.com)](https://github.com/cacay/MemoryPool)

Linux高性能服务器编程，游双著.

## 项目介绍：

### Linux下c++轻量级Web服务器。

* 采用Epoll I/O复用技术，工作模式采用ET + 非堵塞I/O，增加对事件处理的响应速度
* 采用RAII机制和手法封装锁，避免手动加锁解锁，减少锁的Race，智能指针shared_ptr 和 unique_ptr,理内存，避免手动delete和可能存在的内存泄漏问题，如析构函数在跨线程环境下的不安全
* 采用线程池和Round-Robin算法，避免线程频繁创建和销毁开销以及高效地为子线程分发任务
* 对于日志系统，基于muduo采用并实现双缓存机制，自定义固定BUFFER并重载运算符，优化异步日志库写入的性能，并且实现日志分级和日志回滚功能 
* 设计CountDownLatch同步工具类，简化多线程编程的复杂性，使得线程间的协调和时序控制更加明确和易于管理，并且使服务器优雅的关闭：在停止服务器时，可以使用 CountDownLatch 确保所有的服务线程都已经处理完当前的工作，再进行资源的清理和释放。
* 采用c++11 std::move函数和移动语句来避免大量的不必要的拷贝动作
* 基于STL allocator 实现内存池，减少底层malloc的频繁使用，减少cookie的产生，提高内存的使用率。
* 加入缓存机制(LFU)，借鉴LFU-Aging，引入平均计数量，解决缓存污染问题。
  
##项目运行：
./build.sh
./WebServer/WebServer

## 服务器处理的基本流程

可以看出这是一种主从reactor多线程模式的服务器.（reactor模式也称为事件驱动）参考自muduo

![image-20231120152409377](https://github.com/yzhao200910/MyWebServer/assets/128422499/84e348ad-9ca7-4fd6-92e2-0f54370751fe)


![image-20231120161827751](https://github.com/yzhao200910/MyWebServer/assets/128422499/1db2bc5e-18ff-4c1c-b092-94b9c9efc0a1)


学习主从多线程Reactor模式的服务器基础可以参考GitHub：[yuesong-feng/30dayMakeCppServer: 30天自制C++服务器，包含教程和源代码 (github.com)](https://github.com/yuesong-feng/30dayMakeCppServer)

整体框架
![image-20231120164842082](https://github.com/yzhao200910/MyWebServer/assets/128422499/7530ace2-30c2-4483-80e5-0008d1448bf0)

## 日志系统：参考与muduo的日志系统
![image-20231120183122237](https://github.com/yzhao200910/MyWebServer/assets/128422499/d5e0637f-7b86-4cda-b014-2ebb0131fcf1)

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
![image-20231127214347210](https://github.com/yzhao200910/MyWebServer/assets/128422499/40200d9d-b9ec-458b-bb28-a804e097f22a)


