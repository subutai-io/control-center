#!/bin/bash
build="subutai_control_center_bin"

if [ -d "$build" ]; then 
	echo "Try to remove build"
  rm -rf $build
fi
mkdir -p $build
cd $build
lrelease ../SubutaiControlCenter.pro
qmake ../SubutaiControlCenter.pro -r -spec macx-clang CONFIG+=x86_64
make 
mv ../*.qm .
cd ../
macdeployqt subutai_control_center_bin/SubutaiControlCenter.app
cd subutai_control_center_bin/SubutaiControlCenter.app/Contents/MacOS/
cp ../../../*.qm .
#source ../../../../after_build_step_mac_os 
cp SubutaiControlCenter SubutaiControlCenter_osx
cp SubutaiControlCenter_osx ../../../.
