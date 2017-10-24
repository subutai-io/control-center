#!/bin/bash

branch=$1

rm -rf debian/icons
rm -rf debian/SubutaiTray
rm -rf debian/subutai-tray

mkdir -p debian/icons

cp ../../resources/Tray_icon_set-07.png debian/icons
cp ../../subutai_tray_bin/SubutaiTray debian/SubutaiTray

debuild -B -d
