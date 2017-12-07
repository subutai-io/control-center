cd ..
sudo mkdir coverage_testing_file
cd coverage_testing_file

sudo lcov --no-external --directory ../build-TestingTray --directory ../tray/commons --directory ../tray/libssh2 --directory ../tray/hub --directory ../tray/commons --directory ../tray/vbox --capture --output-file app.info 
sudo lcov --remove app.info '../build-TestingTray/*' --output-file app-new.info
sudo genhtml app-new.info
google-chrome ./index.html


