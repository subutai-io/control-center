
<h1 align="center">
  <br>
  <a href="http://www.subutai.io/"><img src="https://github.com/subutai-io/control-center/blob/master/resources/cc_icon_last.png" alt="Subutai" width="500"></a>
  <br>
  Subutai Control Center
  <br>
</h1>

<h4 align="center">Easy life for users of <a href="http://bazaar.subutai.io" target="_blank">Bazaar</a>.</h4>
<!-- need to work on this
<p align="center">
  <a href="https://gitter.im/amitmerchant1990/electron-markdownify"><img src="https://badges.gitter.im/amitmerchant1990/electron-markdownify.svg"></a>
  <a href="https://saythanks.io/to/amitmerchant1990">
      <img src="https://img.shields.io/badge/SayThanks.io-%E2%98%BC-1EAEDB.svg">
  </a>
  <a href="https://www.paypal.me/AmitMerchant">
    <img src="https://img.shields.io/badge/$-donate-ff69b4.svg?maxAge=2592000&amp;style=flat">
  </a>
</p> -->

<p align="center">
  <a href="#key-features">Key Features</a> •
  <a href="#how-to-use">How To Build</a> •
  <a href="#download">Download</a> •
  <a href="#about-subutai">About Subutai</a> •
  <a href="#license">License</a>
</p>

## Key Features

* Bazaar account monitoring
  - Informatin of user environments, peers, balance and etc
* Managing cloud environemtns
  - SSHing, File transfer, Key deployment
* PeerOS manager
  - Create your own virtual machine with PeerOS on it, register them to Bazaar, share your resources to the world
* Software manager
  - Install all 3rd party components with approve of user
  - Vagrant, Virtualbox, Google Chrome, Subutai P2P, X2Go-Client are supported
* Dark/Light themes
* Fast and quick usage tray application
* Cross platform
  - Windows, Mac and Linux ready.

## How To Build

To clone and run this application, you'll need [Git](https://git-scm.com), [qmake](https://www.qt.io/download) are installed on your computer, and [Qt Creator](https://www.qt.io/download) if you want to build from IDE. You need C/C++ compilers [MinGW](http://www.mingw.org/wiki/linuxcrossmingw) for Linux, [clang](https://clang.llvm.org/get_started.html) for macOS and [MSVC++](http://landinghub.visualstudio.com/visual-cpp-build-tools) for Windows. From your command line:

Use ```git checkout $(cat version)``` on **unix** or ```git checkout $(type version)``` on **Windows** to switch to the latest released branch. After you can build from command line using these commands:

for **MacOS**
```bash
qmake SubutaiControlCenter.pro -r -spec macx-clang CONFIG+=x86_64
make
```
for **Linux**
```bash
./configure
make
```

for **Windows**
```bash
qmake ..\SubutaiControlCenter.pro -r -spec win32-msvc
jom
```

## Download

You can [download](https://subutai.io/getting-started.html#Control-Center) latest installable version of Subutai Control Center for Windows, macOS and Linux.

## About Subutai

[Subutai](https://subutai.io) - Open Source Peer-to-Peer (P2P) Cloud Computing
Internet of Things (IoT) and Cryptocurrency Mining for Everyone


## License

Apache License 2.0

