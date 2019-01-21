#!groovy

notifyBuildDetails = ""

try {
	notifyBuild('STARTED')
	switch (env.BRANCH_NAME) {
            case ~/master/: 
            branch = "master"
            break;
	    	default: 
            branch = "dev"
    }
	
	node("debian") {

		stage("Start build Debian")
		
		notifyBuildDetails = "\nFailed on Stage - Start build"
		checkout scm

		sh """
		./build_deb.sh ${branch}
		"""

		stage("Upload")

		notifyBuildDetails = "\nFailed on Stage - Upload"

		sh """
		cd /home/builder/deb_repo
		touch uploading_agent
		scp uploading_agent subutai*.deb dak@deb.subutai.io:incoming/${branch}/
		ssh dak@deb.subutai.io sh /var/reprepro/scripts/scan-incoming.sh ${branch} agent
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
	def mattermost_rest = "https://mm.subutai.io/hooks/k1ujhg8xwigupxjzqbwzpgdp3h"
  // Send notifications
	mattermostSend(color: colorCode, icon: "https://jenkins.io/images/logos/jenkins/jenkins.png", message: summary, channel: "#sysnet-bots", endpoint: "${mattermost_rest}" )
	//slack
	//def slackToken = getSlackToken('sysnet')
	//slackSend (color: colorCode, message: summary, teamDomain: 'optdyn', token: "${slackToken}")

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
