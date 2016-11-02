## Version 4.0.5

1. Fixed bug in logging system that caused to segmentation fault on unix like systems (with POSIX threads). 
2. Fixed bug with RH update.
3. Added ability to use default SSH-key (id_rsa).
4. Changed message about unhealthy environments. 
5. Changed debian package creation.
6. Changed default P2P path for Linux. 


## Version 4.0.4 

1. Changed output of libssh2 library
2. Automated p2p daemon restarting on Mac OSX and Windows systems.
3. Changed "About" dialog behavior. Moved it's initialization to thread. Old version freezed in some cases. 
4. Fixed memory leaks and bugs related to dialog management.
5. Optimized web-socket commands handling for E2E plugin
