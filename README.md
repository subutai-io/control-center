Subutai Tray Application
===================

This application allows non technical users to manage their containers and see information about their environments, peers, balance etc. Should be provided within subutai installers and can be launched in OS specific manner.

Requirenments 
-------------------

* For VM management VirtualBox should be installed. 
* For SSH and swarm joining p2p (https://github.com/subutai-io/p2p) should be installed. The application settings dialog has a setting to specify the path to the p2p executable. 
* SSH should be available from the system command promt.

Development & branching model
-------------------


Building

* You have to install qt sdk. If you want to build from IDE - use qt creator. If you want to build from command line, use commands listed below:

qmake SubutaiTray.pro -r -spec linux-g++

make

### Branching Model 

We use [GITFLOW] (http://nvie.com/posts/a-successful-git-branching-model/)

