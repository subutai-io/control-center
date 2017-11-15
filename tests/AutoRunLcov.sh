lcov --no-external --directory . --directory ../tray/commons --directory ../tray/libssh2 --directory ../tray/hub --directory ../tray/commons --directory ../tray/vbox --capture --output-file app.info 
lcov --remove app.info '/*' -o app-new.info
genhtml app-new.info
google-chrome ./index.html


