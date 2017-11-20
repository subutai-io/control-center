#!/bin/bash
build="subutai_tray_bin"

if [ -d "$build" ]; then 
	echo "Try to remove build"
  rm -rf $build
fi 
mkdir -p $build
cd $build
qmake ../SubutaiTray.pro -r -spec macx-clang CONFIG+=x86_64
make 
cd ../
macdeployqt subutai_tray_bin/SubutaiTray.app
cd subutai_tray_bin/SubutaiTray.app/Contents/MacOS/
#source ../../../../after_build_step_mac_os 
cp SubutaiTray SubutaiTray_osx
