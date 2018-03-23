#!/bin/bash

QTBINPATH=/home/builder/qt_static/bin
BRANCH=$1
TRAYDIR=/home/travis/build/tasankulov/control-center
MAINTAINER="Jenkins Admin"
MAINTAINER_EMAIL="jenkins@subut.ai"
PROD_URL=https://cdn.subutai.io:8338/kurjun/rest
#QTBINPATH=/home/travis/build/tasankulov/qt_static/bin/:$QTBINPATH
export PATH=$QTBINPATH:$PATH
qmake --version
git checkout -- .
git pull
git checkout $BRANCH
git pull origin $BRANCH

./configure --maintainer="$MAINTAINER" --maintainer-email="$MAINTAINER_EMAIL"
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

cd /home/travis/build/tasankulov/control-center/subutai_control_center_bin
cp /home/travis/build/tasankulov/control-center/deb-packages/*.deb /home/travis/build/tasankulov/control-center/subutai_control_center_bin/$PKGNAME
cp /home/travis/build/tasankulov/control-center/deb-packages/deb-packages-internal/debian/SubutaiControlCenter/bin/subutai-control-center .
mv subutai-control-center SubutaiControlCenter

