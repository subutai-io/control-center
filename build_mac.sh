#!/bin/bash
wget -nv -O /Users/travis/build/subutai-io/control-center/libs.tar.gz 'https://masterbazaar.subutai.io/rest/v1/cdn/raw?name=libs.tar.gz&latest&download' && tar -xf libs.tar.gz
wget -nv -O /Users/dev/Qt5.9.2.tar.gz 'http://62.138.0.225/ipfs/QmcefrkUMyjpK8E6C1h44voqDAkFz1YjkrxMEt9ys11RWZ/Qt5.9.2.tar.gz' && tar -xf /Users/dev/Qt5.9.2.tar.gz -C /Users/dev
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
