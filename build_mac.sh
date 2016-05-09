subutai_tray_bin="subutai_tray_bin"
libssh_app_bin="libssh_app_bin"

if [ -d "$subutai_tray_bin" ]; then 
	echo "Try to remove subutai_tray_bin"
  rm -rf subutai_tray_bin
fi 
mkdir subutai_tray_bin
cd subutai_tray_bin
qmake ../SubutaiTray.pro -r -spec macx-clang CONFIG+=x86_64
make 
cd ../
if [ -d "$libssh_app_bin" ]; then
	echo "Try to remove libssh_app_bin"
	rm -rf libssh_app_bin
fi
mkdir libssh_app_bin
cd libssh_app_bin
qmake ../libssh2/libssh2_app.pro -r -spec macx-clang CONFIG+=x86_64
make 
cd ../
cp libssh_app_bin/libssh2_app subutai_tray_bin/SubutaiTray.app/Contents/MacOS/
macdeployqt subutai_tray_bin/SubutaiTray.app
cd subutai_tray_bin/SubutaiTray.app/Contents/MacOS/
source ../../../../after_build_step_mac_os 
