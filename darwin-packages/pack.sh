#!/bin/bash

sstray=$1
# clean
rm -rf flat
rm -rf root
rm -f *.pkg
version=`cat ../VERSION`
# Copy files
mkdir -p flat/Resources/en.lproj
mkdir -p flat/base.pkg
mkdir -p root/Applications
cp -r $sstray root/Applications/

# Determine sizes and modify PackageInfo
rootfiles=`find root | wc -l`
rootsize=`du -b -s root | awk '{print $1}'`
mbsize=$(( ${rootsize%% *} / 1024 ))

echo "Size: $rootsize"
echo "MBSize: $mbsize"

cp ./PackageInfo.tmpl ./flat/base.pkg/PackageInfo
sed -i -e "s/{VERSION_PLACEHOLDER}/$version/g" ./flat/base.pkg/PackageInfo
sed -i -e "s/{SIZE_PLACEHOLDER}/$mbsize/g" ./flat/base.pkg/PackageInfo
sed -i -e "s/{FILES_PLACEHOLDER}/$rootfiles/g" ./flat/base.pkg/PackageInfo

# modify Distribution
cp ./Distribution.tmpl ./flat/Distribution
sed -i -e "s/{VERSION_PLACEHOLDER}/$version/g" ./flat/Distribution
sed -i -e "s/{SIZE_PLACEHOLDER}/$mbsize/g" ./flat/Distribution

# Pack and bom
( cd root && find . | cpio -o --format odc --owner 0:80 | gzip -c ) > flat/base.pkg/Payload
( cd scripts && find . | cpio -o --format odc --owner 0:80 | gzip -c ) > flat/base.pkg/Scripts
mkbom -u 0 -g 80 root flat/base.pkg/Bom
( cd flat && xar --compression none -cf "../SubutaiTray-$version-Installer.pkg" * )
