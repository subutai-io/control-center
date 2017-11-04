#!/bin/bash

./clear.sh
mkdir -p debian/icons
mkdir -p debian/SubutaiTray/bin
mkdir -p debian/SubutaiTray/lib

cp ~/SRC/tray/resources/Tray_icon_set-07.png debian/icons

cp ../../subutai_tray_bin/SubutaiTray debian/SubutaiTray/bin
./dependencies.sh ../../subutai_tray_bin/SubutaiTray debian/SubutaiTray/lib/

cd debian
tar zcvf SubutaiTray.tar.gz SubutaiTray
cd ..

#debuild -B -d
debuild
