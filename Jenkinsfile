#!/usr/bin/env groovy
def info = Artifactory.newBuildInfo()

pipeline {

  agent none

  parameters {

    booleanParam(name: 'DEBUG_BUILD', defaultValue: false, description: 'Сборка с отладочной информацией Maven')
    booleanParam(name: 'SIGN', defaultValue: true, description: 'Подпись')
    booleanParam(name: 'RELEASE', defaultValue: false, description: 'Релиз версия')
  }

  tools {
    maven 'Apache Maven 3'
    jdk 'jdk11'
  }

  options {
    buildDiscarder(logRotator(numToKeepStr:'10'))
  }

  stages {
    stage("Initialize vars") {
      agent { label 'linux && java17' }
      steps {
        echo "Initializing..."
        echo sh(returnStdout: true, script: 'env')
        echo "Initializing build number..."
        script {
          env.TARGET_REPO = "edt-equinox-launcher"
          env.DOCKER_REGISTRY = "launcher-docker-images.artifactory-docker-registry.boreas.dept07"
          env.DOCKER_IMAGE = "gcc-java-compile"
          env.DOCKER_IMAGE_VER = "1.0.4"
        }
      }
    }

    stage ("build natives"){
      parallel {
        stage("Build win32.win32.x86_64"){
          agent { label 'windows && java17' }
          steps{
            mvn(maven: "Maven 3.9.4",
                    jdk: "jdk17",
                    command: "clean verify",
                    profiles: "inventory-windows.x86_64",
                    useVirtualDisplay: false,
                    sign: params.SIGN,
                    // TODO: убрать maven.local.repo, когда решатся проблемы с артефактами
                    additionalParams: "-Dmaven.repo.local=./local-repo -Dnative=win32.win32.x86_64 -Drt.equinox.binaries.loc=${env.WORKSPACE}/rt.equinox.binaries")

            dir("${env.WORKSPACE}/rt.equinox.binaries") {
              stash(name: 'win-natives')
            }
          }
          post {
            always {
              cleanWs()
            }
          }
        }

        stage("Build cocoa.macosx.x86_64"){
          agent { label 'mac' }
          steps{
            mvn(maven: "Maven 3.9.4",
                    jdk: "jdk17",
                    command: "clean verify",
                    profiles: "inventory-macos.x86_64",
                    useVirtualDisplay: false,
                    sign: params.SIGN,
                    // TODO: убрать maven.local.repo, когда решатся проблемы с артефактами
                    additionalParams: "-Dmaven.repo.local=./local-repo -Dnative=cocoa.macosx.x86_64 -Drt.equinox.binaries.loc=${env.WORKSPACE}/rt.equinox.binaries")
            dir("${env.WORKSPACE}/rt.equinox.binaries") {
              stash(name: 'mac-natives')
            }
          }
          post {
            always {
              cleanWs()
            }
          }
        }
        stage("Build gtk.linux.x86_64"){
          agent { label 'docker && java17' }
          steps {
            echo "${env.DOCKER_VER}"
            dockerPullImage("${env.DOCKER_REGISTRY}", "${env.DOCKER_IMAGE}", "${env.DOCKER_IMAGE_VER}")
            mvn(maven: "Maven 3.9.4",
                    jdk: "jdk17",
                    command: "clean verify",
                    profiles: "inventory-linux.x86_64",
                    useVirtualDisplay: false,
                    sign: params.SIGN,
                    // TODO: убрать maven.local.repo, когда решатся проблемы с артефактами
                    additionalParams: "-Dmaven.repo.local=./local-repo -Dnative=gtk.linux.x86_64 -Ddocker -Ddocker-image=${env.DOCKER_REGISTRY}/${env.DOCKER_IMAGE}:${env.DOCKER_IMAGE_VER} -Drt.equinox.binaries.loc=${env.WORKSPACE}/rt.equinox.binaries")
            dir("${env.WORKSPACE}/rt.equinox.binaries") {
              stash(name: 'linux-natives')
            }
          }
          post {
            always {
              cleanWs()
            }
          }
        }
      }
    }
    stage ("Build p2-repository"){
      agent {label 'linux && java17'}
      steps{
        echo "${env.DOCKER_VER}"
        dir("${env.WORKSPACE}/rt.equinox.binaries") {
          unstash 'win-natives'
          unstash 'linux-natives'
          unstash 'mac-natives'
        }
        mvn(maven: "Maven 3.9.4",
                jdk: "jdk17",
                command: "clean verify",
                profiles: "launcher.gtk.linux.x86_64",
                useVirtualDisplay: false,
                sign: params.SIGN,
                // TODO: убрать maven.local.repo, когда решатся проблемы с артефактами
                additionalParams: "-Dmaven.repo.local=./local-repo -Drt.equinox.binaries.loc=${env.WORKSPACE}/rt.equinox.binaries -Plauncher.win32.win32.x86_64 -Plauncher.cocoa.macosx.x86_64")
        script {
          env.BUILD_VERSION = mvnReadProperty('revision')
          if(params.RELEASE) {
            // delete snapshot from build version
            env.BUILD_VERSION = env.BUILD_VERSION.substring(0, env.BUILD_VERSION.lastIndexOf('-'))
            echo "BUILD VERSION: ${env.BUILD_VERSION}"
          }
          env.P2_URL = "${env.BUILD_VERSION}/p2/repo.zip"
          zip archive: true, dir: './p2repository/target/repository', zipFile: './p2repository/target/p2-repo.zip'
        }
        artifactoryUploadArtifacts(
                files: ["${env.WORKSPACE}/p2repository/target/p2-repo.zip"],
                repository: "${env.TARGET_REPO}",
                props: "version=${env.BUILD_VERSION}",
                target: "${P2_URL}",
                buildInfo: info
        )
      }
      post {
        always {
          cleanWs()
        }
      }
    }

  }
}

def dockerPullImage(registry, img, imgVersion) {
  image = docker.image("${registry}/${img}:${imgVersion}")
  docker.withRegistry("https://${registry}", "artifactory-launcher-docker-registry") {
    image.pull()
  }
}
