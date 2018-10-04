tar czf /Users/travis/build/subutai-io/control-center/SubutaiControlCenter.tar.gz /Users/travis/build/subutai-io/control-center/subutai_control_center_bin/SubutaiControlCenter.app
echo "Uploading SubutaiControlCenter.app"
BRANCH=$1
FILE=""
URL=""

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

    json=`curl -k -s -X GET ${cdnHost}/rest/v1/cdn/raw?name=$filename&latest`
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

FILE="/Users/travis/build/subutai-io/control-center/SubutaiControlCenter.tar.gz"
URL=https://devbazaar.subutai.io
if [[ $BRANCH != "master" ]] && [[ $BRANCH != "dev" ]]
then
upload_ipfs $FILE $URL
fi

echo "---------"
echo $FILE
echo $BRANCH
echo "---------"