---
title: muduo中的高性能异步日志库
date: 2019-09-28 15:50:39
categories: ycnt
tags: [muduo,ycnt]
---

# muduo高性能日志学习笔记

----------------

#### 功能

1. 日志库一般可以分为前端和后端，前端提供API并生成日志消息，后端负责把日志消息写到目的地
2. 常见的功能：多级别(level)，多目的地(appender)，可配格式(layout)，过滤(filter)，滚动(rolling)
3. 时间戳精确到微秒，每条日志一行，打印线程id、日志级别、源文件名和行号
4. 日志过多时进行丢弃，确保不会因为日志模块影响整个系统的稳定性

#### 性能

1. 降低磁盘I/O频率，日志首先写到缓冲区，定期flush到磁盘，对于未写入磁盘的内存中日志带有cookie，确保core dump时也能分析(`gdb->find`,`strings`)
2. 时间戳通过`gettimeofday`获得，非系统调用，并且日期和时间进行缓存，若一秒内多条日志，则只需要重新格式化微妙部分即可

    ```
    2019.09.26 20:23:05.426379
    2019.09.26 20.23.05.789234
    2019.09.26 20.23.05.999123
    |------cached-----| |----|
    ```

3. 线程id也在一开始进行格式化并缓存，输出时直接复制已有的数据
4. 文件名、行号采用gcc内置函数在编译期获得
5. 双缓存技术：
    
    1. 准备A, B两块缓存，前端向A写数据，后端将B写入文件
    2. A写满时，交换A和B，如此往复
    
    **muduo中实际采用了四缓存设计**



# ycnt的修改

----------------

#### 与muduo的差异

1. 前端基本保持一致，使用了[Grisu2](https://github.com/miloyip/dtoa-benchmark)替代了`snprintf`进行浮点数格式化
2. 后端由于muduo中使用一个全局锁，在ycnt中改为了concurrent hash map的做法，这样会导致日志中不同线程日志会出现时间交叉，需要日志分析工具对时间进行重新排序整合分析
3. ycnt的日志库基本结构如下：

    ```
    0.basic
    
       thread 0,1,2...-hash-------------------->
                         |----->               |
                               |               |
                               |               |
                     <----- [***  ]         [**   ] 
                     |      bucket 0        bucket 1 ...
                     |            \         /
                     |            fixedBuffers <--------------------<
                     |                                              |
                     |                                              |
       <------[full] [full] ...                                     |
       |       frontBuffers                                         |
       |                                                            |
       |                                                            |
       >------[full] [full] ...------> back-end writer ----> [empty] [empty] ...
               backBuffers                   |                  bufferPool
                                          LogFile
                                             |
                                   <---- appendFile <----------new file
                                   |
                                   |
                             archived file         
    
    1.fixedBuffer full
    
        thread     [0,4,...] ...
                       |
                       |(1)add full to front, get empty from pool
                       |
     fixedBuffers  <-[   ]------------<
                   |                  |
                   |(2)notify writer  |
                   |                  |
     frontBuffers [full]              |
                                      |
       bufferPool  ... [empty] ------->
    
    
    2.back writer wake up (notified or timedout)
    
                          fixedBuffers  [   ]<-<(2)get empty from pool
                                          |    |
     frontBuffers  [full] [full] ... <----<(1)directly add all non-empty to front
           |(3)add front to back               |
      backBuffers  []                          |
           |                                   |
           |        >-->bufferPool ... [empty]->
           |        |(5)add empty to pool
           >--->[logFile](4)write back to logFile
    ```

#### 性能测试

只在WSL下进行过测试，没有经过profile，不够严谨，仅作为参考

参考测试程序在ycnt/test/bench/Logging_bench.cpp中

<img src="https://jasonyuchen.github.io/2019/09/28/async-logging/async_logging_bench.png" height="400" width="651">
