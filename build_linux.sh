#!/bin/bash
nproc_count="$(nproc)"
core_number=$((nproc_count*2+1))
subutai_tray_bin="subutai_tray_bin"

if [ -d "$subutai_tray_bin" ]; then 
	echo "Try to remove subutai_tray_bin"
  rm -rf subutai_tray_bin
fi 
mkdir subutai_tray_bin
cd subutai_tray_bin
#qmake ../SubutaiTray.pro -r -spec linux-g++
qmake ../SubutaiTray.pro -r -spec win32-x-g++
make -j$core_number 
cd ../
