call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

git checkout -- .
git checkout %APPVEYOR_REPO_BRANCH%
git pull origin %APPVEYOR_REPO_BRANCH%
git pull origin %APPVEYOR_REPO_BRANCH% --tags

xcopy "C:\SRC\control-center\ssh" "C:\SRC\ssh" /S /Y /E /I /H
xcopy "C:\SRC\control-center\SubutaiTraySetup" "C:\SRC\SubutaiTraySetup" /S /Y /E /I /H
copy "C:\SRC\control-center\addvariablesshtopath.vbs" C:\SRC\addvariablesshtopath.vbs

for /R C:\Qt\5.10.1\msvc2017_64\plugins\bearer\ %%f in (*.dll) do copy "%%f" c:\SRC\control-center\dll\
for /R C:\Qt\5.10.1\msvc2017_64\plugins\imageformats\ %%f in (*.dll) do copy "%%f" c:\SRC\control-center\dll\
for /R C:\Qt\5.10.1\msvc2017_64\plugins\platforms\ %%f in (*.dll) do copy "%%f" c:\SRC\control-center\dll\
for /R C:\Qt\5.10.1\msvc2017_64\plugins\styles\ %%f in (*.dll) do copy "%%f" c:\SRC\control-center\dll\
for /R C:\Qt\5.10.1\msvc2017_64\bin\ %%f in (*.dll) do copy "%%f" c:\SRC\control-center\dll\
for /R C:\Qt\5.10.1\msvc2017_64\plugins\iconengines\ %%f in (*.dll) do copy "%%f" c:\SRC\control-center\dll\

set arg_vboxrt_dll="c:\SRC\control-center\dll\*.dll"

if exist "subutai_control_center_bin" (
 
  echo "Try to remove subutai_control_center_bin"

   rd /Q /S "subutai_control_center_bin"

)

md subutai_control_center_bin

cd subutai_control_center_bin

lrelease ..\SubutaiControlCenter.pro
qmake ..\SubutaiControlCenter.pro -r -spec win32-msvc

jom

cd ..

copy "libssh2\lib\win32\libssh2.dll" "subutai_control_center_bin\release\"
copy "C:\Qt\5.10.1\msvc2017_64\plugins\bearer\qnativewifibearer.dll" C:\SRC\control-center\subutai_control_center_bin\release\bearer\qnativewifibearer.dll

copy "*.qm" "subutai_control_center_bin\release\"
cd subutai_control_center_bin\release

del *.obj

del *.cpp

del *.h

del *.moc

signtool.exe sign /tr http://timestamp.digicert.com /td sha256 /fd sha256 /f C:\devops\signing.p12 /a SubutaiControlCenter.exe

windeployqt --release --no-translations --compiler-runtime SubutaiControlCenter.exe

copy %arg_vboxrt_dll% . && echo "SUCCESS"

cd /
move "C:\SRC\control-center\subutai_control_center_bin\release" "C:\SRC\control-center\subutai_control_center_bin\SubutaiControlCenter"
del C:\SRC\control-center\subutai_control_center_bin\SubutaiControlCenter\vcredist_x64.exe
cd C:\SRC\control-center
del c:\tray_builds\*.* /s /q
rmdir c:\tray_builds\SubutaiTray /s /q
7z a -r C:\tray_builds\SubutaiTray_libs.zip "C:\SRC\control-center\subutai_control_center_bin\SubutaiControlCenter"
copy "subutai_control_center_bin\SubutaiControlCenter\SubutaiControlCenter.exe"  C:\SRC\control-center\subutai_control_center_bin\SubutaiControlCenter.exe
xcopy "subutai_control_center_bin\SubutaiControlCenter" "C:\tray_builds\SubutaiTray" /S /Y /E /I /H

copy "C:\SRC\control-center\dll\qnativewifibearer.dll" C:\tray_builds\SubutaiTray\bearer\qnativewifibearer.dll
cd C:\SRC\SubutaiTraySetup\
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\devenv.com" SubutaiControlCenterSetup.sln /Rebuild Release
dir C:\SRC\SubutaiTraySetup\SubutaiTraySetup\Release\

if "%APPVEYOR_REPO_BRANCH%" == "dev" (
  copy "C:\SRC\SubutaiTraySetup\SubutaiTraySetup\Release\SubutaiControlCenter.msi" "C:\SRC\control-center\subutai_control_center_bin\subutai-control-center-dev.msi"  
)

if "%APPVEYOR_REPO_BRANCH%" == "master" (
  copy "C:\SRC\SubutaiTraySetup\SubutaiTraySetup\Release\SubutaiControlCenter.msi" "C:\SRC\control-center\subutai_control_center_bin\subutai-control-center-master.msi"  
)

if "%APPVEYOR_REPO_BRANCH%" == "head" (
  copy "C:\SRC\SubutaiTraySetup\SubutaiTraySetup\Release\SubutaiControlCenter.msi" "C:\SRC\control-center\subutai_control_center_bin\subutai-control-center.msi"  
)

if "%APPVEYOR_REPO_BRANCH%" == "HEAD" (
  copy "C:\SRC\SubutaiTraySetup\SubutaiTraySetup\Release\SubutaiControlCenter.msi" "C:\SRC\control-center\subutai_control_center_bin\subutai-control-center.msi"  
)
