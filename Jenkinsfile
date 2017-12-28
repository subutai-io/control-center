#!groovy

notifyBuildDetails = ""

try {
	notifyBuild('STARTED')

	/* Building agent binary.
	Node block used to separate agent and subos code.
	*/

    node("windows") {

		stage("Start build Windows")
		
		notifyBuildDetails = "\nFailed on Stage - Start build"

		bat 'start cmd.exe /c C:\\Jenkins\\build\\Build_dev.lnk'

		stage("Upload")

		notifyBuildDetails = "\nFailed on Stage - Upload"

		bat 'start cmd.exe /c C:\\Jenkins\\upload\\dev\\upload_dev.do C:\\tray_builds\\dev\\SubutaiTray.exe'
        bat 'start cmd.exe /c C:\\Jenkins\\upload\\dev\\upload_dev.do C:\\tray_builds\\dev\\subutai-tray-dev.msi'
        
	}

	node("debian") {

		stage("Start build Debian")
		
		notifyBuildDetails = "\nFailed on Stage - Start build"

		sh """
			/home/builder/./build_dev.sh
		"""

		stage("Upload")

		notifyBuildDetails = "\nFailed on Stage - Upload"

		sh """
            /home/builder/upload_script/./upload_dev.sh /home/builder/build_dev/subutai-tray-dev.deb
            /home/builder/upload_script/./upload_dev.sh /home/builder/build_dev/SubutaiTray
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