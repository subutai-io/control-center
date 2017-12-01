#!/bin/bash
build="subutai_tray_bin"

if [ -d "$build" ]; then 
	echo "Try to remove build"
  rm -rf $build
fi
mkdir -p $build
cd $build
lrelease ../SubutaiTray.pro
qmake ../SubutaiTray.pro -r -spec macx-clang CONFIG+=x86_64
make 
mv ../*.qm .
cd ../
macdeployqt subutai_tray_bin/SubutaiTray.app
cd subutai_tray_bin/SubutaiTray.app/Contents/MacOS/
cp ../../../../*.qm .
#source ../../../../after_build_step_mac_os 
cp SubutaiTray SubutaiTray_osx
