set arg_vboxrt_dll=%1
echo "VBoxRT.dll path : %arg_vboxrt_dll%"

if exist "subutai_tray_bin" ( 
  echo "Try to remove subutai_tray_bin"
  rd /Q /S "subutai_tray_bin"
)

md subutai_tray_bin
cd subutai_tray_bin
qmake ..\SubutaiTray.pro -r -spec win32-msvc2013
jom

cd ..
if exist "libssh_app_bin" (
  echo "Try to remove libssh_app_bin"
  rd /Q /S "libssh_app_bin""
)

md libssh_app_bin
cd libssh_app_bin
qmake ..\libssh2\libssh2_app.pro -r -spec win32-msvc2013
jom
cd ..
copy "libssh_app_bin\release\libssh2_app.exe" "subutai_tray_bin\release\"
copy "libssh2\lib\win32\libssh2.dll" "subutai_tray_bin\release\"

cd subutai_tray_bin\release
del *.obj
del *.cpp
del *.h
del *.moc
windeployqt --release --no-translations --compiler-runtime SubutaiTray.exe
if exist %arg_vboxrt_dll% (
  copy %arg_vboxrt_dll% .
) else (
  echo "Can't find specified VBoxRT.dll. Copy it to destination folder manual!"
)
cd ../..
