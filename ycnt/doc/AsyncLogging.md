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