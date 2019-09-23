```
0.Basic:

    thread     [0,4,...] [1,5,...] [2,6,...] [3,7,...]
                   |         |         |         |
                   |         |         |         |
 fixedBuffers  [***    ] [*      ] [**     ] [****** ]
 frontBuffers  []
  backBuffers  []
   bufferPool  [ empty ] [ empty ] [ empty ] ...

1.fixedBuffer full

    thread     [0,4,...] ...
                   |
                   |(1)add full to front, get empty from pool
 fixedBuffers  <-[   ]------------<
               |                  |
               |(2)notify writer  |
 frontBuffers [full]              |
                                  |
   bufferPool  ... [empty] ------->


2.back writer wake up (notified or timedout)

 fixedBuffers                       [   ]<-<(2)get empty from pool
                                      |    |
 frontBuffers  [full] [full] ... <----<(1)directly add all non-empty to front
       |(3)add front to back               |
  backBuffers  []                          |
       |                                   |
       |        >-->bufferPool ... [empty]->
       |        |(5)add empty to pool
       >--->[logFile](4)write back to logFile
```