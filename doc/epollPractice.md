# epoll编程实践

#### 基本操作

#### 注意点

1. 同一个epoll fd不能自己监听自己
2. POLLERR和POLLHUP事件内核总是会关心，参考epoll源码阅读
3. 内核保证与epoll fd相关的操作(epoll_ctl, epoll_wait)是线程安全的

#### epoll fd自身也可以被另一个epoll fd监听

测试可见，sub epoll超时并不会触发main epoll的返回，只有sub epoll触发读写事件，才会作为一个事件触发main epoll返回

```C++
int subEfd = ::epoll_create1(EPOLL_CLOEXEC);
int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
struct epoll_event subEvent;
subEvent.events = EPOLLIN;
::epoll_ctl(subEfd, EPOLL_CTL_ADD, evtfd, &subEvent);

int efd = ::epoll_create1(EPOLL_CLOEXEC);
struct epoll_event event;
event.events = EPOLLIN;
::epoll_ctl(efd, EPOLL_CTL_ADD, subEfd, &event);

Thread thread([&]{
    struct epoll_event e;
    int num = ::epoll_wait(subEfd, &e, 1, 1000);
    std::cout << "sub epoll returned " << num << std::endl;
});
thread.start();

//::read(evtfd, NULL, 0);  <------

struct epoll_event e;
int num = ::epoll_wait(efd, &e, 1, 2000);
thread.join();
std::cout << "epoll returned " << num << std::endl;
```

是否`::read`分别输出结果如下

```
result with read:
sub epoll returned 1
epoll returned 1

resutl without read:
sub epoll returned 0
epoll returned 0
```
