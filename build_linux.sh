nproc_count="$(nproc)"
core_number=$((nproc_count*2+1))
subutai_tray_bin="subutai_tray_bin"
libssh_app_bin="libssh_app_bin"

if [ -d "$subutai_tray_bin" ]; then 
	echo "Try to remove subutai_tray_bin"
  rm -rf subutai_tray_bin
fi 
mkdir subutai_tray_bin
cd subutai_tray_bin
qmake ../SubutaiTray.pro -r -spec linux-g++
make -j$core_number 
cd ../
if [ -d "$libssh_app_bin" ]; then
	echo "Try to remove libssh_app_bin"
	rm -rf libssh_app_bin
fi
mkdir libssh_app_bin
cd libssh_app_bin
qmake ../libssh2/libssh2_app.pro -r -spec linux-g++
make -j$core_number
cd ../
cp libssh_app_bin/libssh2_app subutai_tray_bin/
