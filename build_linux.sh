#!/bin/bash

QTBINPATH=/home/builder/qt_static/bin
BRANCH=$1
TRAYDIR=/home/travis/build/subutai-io/control-center
MAINTAINER="Jenkins Admin"
MAINTAINER_EMAIL="jenkins@subut.ai"
PROD_URL=https://cdn.subutai.io:8338/kurjun/rest
#QTBINPATH=/home/travis/build/subutai-io/qt_static/bin/:$QTBINPATH
export PATH=$QTBINPATH:$PATH
qmake --version
git checkout -- .
git pull
git checkout $BRANCH
git pull origin $BRANCH

./generate_changelog --maintainer="$MAINTAINER" --maintainer-email="$MAINTAINER_EMAIL"
#./build_linux.sh "$QTBINPATH" "$BRANCH" "$TRAYDIR" "$MAINTAINER" "$MAINTAINER_EMAIL"

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

PKGNAME="subutai-control-center.deb"
case $BRANCH in
	dev)
		PKGNAME="subutai-control-center-dev.deb"
		;;
	master)
		PKGNAME="subutai-control-center-master.deb"
		;;
  head)
		PKGNAME="subutai-control-center.deb" 
    ;;
  HEAD)
		PKGNAME="subutai-control-center.deb"   
    ;;    
esac

if [ $? -eq 0 ]; then
  cd deb-packages/deb-packages-internal
  ./clear.sh
  ./pack_debian.sh 
  cd ../..
#todo upload *.deb as $PKGNAME to kurjun
  cd ..
else
  echo "FAILED TO BUILD"
  exit 1 
fi

wd=./
if [ ! -z "$TRAVIS" ]; then
    wd=$HOME/build/subutai-io/control-center
fi
cd $wd/subutai_control_center_bin
cp $wd/deb-packages/*.deb $wd/subutai_control_center_bin/$PKGNAME
cp $wd/deb-packages/*.deb /tmp/$PKGNAME
cp $wd/control-center/deb-packages/deb-packages-internal/debian/SubutaiControlCenter/bin/subutai-control-center .
cp $wd/control-center/deb-packages/deb-packages-internal/debian/SubutaiControlCenter/bin/subutai-control-center /tmp/SubutaiControlCenter
mv subutai-control-center SubutaiControlCenter
