Subutai Tray Application
===================

This application allows non technical users to manage their containers and see information about their environments, peers, balance etc. Should be provided within subutai installers and can be launched in OS specific manner.

Build requirenments 
-------------------

* Qt sdk
* libssh2 (linux and osx)
* libpthread (linux and osx)
* cat/type commands should be available

Development & branching model
-------------------

Building

* If you want to build from IDE - use qt creator (SubutaiTray.pro project). If you want to build from command line, use commands listed below:

__Modify PATH. Like this : `export PATH=path_to_qmake_bin:$PATH`__

For __LINUX__  run `./build_linux.sh`. You can modify this script, it is very simple.

For __Mac OS__ do the same, but run `./build_mac.sh`

For __WINDOWS__ run this : `build_win.bat "path_to libeay32.dll and ssleay32.dll"`
### Branching Model 

We use [GITFLOW] (http://nvie.com/posts/a-successful-git-branching-model/)

