# TODO list

---------------------------------

A list of planning features that are different from muduo.

|#|description|status|
|:---:|:---:|:---:|
|1|use Grisu2 instead of snprintf for floating number|&radic;|
|2|logging with concurrent back-end support|&radic;|
|3|epoll edge trigger support|&times;|
|4|channel pipeline for handlers|&times;|
|5|channel priority||
|6|new connection load balancing policy (RR in muduo) ||
|7|submit waitable task in ThreadPool|&radic;|

A list of objectives beyond ycnt.

|#|description|status|
|:---:|:---:|:---:|
|1|websocket server||
|2|matching server (liquibook)||
|3|high available matching service (dragonboat)||