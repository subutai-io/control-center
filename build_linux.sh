mkdir subutai_tray_bin
cd subutai_tray_bin
qmake ../SubutaiTray.pro -r -spec linux-g++
make 
cd ../
mkdir libssh_app_bin
cd libssh_app_bin
qmake ../libssh2/libssh2_app.pro -r -spec linux-g++
make
cd ../
cp libssh_app_bin/libssh2_app subutai_tray_bin/
