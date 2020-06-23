#!/bin/bash
QTBINPATH=/Users/dev/Qt5.9.2/5.9.2/clang_64/bin/
BRANCH=$1
TRAYDIR=/Users/travis/build/subutai-io/control-center/

echo "$BRANCH"

export PATH=$QTBINPATH:$PATH

git checkout -- .
git pull
git checkout $BRANCH
git pull origin $BRANCH

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
security default-keychain -s build.keychain
codesign -s YM8SD62476 --deep ./SubutaiControlCenter
cp SubutaiControlCenter SubutaiControlCenter_osx
cp SubutaiControlCenter_osx ../../../.

cd $TRAYDIR

PKGNAME="subutai-control-center.pkg"
case $BRANCH in
	dev)
		PKGNAME="subutai-control-center-dev-unsigned.pkg"
		PKGNAME_S="subutai-control-center-dev.pkg"
		;;
	master)
		PKGNAME="subutai-control-center-master-unsigned.pkg"
		PKGNAME_S="subutai-control-center-master.pkg"
		;;
	head)
		PKGNAME="subutai-control-center-unsigned.pkg"   
		PKGNAME_S="subutai-control-center.pkg"
    	;;
    HEAD)
		PKGNAME="subutai-control-center.pkg"
		PKGNAME_S="subutai-control-center.pkg"
    	;;  
esac

cd darwin-packages/
rm -rf flat
rm -rf root

./pack.sh ../subutai_control_center_bin/SubutaiControlCenter.app $BRANCH
security default-keychain -s build.keychain
productsign --sign YM8SD62476 $PKGNAME $PKGNAME_S
mv $PKGNAME_S ../subutai_control_center_bin/
