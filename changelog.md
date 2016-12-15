## Version 4.0.11

1. Removed all "system()" calls from code.
2. Made all those calls UNICODE compatible.
3. Removed unused parts of code.
4. Changed environments and containers refreshing mechanism.
5. Refactored many things.

## Version 4.0.10
## Version 4.0.9

1. Nothing changed. This new version made for all components compatibility.

## Version 4.0.7

1. Optimized dev/master updating system for p2p and tray.
2. Refactoring

## Version 4.0.6

1. Bug fix with p2p updating.
2. Code size optimization.
3. Settings refactoring.

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
