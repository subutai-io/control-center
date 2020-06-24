#!/bin/bash

BRANCH=$1
TRAYDIR=/Users/travis/build/subutai-io/control-center/

codesign -s YM8SD62476 --keychain build.keychain --deep ./SubutaiControlCenter
cp SubutaiControlCenter SubutaiControlCenter_osx
cp SubutaiControlCenter_osx ../../../.

cd $TRAYDIR

PKGNAME="subutai-control-center-unsigned.pkg"
PKGNAME_S="subutai-control-center.pkg"
case $BRANCH in
	dev)
		PKGNAME="subutai-control-center-dev-unsigned.pkg"
		PKGNAME_S="subutai-control-center-dev.pkg"
		;;
	master)
		PKGNAME="subutai-control-center-master-unsigned.pkg"
		PKGNAME_S="subutai-control-center-master.pkg"
		;;
esac

cd darwin-packages/
rm -rf flat
rm -rf root

./pack.sh ../subutai_control_center_bin/SubutaiControlCenter.app $BRANCH
productsign --sign YM8SD62476 --keychain build.keychain $PKGNAME $PKGNAME_S
mv $PKGNAME_S ../subutai_control_center_bin/
