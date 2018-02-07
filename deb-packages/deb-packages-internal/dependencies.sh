#!/bin/bash 
# Author : Hemanth.HM
# Email : hemanth[dot]hm[at]gmail[dot]com
# License : GNU GPLv3
#

#Check if the paths are vaild
[[ ! -e $1 ]] && echo "Not a vaild input $1" && exit 1 
[[ -d $2 ]] || echo "No such directory $2 creating..."&& mkdir -p "$2"

#Get the library dependencies
echo "Collecting the shared library dependencies for $1..."
deps=$(ldd $1 | grep "libicu\|libinput" | awk 'BEGIN{ORS=" "}$1~/^\//{print $1}$3~/^\//{print $3}' | sed 's/,$/\n/')
echo "Copying the dependencies to $2"
#Copy the deps
for dep in $deps; do
	 echo "Copying $dep to $2"
	 cp "$dep" "$2"
done

libinput=$(find /usr/lib . -name 'libinput.so.5')
if [ -e "$libinput" ]; then
  cp "$libinput" "$2"
fi

libssl=$(find /usr/lib . -name 'libssl.so.1.0.0')
if [ -e "$libssl" ]; then
  cp "$libssl" "$2"
fi

echo "Done!"
