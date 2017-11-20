#!/bin/bash
subutai_tray_bin="subutai_tray_bin"

if [ -d "$subutai_tray_bin" ]; then 
	echo "Try to remove subutai_tray_bin"
  rm -rf subutai_tray_bin
fi 
mkdir subutai_tray_bin
cd subutai_tray_bin
lrelease ../SubutaiTray.pro 
qmake ../SubutaiTray.pro -r -spec macx-clang CONFIG+=x86_64
make 
mv ../*.qm .
cd ../
macdeployqt subutai_tray_bin/SubutaiTray.app
cd subutai_tray_bin/SubutaiTray.app/Contents/MacOS/
source ../../../../after_build_step_mac_os 
cp SubutaiTray SubutaiTray_osx
