tar czf /Users/travis/build/subutai-io/control-center/SubutaiControlCenter.tar.gz /Users/travis/build/subutai-io/control-center/subutai_control_center_bin/SubutaiControlCenter.app
echo "Uploading SubutaiControlCenter.app"
BRANCH=$1
FILE=""
URL=""
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
FILE="/Users/travis/build/subutai-io/control-center/SubutaiControlCenter.tar.gz"
URL=https://devcdn.subutai.io:8338/kurjun/rest
if [[ $BRANCH = "HEAD" ]] || [[ $BRANCH = "head" ]]
then
upload_cdn $FILE $URL
fi

echo "---------"
ECHO $FILE
echo $OS
echo $BRANCH
echo "---------"
