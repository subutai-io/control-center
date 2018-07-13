BRANCH=$1
export MAINTAINER="Jenkins Admin"
export MAINTAINER_EMAIL="jenkins@subut.ai"
#export TRAYDIR=/tmp/jenkins_slave/workspace/tag.builders/tray_linux.subutai-io.tag
export QTBINPATH=/home/builder/qt_static/bin/
export cc_version="$(git describe --abbrev=0 --tags)+$(date +%Y%m%d%H%M%S)"
export workspace="$(pwd)"
echo $workspace
rm -rf /home/builder/deb_repo/
mkdir /home/builder/deb_repo

cd $workspace
qmake --version
./generate_changelog --maintainer="$MAINTAINER" --maintainer-email="$MAINTAINER_EMAIL"

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

case $BRANCH in
	dev)
		PKGNAME="subutai-control-center-dev"
		;;
	master)
		PKGNAME="subutai-control-center-master"
		;;
esac

cd deb-packages/deb-packages-internal
./clear.sh
./pack_debian.sh 
cd ../..

cd /home/builder/deb_repo
ls
cp $workspace/deb-packages/*.deb .
cp $workspace/deb-packages/deb-packages-internal/debian/SubutaiControlCenter/bin/subutai-control-center .


mv *.deb $PKGNAME_$cc_version.deb
mv subutai-control-center SubutaiControlCenter
