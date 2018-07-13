#!groovy

notifyBuildDetails = ""

try {
	notifyBuild('STARTED')
	switch (env.BRANCH_NAME) {
            case ~/master/: 
            PKGNAME = "subutai-control-center-master"
            break;
	    	default: 
            PKGNAME = "subutai-control-center-dev"
    }
	
	node("debian") {

		stage("Start build Debian")
		
		notifyBuildDetails = "\nFailed on Stage - Start build"
		checkout scm
		def workspace = pwd()
        String date = new Date().format( 'yyyyMMddHHMMSS' )
		String plain_version = sh (script: """
					cat ${workspace}/version | tr -d '\n'
					""", returnStdout: true)
        def cc_version = "${plain_version}+${date}"

		sh """
		export MAINTAINER="Jenkins Admin"
        export MAINTAINER_EMAIL="jenkins@subut.ai"
		export QTBINPATH=/home/builder/qt_static/bin/
		qmake --version
        ./generate_changelog --maintainer="${MAINTAINER}" --maintainer-email="${MAINTAINER_EMAIL}"
		nproc_count="${(nproc)}"
        core_number=${((nproc_count*2+1))}
        subutai_control_center_bin="subutai_control_center_bin"
		if [ -d "${subutai_control_center_bin}" ]; then 
	    echo "Try to remove subutai_control_center_bin"
        rm -rf subutai_control_center_bin
        fi 
        mkdir subutai_control_center_bin
        cd subutai_control_center_bin
        lrelease ../SubutaiControlCenter.pro
        qmake ../SubutaiControlCenter.pro -r -spec linux-g++
        make -j${core_number} 
        rm *.o *.cpp *.h
        mv ../*.qm .
        cd ../
        cd deb-packages/deb-packages-internal
        ./clear.sh
        ./pack_debian.sh 
        cd ../..
        rm -r /home/builder/deb_repo/*
        cd /home/builder/deb_repo
        cp ${workspace}/deb-packages/*.deb .
        cp ${workspace}/deb-packages/deb-packages-internal/debian/SubutaiControlCenter/bin/subutai-control-center .
        mv *.deb ${PKGNAME}_${cc_version}.deb
        mv subutai-control-center SubutaiControlCenter
		"""

		stage("Upload")

		notifyBuildDetails = "\nFailed on Stage - Upload"

		sh """
		cd /home/builder/deb_repo
		touch uploading_agent
		scp uploading_agent subutai*.deb dak@deb.subutai.io:incoming/${env.BRANCH_NAME}/
		ssh dak@deb.subutai.io sh /var/reprepro/scripts/scan-incoming.sh ${env.BRANCH_NAME} agent
		"""
	}

} catch (e) { 
	currentBuild.result = "FAILED"
	throw e
} finally {
	// Success or failure, always send notifications
	notifyBuild(currentBuild.result, notifyBuildDetails)
}

def notifyBuild(String buildStatus = 'STARTED', String details = '') {
  // build status of null means successful
  buildStatus = buildStatus ?: 'SUCCESSFUL'

  // Default values
  def colorName = 'RED'
  def colorCode = '#FF0000'
  def subject = "${buildStatus}: Job '${env.JOB_NAME} [${env.BUILD_NUMBER}]'"
  def summary = "${subject} (${env.BUILD_URL})"

  // Override default values based on build status
  if (buildStatus == 'STARTED') {
    color = 'YELLOW'
    colorCode = '#FFFF00'  
  } else if (buildStatus == 'SUCCESSFUL') {
    color = 'GREEN'
    colorCode = '#00FF00'
  } else {
    color = 'RED'
    colorCode = '#FF0000'
	summary = "${subject} (${env.BUILD_URL})${details}"
  }
  // Get token
  def slackToken = getSlackToken('sysnet')
  // Send notifications
  slackSend (color: colorCode, message: summary, teamDomain: 'optdyn', token: "${slackToken}")
}

// get slack token from global jenkins credentials store
@NonCPS
def getSlackToken(String slackCredentialsId){
	// id is ID of creadentials
	def jenkins_creds = Jenkins.instance.getExtensionList('com.cloudbees.plugins.credentials.SystemCredentialsProvider')[0]

	String found_slack_token = jenkins_creds.getStore().getDomains().findResult { domain ->
	  jenkins_creds.getCredentials(domain).findResult { credential ->
	    if(slackCredentialsId.equals(credential.id)) {
	      credential.getSecret()
	    }
	  }
	}
	return found_slack_token
}
