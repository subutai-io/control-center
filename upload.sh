BRANCH=$1
VERSION=$(cat version)
OS=$(uname)
PKGNAME=""
BINNAME=""
FILE=""
URL=""
PKG_EXT=""
BINARY_EXT=""
USER=jenkins
EMAIL=jenkins@subut.ai

upload_cdn (){
    filename=$1
    user="jenkins@subut.ai"
    fingerprint="7CD0CB4AAA727A884B2B811918B54AF8076EEE5B"
    cdnHost=$2
    authId="$(curl -s https://${cdnHost}/rest/v1/cdn/token?fingerprint=${fingerprint})"
    echo "Auth id obtained and signed $authId"

    sign="$(echo ${authId} | gpg --clearsign -u ${user})"
    token="$(curl -s --data-urlencode "request=${sign}"  https://${cdnHost}/rest/v1/cdn/token)"
    echo "Token obtained $token"

    echo "Uploading file..."
    curl -sk -H "token: ${token}" -Ffile=@$filename -Ftoken=${token} -X POST "https://${cdnHost}/rest/v1/cdn/uploadRaw"

    echo -e "\\nCompleted"
}

case $OS in
    Linux)
        PKG_EXT=".deb"
        ;;
    MSYS_NT-10.0)
        PKG_EXT=".msi"
        BINARY_EXT=".exe"         
        ;;
    Darwin)
        PKG_EXT=".pkg"
        BINARY_EXT="_osx"
        ;;
esac

case $BRANCH in
    dev)
        PKGNAME="subutai-control-center-dev$PKG_EXT"
        BINNAME="SubutaiControlCenter-dev$BINARY_EXT"
        mv subutai_control_center_bin/SubutaiControlCenter$BINARY_EXT subutai_control_center_bin/$BINNAME
        URL=https://devbazaar.subutai.io
        upload_cdn subutai_control_center_bin/$PKGNAME $URL 
        upload_cdn subutai_control_center_bin/$BINNAME $URL
        ;;
    master)
        PKGNAME="subutai-control-center-master$PKG_EXT"
        BINNAME="SubutaiControlCenter-master$BINARY_EXT"
        mv subutai_control_center_bin/SubutaiControlCenter$BINARY_EXT subutai_control_center_bin/$BINNAME
        URL=https://masterbazaar.subutai.io
        upload_cdn subutai_control_center_bin/$PKGNAME $URL
        upload_cdn subutai_control_center_bin/$BINNAME $URL
        ;;
    head)
        PKGNAME="subutai-control-center$PKG_EXT"
        BINNAME="SubutaiControlCenter$BINARY_EXT"
        if [ $OS = Linux ]
        then
        URL=https://bazaar.subutai.io
        upload_cdn subutai_control_center_bin/$PKGNAME $URL
        upload_cdn subutai_control_center_bin/$BINNAME $URL
        fi
        ;;
    HEAD)
        PKGNAME="subutai-control-center$PKG_EXT"
        BINNAME="SubutaiControlCenter$BINARY_EXT"
        if [ $OS = Linux ]
        then
        URL=https://bazaar.subutai.io
        upload_cdn subutai_control_center_bin/$PKGNAME $URL
        upload_cdn subutai_control_center_bin/$BINNAME $URL
        fi
        ;;
esac

echo "---------"
echo $PKGNAME
echo $BINNAME
echo $OS
echo $BRANCH
echo $VERSION
echo "---------"
