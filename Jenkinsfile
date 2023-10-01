pipeline {
  agent {
    node {
      label 'DevEnv'
      customWorkspace '/var/jenkins/workspace/PanLPower'
    }
  }

  options {
    buildDiscarder(logRotator(numToKeepStr:'1', artifactNumToKeepStr: '1'))
    disableConcurrentBuilds()
  }

  parameters {
    booleanParam(defaultValue: true, description: 'To enable source code analysis', name: "Enable SonarQube scan" )
    string defaultValue: 'buong.vo@brtchip.com', description: 'Comma seperated emails', name: 'Notified Emails', trim: true
  }

  stages {
    stage ('Validating tag') {
      steps {
        echo "GIT_COMMIT = ${GIT_COMMIT}"
        echo "GIT_BRANCH = ${GIT_BRANCH}"
        }
      }

    stage('Prepare build enviroment') {
      steps {
        sh '''
          docker pull sdthirlwall/raspberry-pi-cross-compiler
          docker build -t panlpowerbuild .
          docker run -d -v /var/jenkins/workspace/PanLPower:/build/panlpowerbuild --name panlpowerbuild panlpowerbuild
          docker exec -it panlpowerbuild /bin/bash
          cd panlpowerbuild/Debug && make clean && make
          exit
          docker stop panlpowerbuild
          docker rm panlpowerbuild
          docker rmi panlpowerbuild
          docker images
        '''
      }
    }
    stage ('Generate Package') {
      steps {
        dir ("Debug") {
          sh "tar.exe -a -c -f ../../PanLPower.zip PanLPower/Debug/main"
        }
      }

      post {
        success {
          archiveArtifacts artifacts: "PanLPower.zip", fingerprint: true
          echo "Success: - archiveArtifacts PanLPower.zip"
        }
        failure {
          echo "Error archiveArtifacts"
        }
      }
    }
  }

  post {
    always {
      notifyViaEmail(params['Notified Emails'])
      cleanWs()
    }
  }
}

def notifyViaEmail(emails) {
  if (!emails) {
    return
  }

  def buildStatus = currentBuild.result.toString()
  def jobName = currentBuild.fullDisplayName
  emailext attachLog: true,
    body: '''${SCRIPT, template="groovy-html.template"}''',
    mimeType: 'text/html',
    subject: "[${buildStatus}] Jenkins ${jobName}",
    to: "${emails}",
    replyTo: "${emails}",
    recipientProviders: [[$class: 'CulpritsRecipientProvider']]
}
