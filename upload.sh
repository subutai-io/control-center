BRANCH=$1
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
    echo "Obtaining auth id..."

    curl -k "$2/auth/token?user=$USER" -o /tmp/filetosign
    rm -rf /tmp/filetosign.asc
    gpg --armor -u $EMAIL --clearsign /tmp/filetosign

    SIGNED_AUTH_ID=$(cat /tmp/filetosign.asc)

    echo "Auth id obtained and signed\\n$SIGNED_AUTH_ID"

    TOKEN=$(curl -k -s -Fmessage="$SIGNED_AUTH_ID" -Fuser=$USER "$2/auth/token")

    echo "Token obtained $TOKEN"

    echo "Uploading file..."

    ID=$(curl -sk -H "token: $TOKEN" -Ffile=@$1 -Ftoken=$TOKEN "$2/raw/upload")

    echo "File uploaded with ID $ID"
    echo "URL: $2"
    echo "Signing file..."

    SIGN=$(echo $ID | gpg --clearsign --no-tty -u $EMAIL)

    curl -ks -Ftoken="$TOKEN" -Fsignature="$SIGN" "$2/auth/sign"

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
        BINNAME="SubutaiControlCenter$BINARY_EXT"
        URL=https://devcdn.subutai.io:8338/kurjun/rest
        upload_cdn subutai_control_center_bin/$PKGNAME $URL
        upload_cdn subutai_control_center_bin/$BINNAME $URL
        upload_cdn subutai_control_center_bin/$PKGNAME https://cdn.subutai.io:8338/kurjun/rest
        ;;
    master)
        PKGNAME="subutai-control-center-master$PKG_EXT"
        BINNAME="SubutaiControlCenter$BINARY_EXT"
        URL=https://mastercdn.subutai.io:8338/kurjun/rest
        upload_cdn subutai_control_center_bin/$PKGNAME $URL
        upload_cdn subutai_control_center_bin/$BINNAME $URL
        upload_cdn subutai_control_center_bin/$PKGNAME https://cdn.subutai.io:8338/kurjun/rest
        ;;
    head)
        PKGNAME="subutai-control-center$PKG_EXT"
        BINNAME="SubutaiControlCenter$BINARY_EXT"
        URL=https://cdn.subutai.io:8338/kurjun/rest
        upload_cdn subutai_control_center_bin/$PKGNAME $URL
        upload_cdn subutai_control_center_bin/$BINNAME $URL
        ;;
    HEAD)
        PKGNAME="subutai-control-center$PKG_EXT"
        BINNAME="SubutaiControlCenter$BINARY_EXT"
        URL=https://cdn.subutai.io:8338/kurjun/rest
        upload_cdn subutai_control_center_bin/$PKGNAME $URL
        upload_cdn subutai_control_center_bin/$BINNAME $URL
        ;;
esac

echo "---------"
echo $PKGNAME
echo $BINNAME
echo $OS
echo $URL
echo "---------"