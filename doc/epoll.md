# epoll源码阅读

> 参考 https://www.nowcoder.com/discuss/26226?type=all&order=time&pos=&page=1

#### 内核预备知识

1. 等待队列(waitqueue)

    队列头(wait_queue_head_t)往往是资源生产者，队列成员(wait_queue_t)往往是资源消费者，当头的资源ready后，会逐个执行每个成员指定的回调函数，来通知它们资源已经ready了
    
2. 内核poll

    1. 被poll的fd必须实现file_operations中的poll操作， 给自己分配有一个等待队列头
    2. 主动poll这个fd的某个进程必须分配一个等待队列成员，添加到fd的等待队列里面，并指定ready时的回调函数

3. 其他相关

    1. fd在内核中对应`struct file`
    2. 自旋锁spinlock，对应操作`spin_lock_irqsave()`等，会改变中断状态，参考
    3. 引用计数，在变为0时可以清理资源且不用加锁，参考C++ shared_ptr，`struct file`持有引用计数

4. epoll调用关系(in progress)

5. epoll源码

```C
/* This structure is stored inside the "private_data" member of the file
 * structure and rapresent the main data sructure for the eventpoll
 * interface.
 * 每创建一个epollfd, 内核就会分配一个eventpoll与之对应, 可以说是内核态的epollfd */
struct eventpoll {

    /* Protect the this structure access
     * 自旋锁，保护eventpoll */
    spinlock_t lock;

    /* This mutex is used to ensure that files are not removed
     * while epoll is using them. This is held during the event
     * collection loop, the file cleanup path, the epoll file exit
     * code and the ctl operations.
     * 添加, 修改或者删除监听fd的时候, 以及epoll_wait返回, 向用户空间
     * 传递数据时都会持有这个互斥锁, 所以在用户空间可以放心的在多个线程
     * 中同时执行epoll相关的操作, 内核级已经做了保护 */
    struct mutex mtx;

    /* Wait queue used by sys_epoll_wait()
     * 调用epoll_wait()时, 调用者就是等待在了这个等待队列上 */
    wait_queue_head_t wq;

    /* Wait queue used by file->poll() 
     * 这个用于epollfd本身被poll的时候 */
    wait_queue_head_t poll_wait;

    /* List of ready file descriptors
     * 所有已经ready的监听fd即epitem都在这个链表里面 */
    struct list_head rdllist;

    /* RB tree root used to store monitored fd structs
     * 红黑树保存所有epitem */
    struct rb_root rbr;

    /* This is a single linked list that chains all the "struct epitem" that
     * happened while transfering ready events to userspace w/out
     * holding ->lock.
     * 当目前的event转移到用户空间时又有回调触发，由这个单链表ovflist(overflow list)链接着
     * 此时触发的所有epitem，在下一次返回 */
    struct epitem *ovflist;

    /* The user that created the eventpoll descriptor
     * 这里保存了一些用户变量, 比如fd监听数量的最大值等等 */
    struct user_struct *user;
};

/* Each file descriptor added to the eventpoll interface will
 * have an entry of this type linked to the "rbr" RB tree.
 * epitem 表示一个被监听的fd */
struct epitem {

    /* RB tree node used to link this structure to the eventpoll RB tree
     * 当使用epoll_ctl()将一批fd加入到某个epollfd时, 内核会分配一批的epitem与fd相对应, 
     * 而且它们以rb_tree的形式组织起来，这里的rbn就代表自身这个epitem,
     * tree的root也就是struct eventpoll中的struct rb_root rbr */
    struct rb_node rbn;

    /* List header used to link this structure to the eventpoll ready list
     * 链表节点, 用来将自身eptiem在ready时链接到struct eventpoll中的rdllist
     * 所有已经ready的epitem都会被链到eventpoll的rdllist中 */
    struct list_head rdllink;

    /* Works together "struct eventpoll"->ovflist in keeping the
     * single linked chain of items.
     * 这个在代码中再解释 */
    struct epitem *next;

    /* The file descriptor information this item refers to
     * epitem对应的fd和struct file 
     *      struct epoll_filefd {
     *          struct file *file;
     *          int fd;
     *      };*/
    struct epoll_filefd ffd;

    /* Number of active wait queue attached to poll operations */
    int nwait;

    /* List containing poll wait queues */
    struct list_head pwqlist;

    /* The "container" of this item
     * 此epitem所属的eventpoll */
    struct eventpoll *ep;

    /* List header used to link this item to the "struct file" items list */
    struct list_head fllink;

    /* The structure that describe the interested events and the source fd
     * 当前的epitem关心的events, 调用epoll_ctl时从用户态传递过来 */
    struct epoll_event event;
};

/* Wait structure used by the poll hooks */
struct eppoll_entry {

    /* List header used to link this structure to the "struct epitem" */
    struct list_head llink;

    /* The "base" pointer is set to the container "struct epitem" */
    struct epitem *base;

    /* Wait queue item that will be linked to the target file wait queue head. */
    wait_queue_t wait;

    /* The wait queue head that linked the "wait" wait queue item */
    wait_queue_head_t *whead;
};

/* Wrapper struct used by poll queueing */
struct ep_pqueue {
    poll_table pt;
    struct epitem *epi;
};

/* Used by the ep_send_events() function as callback private data */
struct ep_send_events_data {
    int maxevents;
    struct epoll_event __user *events;
};
 
/* epoll_create()直接调用epoll_create1，size参数只是为了兼容 */
SYSCALL_DEFINE1(epoll_create, int, size)
{
     if (size <= 0)
          return -EINVAL;
     return sys_epoll_create1(0);
}

/* epoll_create1 */
SYSCALL_DEFINE1(epoll_create1, int, flags)
{
    int error;
    struct eventpoll *ep = NULL;//主描述符
    /* Check the EPOLL_* constant for consistency.  */
    BUILD_BUG_ON(EPOLL_CLOEXEC != O_CLOEXEC);

    /* 对于epoll来讲, 目前唯一有效的FLAG就是CLOEXEC */
    if (flags & ~EPOLL_CLOEXEC)
        return -EINVAL;

    /* Create the internal data structure ("struct eventpoll").
     * 分配一个struct eventpoll */
    error = ep_alloc(&ep);
    if (error < 0)
        return error;

    /* Creates all the items needed to setup an eventpoll file. That is,
     * a file structure and a free file descriptor.
     * 创建一个匿名fd, epollfd本身并不代表一个真正的文件, 所以内核需要创建一个
     * "虚拟"的文件, 并为之分配真正的struct file
     * 这里2个参数比较关键:
     * (1)eventpoll_fops, fops就是file operations, 就是当你对这个虚拟文件进行操作(比如读)时,
     * fops里面的函数指针指向真正的操作实现, 类似C++里面虚函数和子类的概念.
     * epoll只实现了poll和release(就是close)操作, 其它文件系统操作都有VFS全权处理了.
     * (2)ep, ep就是struct eventpoll, 它会作为一个私有数据保存在struct file的private指针里面
     * 就是为了能通过fd找到struct file, 再通过struct file能找到struct eventpoll. */
    error = anon_inode_getfd("[eventpoll]", &eventpoll_fops, ep,
                 O_RDWR | (flags & O_CLOEXEC));
    if (error < 0)
        ep_free(ep);
    return error;
}

/* epoll_ctl
 * (1)epfd, 就是epollfd
 * (2)op, ADD,MOD,DEL
 * (3)fd, 需要监听的描述符
 * (4)event, 关心的events */
SYSCALL_DEFINE4(epoll_ctl, int, epfd, int, op, int, fd,
        struct epoll_event __user *, event)
{
    int error;
    struct file *file, *tfile;
    struct eventpoll *ep;
    struct epitem *epi;
    struct epoll_event epds;
    error = -EFAULT;
    /* 错误处理以及从用户空间将struct epoll_event拷贝到内核空间. */
    if (ep_op_has_event(op) &&
        copy_from_user(&epds, event, sizeof(struct epoll_event)))
        goto error_return;
    error = -EBADF;

    /* Get the "struct file *" for the eventpoll file
     * 取得epfd对应的struct file, epfd是虚拟文件的fd
     * 在epoll_create1()中, 由函数anon_inode_getfd()分配 */
    file = fget(epfd);
    if (!file)
        goto error_return;

    /* Get the "struct file *" for the target file
     * 取得被监听的fd对应的struct file */
    tfile = fget(fd);
    if (!tfile)
        goto error_fput;

    /* The target file descriptor must support poll
     * epoll要求被监听的文件支持poll操作
     * 注意: 普通文件不支持poll操作，因此不能被epoll监听 */
    error = -EPERM;
    if (!tfile->f_op || !tfile->f_op->poll)
        goto error_tgt_fput;

    /* We have to check that the file structure underneath the file descriptor
     * the user passed to us _is_ an eventpoll file. And also we do not permit
     * adding an epoll file descriptor inside itself.
     * epoll不能自己监听自己 */
    error = -EINVAL;
    if (file == tfile || !is_file_epoll(file))
        goto error_tgt_fput;

    /* At this point it is safe to assume that the "private_data" contains
     * our own data structure.
     * 取到struct eventpoll, 先前再epoll_create1()中分配
     * 并保存在struct file的private_data字段 */
    ep = file->private_data;

    /* 接下来的操作有可能修改stuct eventpoll的内容, 加锁 
     * 由此可见, 内核保证了epoll相关的操作是线程安全的 */
    mutex_lock(&ep->mtx);

    /* Try to lookup the file inside our RB tree, Since we grabbed "mtx"
     * above, we can be sure to be able to use the item looked up by
     * ep_find() till we release the mutex.
     * 对于每一个监听的fd, 内核都有分配一个epitem结构,
     * 而且我们也知道, epoll是不允许重复添加fd的, 所以我们首先查找该fd是不是已经存在了.
     * ep_find()就是红黑树查找, O(logn)的时间复杂度. */
    epi = ep_find(ep, tfile, fd);
    error = -EINVAL;

    /* 根据ADD/MOD/DEL分别操作 */
    switch (op) {
    case EPOLL_CTL_ADD:
        if (!epi) {

            /* 之前的find没有找到, 证明是第一次监听
             * 这里我们可以知道, POLLERR和POLLHUP事件内核总是会关心的 */
            epds.events |= POLLERR | POLLHUP;

            /* 见ep_insert()流程 */
            error = ep_insert(ep, &epds, tfile, fd);
        } else

            /* 重复添加, 返回EEXIST错误 */
            error = -EEXIST;
        break;
    case EPOLL_CTL_DEL:
        if (epi)

            /* 之前的find找到了, 移除, 见ep_remove()流程*/
            error = ep_remove(ep, epi);
        else
            error = -ENOENT;
        break;
    case EPOLL_CTL_MOD:
        if (epi) {

            /* 之前的find找到了, 修改关心的事件，并保证总是关心POLLERR和POLLHUP */
            epds.events |= POLLERR | POLLHUP;
            error = ep_modify(ep, epi, &epds);
        } else
            error = -ENOENT;
        break;
    }

    /* 结束对struct eventpoll的内容修改，释放锁 */
    mutex_unlock(&ep->mtx);
error_tgt_fput:
    fput(tfile);
error_fput:
    fput(file);
error_return:
    return error;
}

/* 分配一个struct eventpoll, 在epoll_create1中被调用 */
static int ep_alloc(struct eventpoll **pep)
{
    int error;
    struct user_struct *user;
    struct eventpoll *ep;

    /* 获取当前用户的一些信息, 比如是不是root, 最大监听fd数目等 */
    user = get_current_user();
    error = -ENOMEM;
    ep = kzalloc(sizeof(*ep), GFP_KERNEL);
    if (unlikely(!ep))
        goto free_uid;

    /* 初始化struct eventpoll的各字段 */
    spin_lock_init(&ep->lock);
    mutex_init(&ep->mtx);

    /* 初始化自己wait的等待队列, 即调用epoll_wait会等待的队列 */
    init_waitqueue_head(&ep->wq);

    /* 初始化自身被poll的等待队列 */
    init_waitqueue_head(&ep->poll_wait);

    /* 初始化就绪链表 */
    INIT_LIST_HEAD(&ep->rdllist);
    ep->rbr = RB_ROOT;
    ep->ovflist = EP_UNACTIVE_PTR;
    ep->user = user;
    *pep = ep;
    return 0;
free_uid:
    free_uid(user);
    return error;
}

/* Must be called with "mtx" held.
 * ep_insert()在epoll_ctl()中被调用, 完成往epollfd里面添加一个监听fd的工作
 * tfile是我们监听的fd在内核态的struct file */
static int ep_insert(struct eventpoll *ep, struct epoll_event *event,
             struct file *tfile, int fd)
{
    int error, revents, pwake = 0;
    unsigned long flags;
    struct epitem *epi;
    struct ep_pqueue epq;

    /* 查看是否达到当前用户的最大监听数 */
    if (unlikely(atomic_read(&ep->user->epoll_watches) >=
             max_user_watches))
        return -ENOSPC;

    /* 从slab中分配一个epitem */
    if (!(epi = kmem_cache_alloc(epi_cache, GFP_KERNEL)))
        return -ENOMEM;

    /* Item initialization follow here 初始化epitem */
    INIT_LIST_HEAD(&epi->rdllink);
    INIT_LIST_HEAD(&epi->fllink);
    INIT_LIST_HEAD(&epi->pwqlist);
    epi->ep = ep;

    /* 保存了我们需要监听的文件fd和它的file结构 */
    ep_set_ffd(&epi->ffd, tfile, fd);
    epi->event = *event;
    epi->nwait = 0;
    epi->next = EP_UNACTIVE_PTR;
    epq.epi = epi;

    /* 初始化一个poll_table(epq.pt)
     * 指定调用poll_wait(不是epoll_wait)时的回调函数, 和我们关心的events,
     * ep_ptable_queue_proc()就是注册的回调, 初值是所有event都关心 */
    init_poll_funcptr(&epq.pt, ep_ptable_queue_proc);

    /* Attach the item to the poll hooks and get current event bits.
     * We can safely use the file* here because its usage count has
     * been increased by the caller of this function. Note that after
     * this operation completes, the poll callback can start hitting
     * the new item.
     * 首先, f_op->poll()一般来说只是个wrapper, 会调用真正的poll实现
     * 并且最终调用到我们的回调ep_ptable_queue_proc()完成fd和ep_poll_callback()的绑定
     * 因此实现了poll方法的才能被epoll监听
     * 完成这一步, 我们的epitem就跟这个socket关联起来了, 当它有状态变化时,
     * 会通过ep_poll_callback()来通知.
     * 最后, 这个函数还会查询当前的fd是不是已经有啥event已经ready了, 有的话
     * 会将event返回. */
    revents = tfile->f_op->poll(tfile, &epq.pt);
    error = -ENOMEM;
    if (epi->nwait < 0)
        goto error_unregister;

    /* Add the current item to the list of active epoll hook for this file
     * 每个文件会将所有监听自己的epitem链起来 */
    spin_lock(&tfile->f_lock);
    list_add_tail(&epi->fllink, &tfile->f_ep_links);
    spin_unlock(&tfile->f_lock);

    /* Add the current item to the RB tree. All RB tree operations are
     * protected by "mtx", and ep_insert() is called with "mtx" held.
     * 将epitem插入到对应的eventpoll中去 */
    ep_rbtree_insert(ep, epi);
    spin_lock_irqsave(&ep->lock, flags);

    /* If the file is already "ready" we drop it inside the ready list
     * 此时, 如果我们监听的fd已经有事件发生 */
    if ((revents & event->events) && !ep_is_linked(&epi->rdllink)) {

        /* 将当前的epitem加入到ready list中去 */
        list_add_tail(&epi->rdllink, &ep->rdllist);

        /* Notify waiting tasks that events are available
         * 谁在epoll_wait, 就唤醒它 */
        if (waitqueue_active(&ep->wq))
            wake_up_locked(&ep->wq);

        /* 谁在epoll当前的epollfd, 用pwake标记, 随后再唤醒 */
        if (waitqueue_active(&ep->poll_wait))
            pwake++;
    }
    spin_unlock_irqrestore(&ep->lock, flags);
    atomic_inc(&ep->user->epoll_watches);
    if (pwake)
        ep_poll_safewake(&ep->poll_wait);
    return 0;
error_unregister:
    ep_unregister_pollwait(ep, epi);
    spin_lock_irqsave(&ep->lock, flags);
    if (ep_is_linked(&epi->rdllink))
        list_del_init(&epi->rdllink);
    spin_unlock_irqrestore(&ep->lock, flags);
    kmem_cache_free(epi_cache, epi);
    return error;
}

/* This is the callback that is used to add our wait queue to the
 * target file wakeup lists.
 * 该函数在调用f_op->poll()时会被调用.
 * 也就是epoll主动poll某个fd, 用来将epitem与指定的fd关联起来的.
 * 关联的办法就是使用等待队列(waitqueue) */
static void ep_ptable_queue_proc(struct file *file, wait_queue_head_t *whead,
                 poll_table *pt)
{
    struct epitem *epi = ep_item_from_epqueue(pt);
    struct eppoll_entry *pwq;
    if (epi->nwait >= 0 && (pwq = kmem_cache_alloc(pwq_cache, GFP_KERNEL))) {

        /* 初始化等待队列, 指定ep_poll_callback为唤醒时的回调函数,
         * 当我们监听的fd发生状态改变时, 也就是队列头被唤醒时,
         * 指定的回调函数将会被调用. */
        init_waitqueue_func_entry(&pwq->wait, ep_poll_callback);
        pwq->whead = whead;
        pwq->base = epi;

        /* 将刚分配的等待队列成员加入到等待队列头中(相应的fd持有) */
        add_wait_queue(whead, &pwq->wait);

        /* 将eppoll_entry链接到对应的epitem上 */        
        list_add_tail(&pwq->llink, &epi->pwqlist);

        /* nwait记录了当前epitem加入到了多少个等待队列中, 1 */
        epi->nwait++;
    } else {
        /* We have to signal that an error occurred */
        epi->nwait = -1;
    }
}

/* This is the callback that is passed to the wait queue wakeup
 * machanism. It is called by the stored file descriptors when they
 * have events to report.
 * 这个是与fd绑定的回调函数, 当监听的fd发生事件时, 它会被调用.
 * 参数key被当作一个unsigned long整数使用, 携带的是events. */
static int ep_poll_callback(wait_queue_t *wait, unsigned mode, int sync, void *key)
{
    int pwake = 0;
    unsigned long flags;
    struct epitem *epi = ep_item_from_wait(wait);
    struct eventpoll *ep = epi->ep;
    spin_lock_irqsave(&ep->lock, flags);

    /* If the event mask does not contain any poll(2) event, we consider the
     * descriptor to be disabled. This condition is likely the effect of the
     * EPOLLONESHOT bit that disables the descriptor when an event is received,
     * until the next EPOLL_CTL_MOD will be issued. */
    if (!(epi->event.events & ~EP_PRIVATE_BITS))
        goto out_unlock;

    /* Check the events coming with the callback. At this stage, not
     * every device reports the events in the "key" parameter of the
     * callback. We need to be able to handle both cases here, hence the
     * test for "key" != NULL before the event match test.
     * 没有关心的event */
    if (key && !((unsigned long) key & epi->event.events))
        goto out_unlock;

    /* If we are trasfering events to userspace, we can hold no locks
     * (because we're accessing user memory, and because of linux f_op->poll()
     * semantics). All the events that happens during that period of time are
     * chained in ep->ovflist and requeued later on.
     * 如果该callback被调用的同时, epoll_wait()已经返回了,
     * 这种情况下, 内核将此刻发生event的epitem用一个单独的链表ovflist
     * 链起来, 不发给应用程序, 也不丢弃, 而是在下一次epoll_wait时再返回给用户. */
    if (unlikely(ep->ovflist != EP_UNACTIVE_PTR)) {
        if (epi->next == EP_UNACTIVE_PTR) {
            epi->next = ep->ovflist;
            ep->ovflist = epi;
        }
        goto out_unlock;
    }

    /* If this file is already in the ready list we exit soon
     * 将当前的epitem放入ready list */
    if (!ep_is_linked(&epi->rdllink))
        list_add_tail(&epi->rdllink, &ep->rdllist);

    /* Wake up ( if active ) both the eventpoll wait list and the ->poll()
     * wait list.
     * 唤醒epoll_wait */
    if (waitqueue_active(&ep->wq))
        wake_up_locked(&ep->wq);

    /* 如果epollfd自身也在被poll, pwake标记, 随后唤醒. */
    if (waitqueue_active(&ep->poll_wait))
        pwake++;
out_unlock:
    spin_unlock_irqrestore(&ep->lock, flags);
    if (pwake)
        ep_poll_safewake(&ep->poll_wait);
    return 1;
}

/* Implement the event wait interface for the eventpoll file. It is the kernel
 * part of the user space epoll_wait(2).
 * epoll_wait */
SYSCALL_DEFINE4(epoll_wait, int, epfd, struct epoll_event __user *, events,
        int, maxevents, int, timeout)
{
    int error;
    struct file *file;
    struct eventpoll *ep;
    /* The maximum number of event must be greater than zero */
    if (maxevents <= 0 || maxevents > EP_MAX_EVENTS)
        return -EINVAL;

    /* Verify that the area passed by the user is writeable
     * 内核对应用程序采取的策略是绝对不信任,
     * 所以内核跟应用程序之间的数据交互大都是copy, 不允许(也时候也是不能)指针引用.
     * epoll_wait()需要内核返回数据给用户空间, 内存由用户程序提供,
     * 所以内核会用一些手段来验证这一段内存空间是不是有效的. */
    if (!access_ok(VERIFY_WRITE, events, maxevents * sizeof(struct epoll_event))) {
        error = -EFAULT;
        goto error_return;
    }

    error = -EBADF;
    /* 获取epollfd的struct file, epollfd是虚拟文件 */
    file = fget(epfd);
    if (!file)
        goto error_return;

    error = -EINVAL;
    /* 检查一下它是不是一个真正的epollfd */
    if (!is_file_epoll(file))
        goto error_fput;

    /* 获取eventpoll结构 */
    ep = file->private_data;

    /* 调用ep_poll真正等待event到来或超时 */
    error = ep_poll(ep, events, maxevents, timeout);
error_fput:
    fput(file);
error_return:
    return error;
}

/* ep_poll被epoll_wait调用 */
static int ep_poll(struct eventpoll *ep, struct epoll_event __user *events,
           int maxevents, long timeout)
{
    int res, eavail;
    unsigned long flags;
    long jtimeout;
    wait_queue_t wait;

    /* Calculate the timeout by checking for the "infinite" value (-1)
     * and the overflow condition. The passed timeout is in milliseconds,
     * that why (t * HZ) / 1000.
     * 计算等待时间 */
    jtimeout = (timeout < 0 || timeout >= EP_MAX_MSTIMEO) ?
        MAX_SCHEDULE_TIMEOUT : (timeout * HZ + 999) / 1000;
retry:
    spin_lock_irqsave(&ep->lock, flags);
    res = 0;

    /* 如果ready list为空说明还未有事件, 准备等待 */
    if (list_empty(&ep->rdllist)) {

        /* We don't have any available event to return to the caller.
         * We need to sleep here, and we will be wake up by
         * ep_poll_callback() when events will become available.
         * 初始化一个等待队列, 准备直接把自己挂起,等到ep_poll_callback()唤醒自己
         * 注意current是一个宏, 代表当前进程 */
        init_waitqueue_entry(&wait, current);

        /* 挂载到struct eventpoll的等待队列，随后由ep_poll_callback()唤醒 */
        __add_wait_queue_exclusive(&ep->wq, &wait);
        for (;;) {

            /* We don't want to sleep if the ep_poll_callback() sends us
             * a wakeup in between. That's why we set the task state
             * to TASK_INTERRUPTIBLE before doing the checks.
             * 将当前进程设置为睡眠, 但是可以被信号唤醒的状态 */
            set_current_state(TASK_INTERRUPTIBLE);

            /* 此时若ready list里面有成员了, 或者已经等待超时, 直接返回 */
            if (!list_empty(&ep->rdllist) || !jtimeout)
                break;

            /* 如果此时已经有信号产生, 直接返回 */
            if (signal_pending(current)) {
                res = -EINTR;
                break;
            }
            spin_unlock_irqrestore(&ep->lock, flags);

            /* jtimeout这个时间后, 会被唤醒,
             * 若fd有事件, 则ep_poll_callback()被调用, 直接被唤醒并返回 */
            jtimeout = schedule_timeout(jtimeout);//睡觉
            spin_lock_irqsave(&ep->lock, flags);
        }
        __remove_wait_queue(&ep->wq, &wait);

        /* 更新状态 */
        set_current_state(TASK_RUNNING);
    }

    /* Is it worth to try to dig for events ?
     * 此时如果有事件就绪，无论是ready事件还是ovflist链接的事件都算就绪事件 */
    eavail = !list_empty(&ep->rdllist) || ep->ovflist != EP_UNACTIVE_PTR;
    spin_unlock_irqrestore(&ep->lock, flags);

    /* Try to transfer events to user space. In case we get 0 events and
     * there's still timeout left over, we go trying again in search of
     * more luck.
     * 如果一切正常, 有event发生, 开始准备数据copy给用户空间了 */
    if (!res && eavail &&
        !(res = ep_send_events(ep, events, maxevents)) && jtimeout)
        goto retry;
    return res;
}

/* ep_send_events拷贝事件给用户空间，不超过指定的最大事件数 */
static int ep_send_events(struct eventpoll *ep,
              struct epoll_event __user *events, int maxevents)
{
    struct ep_send_events_data esed;
    esed.maxevents = maxevents;
    esed.events = events;
    return ep_scan_ready_list(ep, ep_send_events_proc, &esed);
}

/* ep_scan_ready_list - Scans the ready list in a way that makes possible for
 *                      the scan code, to call f_op->poll(). Also allows for
 *                      O(NumReady) performance.
 *
 * @ep: Pointer to the epoll private data structure.
 * @sproc: Pointer to the scan callback.
 * @priv: Private opaque data passed to the @sproc callback.
 *
 * Returns: The same integer error code returned by the @sproc callback. */
static int ep_scan_ready_list(struct eventpoll *ep,
                  int (*sproc)(struct eventpoll *, struct list_head *, void *),
                  void *priv)
{
    int error, pwake = 0;
    unsigned long flags;
    struct epitem *epi, *nepi;
    LIST_HEAD(txlist);

    /* We need to lock this because we could be hit by
     * eventpoll_release_file() and epoll_ctl(). 
     * 扫描就绪事件时需要把struct eventpoll上锁 */
    mutex_lock(&ep->mtx);

    /* Steal the ready list, and re-init the original one to the
     * empty list. Also, set ep->ovflist to NULL so that events
     * happening while looping w/out locks, are not lost. We cannot
     * have the poll callback to queue directly on ep->rdllist,
     * because we want the "sproc" callback to be able to do it
     * in a lockless way. */
    spin_lock_irqsave(&ep->lock, flags);

    /* 首先, 所有监听到events的epitem都链到rdllist上了,
     * 但是这一步之后, 所有的epitem都转移到了txlist上, 而rdllist被清空了 */
    list_splice_init(&ep->rdllist, &txlist);
    ep->ovflist = NULL;
    spin_unlock_irqrestore(&ep->lock, flags);

    /* 在这个回调函数里面处理每个epitem
     * sproc 就是 ep_send_events_proc */
    error = (*sproc)(ep, &txlist, priv);
    spin_lock_irqsave(&ep->lock, flags);

    /* During the time we spent inside the "sproc" callback, some
     * other events might have been queued by the poll callback.
     * We re-insert them inside the main ready-list here.
     * 在处理回调时发生了新的事件保存在ovflist中, 即ovflist里的epitem都是
     * 我们在传递数据给用户空间时监听到了事件. */
    for (nepi = ep->ovflist; (epi = nepi) != NULL;
         nepi = epi->next, epi->next = EP_UNACTIVE_PTR) {

        /* We need to check if the item is already in the list.
         * During the "sproc" callback execution time, items are
         * queued into ->ovflist but the "txlist" might already
         * contain them, and the list_splice() below takes care of them.
         * 检查重复, 将不重复的直接放入ready list */
        if (!ep_is_linked(&epi->rdllink))
            list_add_tail(&epi->rdllink, &ep->rdllist);
    }
    
    /* We need to set back ep->ovflist to EP_UNACTIVE_PTR, so that after
     * releasing the lock, events will be queued in the normal way inside
     * ep->rdllist. 
     * 重置ovflist, 释放锁后新事件继续添加到ovflist */
    ep->ovflist = EP_UNACTIVE_PTR;
    
    /* Quickly re-inject items left on "txlist".
     * 上一次没有处理完的epitem, 重新插入到ready list */
    list_splice(&txlist, &ep->rdllist);

    /* ready list不为空, 释放锁后继续直接唤醒 */
    if (!list_empty(&ep->rdllist)) {
        /* Wake up (if active) both the eventpoll wait list and
         * the ->poll() wait list (delayed after we release the lock). */
        if (waitqueue_active(&ep->wq))
            wake_up_locked(&ep->wq);
        if (waitqueue_active(&ep->poll_wait))
            pwake++;
    }
    spin_unlock_irqrestore(&ep->lock, flags);
    mutex_unlock(&ep->mtx);
    /* We have to call this outside the lock */
    if (pwake)
        ep_poll_safewake(&ep->poll_wait);
    return error;
}

/* 该函数作为callback在ep_scan_ready_list()中被调用
 * head是一个链表, 包含了已经ready的epitem,
 * 这个不是eventpoll里面的ready list, 而是上面函数中的txlist. */
static int ep_send_events_proc(struct eventpoll *ep, struct list_head *head, void *priv)
{
    struct ep_send_events_data *esed = priv;
    int eventcnt;
    unsigned int revents;
    struct epitem *epi;
    struct epoll_event __user *uevent;

    /* We can loop without lock because we are passed a task private list.
     * Items cannot vanish during the loop because ep_scan_ready_list() is
     * holding "mtx" during this call.
     * 扫描整个txlist链表 */
    for (eventcnt = 0, uevent = esed->events;
         !list_empty(head) && eventcnt < esed->maxevents;) {

        /* 取出第一个成员 */
        epi = list_first_entry(head, struct epitem, rdllink);

        /* 然后从链表里面移除 */
        list_del_init(&epi->rdllink);

        /* 读取events,
         * 注意events我们ep_poll_callback()里面已经取过一次了, 重复取是因为
         * 1. 我们当然希望能拿到此刻的最新数据, events是会变的~
         * 2. 不是所有的poll实现都通过等待队列传递了events, 有可能某些驱动没传, 必须主动去读. */
        revents = epi->ffd.file->f_op->poll(epi->ffd.file, NULL) &
            epi->event.events;

        /* If the event mask intersect the caller-requested one,
         * deliver the event to userspace. Again, ep_scan_ready_list()
         * is holding "mtx", so no operations coming from userspace
         * can change the item. 
         * 如果发生的事件和用户关心的事件有重叠, 传递给用户 */
        if (revents) {

            /* 将当前的事件和用户传入的数据都copy给用户空间,
             * 就是epoll_wait()后应用程序能读到的那一堆数据. */
            if (__put_user(revents, &uevent->events) ||
                __put_user(epi->event.data, &uevent->data)) {

                /* 如果copy过程中发生错误, 会中断链表的扫描,
                 * 并把当前发生错误的epitem重新插入到ready list.
                 * 剩下的没处理的epitem也不会丢弃, 在ep_scan_ready_list()
                 * 中它们也会被重新插入到ready list */
                list_add(&epi->rdllink, head);
                return eventcnt ? eventcnt : -EFAULT;
            }
            eventcnt++;
            uevent++;
            if (epi->event.events & EPOLLONESHOT)
                epi->event.events &= EP_PRIVATE_BITS;
            else if (!(epi->event.events & EPOLLET)) {

                /* If this file has been added with Level
                 * Trigger mode, we need to insert back inside
                 * the ready list, so that the next call to
                 * epoll_wait() will check again the events
                 * availability. At this point, noone can insert
                 * into ep->rdllist besides us. The epoll_ctl()
                 * callers are locked out by
                 * ep_scan_ready_list() holding "mtx" and the
                 * poll callback will queue them in ep->ovflist.
                 * 水平触发EPOLLLT和边沿触发EPOLLET区别就在这里
                 * 1. 如果是ET, 无论是否还有事件或数据, 
                 * epitem是不会再进入到readly list,
                 * 除非fd再次发生了状态改变, ep_poll_callback被调用.
                 * 2. 如果是LT, 无论是否还有事件或数据,
                 * epitem都会被重新插入到ready list, 在下一次epoll_wait
                 * 时, 会立即返回, 并通知给用户空间. 如果这个被监听的fd
                 * 没事件也没数据了, epoll_wait会返回一个0, 空转一次. */
                list_add_tail(&epi->rdllink, &ep->rdllist);
            }
        }
    }
    return eventcnt;
}

/* ep_free在epollfd被close时调用, 释放一些资源 */
static void ep_free(struct eventpoll *ep)
{
    struct rb_node *rbp;
    struct epitem *epi;
    /* We need to release all tasks waiting for these file */
    if (waitqueue_active(&ep->poll_wait))
        ep_poll_safewake(&ep->poll_wait);
    /* We need to lock this because we could be hit by
     * eventpoll_release_file() while we're freeing the "struct eventpoll".
     * We do not need to hold "ep->mtx" here because the epoll file
     * is on the way to be removed and no one has references to it
     * anymore. The only hit might come from eventpoll_release_file() but
     * holding "epmutex" is sufficent here.*/
    mutex_lock(&epmutex);
    /* Walks through the whole tree by unregistering poll callbacks. */
    for (rbp = rb_first(&ep->rbr); rbp; rbp = rb_next(rbp)) {
        epi = rb_entry(rbp, struct epitem, rbn);
        ep_unregister_pollwait(ep, epi);
    }
    /* Walks through the whole tree by freeing each "struct epitem". At this
     * point we are sure no poll callbacks will be lingering around, and also by
     * holding "epmutex" we can be sure that no file cleanup code will hit
     * us during this operation. So we can avoid the lock on "ep->lock".
     * 关闭epollfd之前不需要调用epoll_ctl移除已经添加的fd, 是因为close时已经做了 */
    while ((rbp = rb_first(&ep->rbr)) != NULL) {
        epi = rb_entry(rbp, struct epitem, rbn);
        ep_remove(ep, epi);
    }
    mutex_unlock(&epmutex);
    mutex_destroy(&ep->mtx);
    free_uid(ep->user);
    kfree(ep);
}

/* File callbacks that implement the eventpoll file behaviour */
static const struct file_operations eventpoll_fops = {
    .release    = ep_eventpoll_release,
    .poll       = ep_eventpoll_poll
};

/* Fast test to see if the file is an evenpoll file */
static inline int is_file_epoll(struct file *f)
{
    return f->f_op == &eventpoll_fops;
}
```