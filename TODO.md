# TODO list

---------------------------------

A list of planning features that are different from muduo.

|#|description|status|
|:---:|:---:|:---:|
|1|use Grisu2 instead of snprintf for floating number|&radic;|
|2|async logging with concurrent back-end support|&radic;|
|3|epoll edge trigger support|&times;|
|4|channel pipeline for handlers|&times;|
|5|channel priority||
|6|new connection load balancing policy (RR in muduo) ||
|7|a new design of ThreadPool|&radic;|
|8|batch accept new connections|&times;|
|9|use maxConnections to limit the acceptor|&times;|

A list of objectives beyond ycnt.

|#|description|status|
|:---:|:---:|:---:|
|1|websocket server|0%|
|2|matching server (liquibook)|0%|
|3|high available matching service (dragonboat)|0%|
|4|rpc framework|0%|