#!/bin/bash

./clear.sh
mkdir -p debian/icons
mkdir -p debian/SubutaiControlCenter/bin
mkdir -p debian/SubutaiControlCenter/lib
mkdir -p debian/SubutaiControlCenter/Libs

cp $PWD/../../resources/cc_icon.png debian/icons

cp ../../subutai_control_center_bin/SubutaiControlCenter debian/SubutaiControlCenter/bin/subutai-control-center
cp ../../subutai_control_center_bin/*.qm debian/SubutaiControlCenter/bin/
./dependencies.sh ../../subutai_control_center_bin/SubutaiControlCenter debian/SubutaiControlCenter/Libs/

cd debian
tar zcvf SubutaiControlCenter.tar.gz SubutaiControlCenter
cd ..

#debuild -B -d
debuild
