#!/bin/bash

rm debian/subutai-control-center.*
rm debian/debhelper-build-stamp
rm -rf debian/SubutaiControlCenter.tar.gz
rm -rf debian/icons
rm -rf debian/SubutaiControlCenter
rm -rf debian/subutai-control-center
rm -rf debian/libicu52

for f in $(ls ../); do
	if [ "$f" != "deb-packages-internal" ]; then 
    rm ../$f
	fi
done
