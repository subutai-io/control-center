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
    echo "Obtaining auth id..."

    curl -k "$2/auth/token?user=$USER" -o /tmp/filetosign
    rm -rf /tmp/filetosign.asc
    gpg --armor -u $EMAIL --clearsign /tmp/filetosign

    SIGNED_AUTH_ID=$(cat /tmp/filetosign.asc)

    echo "Auth id obtained and signed\\n$SIGNED_AUTH_ID"

    TOKEN=$(curl -k -s -Fmessage="$SIGNED_AUTH_ID" -Fuser=$USER "$2/auth/token")

    echo "Token obtained $TOKEN"

    echo "Uploading file..."

    ID=$(curl -sk -H "token: $TOKEN" -Ffile=@$1 -Fversion=$3 -Ftoken=$TOKEN "$2/raw/upload")

    echo "File uploaded with ID $ID"
    echo "URL: $2"
    echo "VERSION: $3"
    echo "Signing file..."

    SIGN=$(echo $ID | gpg --clearsign --no-tty -u $EMAIL)

    curl -ks -Ftoken="$TOKEN" -Fsignature="$SIGN" "$2/auth/sign"

    echo -e "\\nCompleted"
}
upload_ipfs (){
    filename=$1
    user="jenkins@optimal-dynamics.com"
    fingerprint="877B586E74F170BC4CF6ECABB971E2AC63D23DC9"
    cdnHost=$2
    echo $filename
    extract_id()
        {
            id_src=$(echo $json | grep "id")
            id=${id_src:10:46}
        }       

    json=`curl -k -s -X GET ${cdnHost}/rest/v1/cdn/raw?name=$filename`
    echo "Received: $json"
    extract_id
    echo "Previous file ID is $id"

    authId="$(curl -s ${cdnHost}/rest/v1/cdn/token?fingerprint=${fingerprint})"
    echo "Auth id obtained and signed $authId"

    sign="$(echo ${authId} | gpg --clearsign -u ${user})"
    token="$(curl -s --data-urlencode "request=${sign}"  ${cdnHost}/rest/v1/cdn/token)"
    echo "Token obtained $token"

    echo "Uploading file..."
    upl_msg="$(curl -sk -H "token: ${token}" -Ffile=@$filename -Ftoken=${token} -X POST "${cdnHost}/rest/v1/cdn/uploadRaw")"
    echo "$upl_msg"

    echo "Removing previous"
    echo $Upload
    if [[ -n "$id" ]] && [[ $upl_msg != "An object with id: $id is exist in Bazaar. Increase the file version." ]]
    then
        curl -k -s -X DELETE "$cdnHost/rest/v1/cdn/raw?token=${token}&id=$id"
    fi
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
        IPFSBIN="SubutaiControlCenter-dev$BINARY_EXT"
        cp subutai_control_center_bin/SubutaiControlCenter$BINARY_EXT subutai_control_center_bin/$IPFSBIN
        cd subutai_control_center_bin
        IPFSURL=https://devbazaar.subutai.io
        URL=https://devcdn.subutai.io:8338/kurjun/rest
        upload_ipfs $PKGNAME $IPFSURL
        upload_ipfs $IPFSBIN $IPFSURL
#        upload_cdn $PKGNAME $URL $VERSION
#        upload_cdn $BINNAME $URL $VERSION
#        upload_cdn $PKGNAME https://cdn.subutai.io:8338/kurjun/rest $VERSION
        ;;
    master)
        PKGNAME="subutai-control-center-master$PKG_EXT"
        BINNAME="SubutaiControlCenter$BINARY_EXT"
        IPFSBIN="SubutaiControlCenter-master$BINARY_EXT"
        cp subutai_control_center_bin/SubutaiControlCenter$BINARY_EXT subutai_control_center_bin/$IPFSBIN
        cd subutai_control_center_bin
        IPFSURL=https://masterbazaar.subutai.io
        URL=https://mastercdn.subutai.io:8338/kurjun/rest
        upload_ipfs $PKGNAME $IPFSURL
        upload_ipfs $IPFSBIN $IPFSURL
        ;;
    head)
        PKGNAME="subutai-control-center$PKG_EXT"
        BINNAME="SubutaiControlCenter$BINARY_EXT"

        ;;
    HEAD)
        PKGNAME="subutai-control-center$PKG_EXT"
        BINNAME="SubutaiControlCenter$BINARY_EXT"

        ;;
esac

echo "---------"
echo $PKGNAME
echo $BINNAME
echo $OS
echo $BRANCH
echo $VERSION
echo "---------"