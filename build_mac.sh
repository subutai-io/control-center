#!/bin/bash
QTBINPATH=/home/builder/qt_static/bin
BRANCH=$1
TRAYDIR=/Users/travis/build/subutai-io/control-center/

echo "$BRANCH"

export PATH=$QTBINPATH:$PATH

git checkout -- .
git pull
git checkout $BRANCH
git pull origin $BRANCH

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

cd $TRAYDIR

PKGNAME="subutai-control-center.pkg"
case $BRANCH in
	dev)
		PKGNAME="subutai-control-center-dev.pkg"
		;;
	master)
		PKGNAME="subutai-control-center-master.pkg"
		;;
	head)
		PKGNAME="subutai-control-center.pkg"   
    	;;
    HEAD)
		PKGNAME="subutai-control-center.pkg"
    	;;  
esac

cd darwin-packages/
rm -rf flat
rm -rf root

./pack.sh ../subutai_control_center_bin/SubutaiControlCenter.app $BRANCH
mv $PKGNAME ../subutai_control_center_bin/
