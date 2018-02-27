set arg_vboxrt_dll=%1

if exist "subutai_control_center_bin" (
 
  echo "Try to remove subutai_control_center_bin"

   rd /Q /S "subutai_control_center_bin"

)



md subutai_control_center_bin

cd subutai_control_center_bin

qmake ..\SubutaiControlCenter.pro -r -spec win32-msvc

jom

cd ..

copy "libssh2\lib\win32\libssh2.dll" "subutai_control_center_bin\release\"


cd subutai_control_center_bin\release

del *.obj

del *.cpp

del *.h

del *.moc

windeployqt --release --no-translations --compiler-runtime SubutaiControlCenter.exe

copy %arg_vboxrt_dll% . && echo "SUCCESS"

cd ../..
