set arg_vboxrt_dll=%1
if exist "subutai_tray_bin" ( 
  echo "Try to remove subutai_tray_bin"
  rd /Q /S "subutai_tray_bin"
)

md subutai_tray_bin
cd subutai_tray_bin
qmake ..\SubutaiTray.pro -r -spec win32-msvc2013
jom
cd ..
copy "libssh2\lib\win32\libssh2.dll" "subutai_tray_bin\release\"

cd subutai_tray_bin\release
del *.obj
del *.cpp
del *.h
del *.moc
windeployqt --release --no-translations --compiler-runtime SubutaiTray.exe
copy %arg_vboxrt_dll% . && echo "SUCCESS"
cd ../..
