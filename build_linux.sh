#!/bin/bash
nproc_count="$(nproc)"
core_number=$((nproc_count*2+1))
subutai_control_center_bin="subutai_control_center_bin"

if [ -d "$subutai_control_center_bin" ]; then 
	echo "Try to remove subutai_control_center_bin"
  rm -rf subutai_control_center_bin
fi 
mkdir subutai_control_center_bin
cd subutai_control_center_bin
lrelease ../SubutaiControlCenter.pro
qmake ../SubutaiControlCenter.pro -r -spec linux-g++
make -j$core_number 
rm *.o *.cpp *.h
mv ../*.qm .
cd ../
