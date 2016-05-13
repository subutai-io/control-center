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

1. Get libssh2 sources from [github](https://github.com/libssh2/libssh2)
2. Get openssl sources for windows
3. Install Windows SDK
4. Build and install openssl libraries. Follow instructions in INSTALL.W64 and INSTALL.W32. You should use Visual Studio Command Promt x64.
5. Build libssh2 as __SHARED LIBRARY__. Follow instructions in docs/INSTALL_CMAKE. 
  * Use `cmake -DBUILD_SHARED_LIBS=ON ..`
  * Open libssh2.sln in VisualStudio.
  * Change target platform to x64 and build type to release 
  * Go to libssh2 project properties -> linker -> Command Line. There you should remove everything in AdditionalOptions
  * Rebuild libssh2.
  * Make lib file from result dll. [Instruction](https://adrianhenke.wordpress.com/2008/12/05/create-lib-file-from-dll/)
6. Add qmake (and other qt bin tools) and jom to PATH.
7. Open VS Command Promt x64 and go to directory with all sources.
8. There launch `build_win.bat arg` where arg is full path to file VBoxRT.dll
9. Your SubutaiTray.exe will be located in subutai_tray_bin/release folder
### Branching Model 

We use [GITFLOW] (http://nvie.com/posts/a-successful-git-branching-model/)

