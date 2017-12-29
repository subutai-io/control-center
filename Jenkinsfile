#!groovy

notifyBuildDetails = ""

try {
	notifyBuild('STARTED')
    switch (env.BRANCH_NAME) {
            case ~/master/: 
            upload_path = "C:\\Jenkins\\upload\\master\\"
            build_path_linux = "home/builder/build_master"
            windows_tray_build = "build_master.lnk"
            linux_tray_build = "build_master.sh"
            upload_msi = "upload_master_msi.do"
            upload_exe = "upload_master_exe.do"
            upload_deb = "subutai-tray-master.deb"
            upload_sh = "SubutaiTray"
            upload_script = "upload_master.sh"

            break;
	        default: 
            upload_path = "C:\\Jenkins\\upload\\dev\\"
            build_path_linux = "home/builder/build_dev"
            windows_tray_build = "build_dev.lnk"
            linux_tray_build = "./build_dev.sh"
            upload_msi = "upload_dev_msi.do"
            upload_exe = "upload_dev_exe.do"
            upload_deb = "subutai-tray-dev.deb"
            upload_sh = "SubutaiTray"
            upload_script = "upload_dev.sh"
    }
	/* Building agent binary.
	Node block used to separate agent and subos code.
	*/
	node("windows") {

		stage("Start build Windows")
		
		notifyBuildDetails = "\nFailed on Stage - Start build"

		bat "C:\\Jenkins\\build\\${windows_tray_build}"
		
        stage("Upload")

		notifyBuildDetails = "\nFailed on Stage - Upload"

		bat "${upload_path}${upload_msi}"
		bat "${upload_path}${upload_exe}"
	}

	node("debian") {

		stage("Start build Debian")
		
		notifyBuildDetails = "\nFailed on Stage - Start build"

		sh """
			/home/builder/./${linux_tray_build}
		"""

		stage("Upload")

		notifyBuildDetails = "\nFailed on Stage - Upload"

		sh """
		/home/builder/upload_script/./${upload_script} /${build_path_linux}/${upload_deb}
		/home/builder/upload_script/./${upload_script} /${build_path_linux}/${upload_sh}
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
