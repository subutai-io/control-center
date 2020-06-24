#!/bin/bash
QTBINPATH=/Users/dev/Qt5.9.2/5.9.2/clang_64/bin/
BRANCH=$1
TRAYDIR=/Users/travis/build/subutai-io/control-center/

echo "$BRANCH"

export PATH=$QTBINPATH:$PATH

#git checkout -- .
#git pull
#git checkout $BRANCH
#git pull origin $BRANCH

mkdir -p /usr/local/opt/libssh2/lib/
mkdir -p /usr/local/lib/
mkdir -p /usr/local/lib/pkgconfig/
cp /Users/travis/build/subutai-io/control-center/libssh2/lib/mac/libssh2.1.dylib /usr/local/opt/libssh2/lib/libssh2.1.dylib
cp /Users/travis/build/subutai-io/control-center/libssh2/lib/mac/* /usr/local/lib/
cp /Users/travis/build/subutai-io/control-center/libssh2/lib/mac/libssh2.pc /usr/local/lib/pkgconfig/libssh2.pc
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
