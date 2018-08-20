tar czf /Users/travis/build/subutai-io/control-center/SubutaiControlCenter.tar.gz /Users/travis/build/subutai-io/control-center/subutai_control_center_bin/SubutaiControlCenter.app
echo "Uploading SubutaiControlCenter.app"
BRANCH=$1
FILE=""
URL=""
USER=jenkins
EMAIL=jenkins@subut.ai

upload_cdn (){
    filename=$1
    user="jenkins@subut.ai"
    fingerprint="7CD0CB4AAA727A884B2B811918B54AF8076EEE5B"
    cdnHost=$2

    extract_id()
        {
            id_src=$(echo $json | grep -Po '"id" : ".*?[^\\]"')
            id=${id_src:8:46}
        }       

    json=`curl -k -s -X GET https://$cdnHost/rest/v1/cdn/raw?name=$filename`
    echo "Received: $json"
    extract_id
    echo "Previous file ID is $id"

    authId="$(curl -s https://${cdnHost}/rest/v1/cdn/token?fingerprint=${fingerprint})"
    echo "Auth id obtained and signed $authId"

    sign="$(echo ${authId} | gpg --clearsign -u ${user})"
    token="$(curl -s --data-urlencode "request=${sign}"  https://${cdnHost}/rest/v1/cdn/token)"
    echo "Token obtained $token"

    echo "Uploading file..."
    curl -sk -H "token: ${token}" -Ffile=@$filename -Ftoken=${token} -X POST "https://${cdnHost}/rest/v1/cdn/uploadRaw"

    echo "Removing previous"
    if [[ -z "$id" ]]; then
        echo "File not found"
    else curl -k -s -X DELETE "$cdnHost/rest/v1/cdn/raw?token=${token}&id=$id"
    fi

    echo -e "\\nCompleted"
}
FILE="/Users/travis/build/subutai-io/control-center/SubutaiControlCenter.tar.gz"
URL=https://devcdn.subutai.io:8338/kurjun/rest
if [[ $BRANCH != "master" ]] && [[ $BRANCH != "dev" ]]
then
upload_cdn $FILE $URL
fi

echo "---------"
echo $FILE
echo $BRANCH
echo "---------"
