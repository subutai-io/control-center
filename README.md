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

__Modify PATH. Like this : `export PATH=path_to_qmake_bin:$PATH`__

For __LINUX__  run `./build_linux.sh`. You can modify this script, it is very simple.

For __Mac OS__ do the same, but run `./build_mac.sh`

For __WINDOWS__ follow this : 

Windows build:

1. Launch qt-creator. You can download it from [official site](http://www.qt.io)
* Load SubutaiTray.pro
* Change build type to release.
* Run qmake (Build -> Run qmake in main menu)
* Run Rebuild All (Build -> Rebuild All in main menu) 
* Close qt-creator
* Launch VS (project was created in VS2013, but you can use 2005 and newer).
* Load libssh2/libssh2_vc_app.vcproj
* Set build type to release.
* Set platform WIN32 (default)
* Rebuild all.
* Rename result binary to libssh2_app.exe
* Copy libssh2_app.exe to folder with SubutaiTray.exe
* Close VS
* Run qt command promt.
* Go to folder with SubutaiTray.exe (by cd commands)
* Run next commands  : 
* Copy VBoxRT.dll to that folder. 
```
del *.obj
del *.cpp
del *.h
windeployqt --compiler-runtime --release --no-translations 
```

__NOTE__ : You should have installed open ssl libs. You should have installed libssh2 libraries. You have to install VirtualBox.

### Branching Model 

We use [GITFLOW] (http://nvie.com/posts/a-successful-git-branching-model/)

