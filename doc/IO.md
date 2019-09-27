# 多线程IO模型

|方案|模型|多进程|多线程|阻塞|复用|长连接|并发性|多核|开销|互通性|顺序性|线程数|特点|
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
|0|accept+read/write| | |&radic;| | |无| |低| |&radic;|固定|one client every time|
|1|accept+fork|&radic;| |&radic;| | &radic;|低|&radic;|高| |&radic;||变化|one process per client|
|2|accept+thread| |&radic;|&radic;| |&radic;|中|&radic;|中|&radic;|&radic;|变化|one thread per client|
|3|prefork|&radic;| |&radic;| |&radic;|低|&radic;|高| |&radic;|变化|
|4|prethreaded| |&radic;|&radic;| |&radic;|中|&radic;|中|&radic;|&radic;|变化|
|5|reactor| | | |&radic;|&radic;|高| |低|&radic;|&radic;|固定|单线程reactor|
|6|reactor+thread per task| |&radic;| |&radic;|&radic;|中|&radic;|中|&radic;| |变化|one thread per request|
|7|reactor+worker thread| |&radic;| |&radic;|&radic;|中|&radic;|中|&radic;|&radic;|变化|one worker thread per connectio|
|8|reactor+thread pool| |&radic;| |&radic;|&radic;|高|&radic;|低|&radic;|否|固定|主线程IO工作线程计算|
|9|reactors in threads| |&radic;| |&radic;|&radic;|高|&radic;|低|&radic;|&radic;|固定|one loop per thread|
|10|reactors in processes|&radic;| | |&radic;|&radic;|高|&radic;|低| |&radic;|固定|one loop per process (Nginx)|
|11|reactors+thread pool| |&radic;| |&radic;|&radic;|高|&radic;|低|&radic;|否|固定|最灵活(Netty)|

- 互通性：数据在多个连接之间是否方便直接交换
- 顺序性：同一个连接的请求，不刻意同步的情况下，是否满足先请求先响应的顺序

1. 方案2：原始，但客户不多的情况下完全适用，编程简单
2. 方案5：基本的单线程reactor方案，适合IO密集但CPU不密集的应用，较难发挥多核的威力，通过注册回调，将网络部分与业务逻辑分离，代码的扩展性和可维护性高
3. 方案8：网络部分与业务逻辑分离，线程池充分发挥多核的威力，但计算是线程池异步完成的，延迟比IO线程直接计算要高
4. 方案9：muduo的方案，一个main reactor专门负责accept连接，随后所有读写都放到sub reactor中完成
5. 方案11：在方案8的基础上通过multi reactors进一步加强系统的IO处理能力，最为灵活

**根据ZeroMQ手册给出的建议，event loop / (1024Mbits/s)的配置较为合理，即在千兆以太网上的程序只需要一个reactor即可（若TCP连接之间有优先级，那么单个reactor可能会导致优先级反转，因此非同质连接要用multi reactors来处理）**