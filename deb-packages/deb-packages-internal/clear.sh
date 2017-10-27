#!/bin/bash

rm debian/subutai-tray.*
rm debian/debhelper-build-stamp
rm -rf debian/SubutaiTray.tar.gz
rm -rf debian/icons
rm -rf debian/SubutaiTray
rm -rf debian/subutai-tray
rm -rf debian/libicu52

for f in $(ls ../); do
	if [ "$f" != "deb-packages-internal" ]; then 
    rm ../$f
	fi
done
