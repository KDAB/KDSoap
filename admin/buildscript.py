#!/usr/bin/python
import copy, platform, os, re
from AutobuildCore.Configuration import Configuration
from AutobuildCore.Project import Project
from AutobuildCore.helpers.build_script_helpers import DebugN
from AutobuildCore.helpers.exceptdefs import AutobuildException
from AutobuildCore import autobuildRoot

# some variables:
scmPath = 'svn+ssh://svn.kdab.net/home/SVN-klaralv/projects/Siemens/Soap/kdsoap'
buildSequenceSwitches = 'enable-conf-create-installer'
noDebug = ''
noRelease = ''
hiddenFileIdent = '.'

if 'Windows' in platform.platform():
	noDebug = ' CONFIG-=debug CONFIG-=debug_and_release'
	noRelease = ' CONFIG-=release CONFIG-=debug_and_release'
	buildSequenceSwitches += ',disable-conf-bin-package'
	hiddenFileIdent = '_'

product = Project( 'KD Soap' )
product.setVersionName( '1.1.0' )
product.setScmUrl(scmPath + '/trunk')
product.setPackageLocation( 'svn.kdab.net:/home/build/autobuild/packages/kdsoap' )
product.setBuildSequenceSwitches('s', buildSequenceSwitches)
product.setBuildSequenceSwitches('f', buildSequenceSwitches)

# pre-build hook functions that copy the documentation and the license into the
# target folder, if they are checked into internal/extra/<configname>:
def preBuildJobHook( buildJob ):
        DebugN( 1, 'kdSoap build script: pre-build hook called for build job ' + buildJob.fileSystemName() )
	step = buildJob.executomat().step( 'conf-make-install' )
	targetDir = os.path.normpath( buildJob.targetDir() )
	exportTool = autobuildRoot() + os.sep + '..' + os.sep + 'Tools' + os.sep + 'SvnExport.py'
	if not os.path.isfile( exportTool ):
		raise AutoBuildError( 'cannot find SvnExport.py in Tools/' )
	# we only copy things in for packaging
	if buildJob.configuration().project().getBuildType() in 'sSfF':
		extrasFolder = buildJob.configuration().getConfigName().lower()
		extrasFolder = re.sub( '\s', '_', extrasFolder )
		extrasPath = os.path.normpath( buildJob.configuration().project().checkoutDir() + os.sep \
			+ 'internal' + os.sep + 'extra' + os.sep + extrasFolder )
		cmd = 'python ' + exportTool + ' -f -s ' + extrasPath + ' -t ' + targetDir 
		step.addPostCommand( cmd, targetDir )
		# prepare the install folder for the different configurations:
		if 'Commercial' in buildJob.configuration().getConfigName():
			# copy source code into install folder for commercial builds:
                        excludedFiles = ' -d lib,manual,Licenses,config -x g++.pri,kdsoap.dox,.qmake-cache'
			cmd = 'python ' + exportTool + ' -f -s ' + buildJob.configuration().project().srcDir() + excludedFiles
			cmd += ' -t ' + targetDir
			step.addPostCommand( cmd, buildJob.configuration().project().srcDir() )
			# clean up the source
                        # (not there in KD Soap, yet)
			# cmd = 'python ' + buildJob.configuration().project().checkoutDir() \
			#		+ os.sep + 'admin' + os.sep + 'prepare-source-export.py'
			#		
			# step.addPostCommand( cmd, targetDir )
			# rm the lib directory, the user will compile the source by himself:
			cmd = 'rm -Rf lib'
			step.addPostCommand( cmd, targetDir )
                DebugN( 2, 'kdsoap build script: added command to copy docs and licenses into target folder' )

SharedDebug = Configuration( product, 'Debug' )
SharedDebug.setPackageDependencies( ['Qt-4.[3-9].?-Shared-Debug'])
SharedDebug.setBuildMode( 'inSource' )
SharedDebug.setPlatFormWhiteList( ['win32-msvc2005', 'macx-g++','linux-g++','linux-g++-64'] )
SharedDebug.setOptions( 'CONFIG+=debug' + noRelease )
SharedDebug.setPreBuildHook( preBuildJobHook )
SharedDebug.setBuildTypes( 'MCDF' )# debug is not built for snapshots

comSharedRelease = copy.copy( SharedDebug ) # use comDebug as the base configuration
comSharedRelease.setConfigName( 'Commercial Release' )
comSharedRelease.setPackageDependencies( ['Qt-4.[3-9].?-Shared-Release'])
comSharedRelease.setOptions( 'CONFIG+=release' + noDebug )
comSharedRelease.setBuildTypes( 'MCDSF' )# snapshots are release builds

SharedDebugKDChartSupport = copy.copy( SharedDebug)
SharedDebugKDChartSupport.setConfigName('Debug KDchart Support')
SharedDebugKDChartSupport.setPackageDependencies( ['Qt-4.[3-9].?-Shared-Debug','KDChart-2.*'])
SharedDebugKDChartSupport.setBuildTypes( 'MCDF' )# debug is not built for snapshots

SharedReleaseKDChartSupport = copy.copy( comSharedRelease ) # use comDebug as the base configuration
SharedReleaseKDChartSupport.setConfigName( 'Release KDChart Support' )
SharedReleaseKDChartSupport.setPackageDependencies( ['Qt-4.[3-9].?-Shared-Release','KDChart-2.*'] )
SharedReleaseKDChartSupport.setOptions( 'CONFIG+=release' + noDebug )

comSharedReleaseUS = copy.copy( comSharedRelease ) # use comRelease as the base configuration
comSharedReleaseUS.setConfigName( 'Commercial US Release' )
comSharedReleaseUS.setBuildTypes( 'MSF' )# snapshots are release builds

gplRelease = copy.copy( comSharedRelease )
gplRelease.setConfigName( 'GPL' )
gplRelease.setBuildTypes( 'MSF' )# snapshots are release builds

jobs = [ SharedDebug, comSharedRelease, SharedDebugKDChartSupport, SharedReleaseKDChartSupport, comSharedReleaseUS, gplRelease ]

def preSetupCallback(project):
    if len(scmPath) < len(project.getScmUrl()):
        project.setPackageSubDir(project.getScmUrl()[len(scmPath):])
product.registerPresetupCallback(preSetupCallback)

# set up a callback to inject extra command line commands into the build process:
# in this case, we add a command to be executed in the folder where the source code is executed 
# before packaging, that deletes a number of files that are unwanted in the distribution. the command 
# is a script that is located in the admin/ folder in the svn checkout: 
#
#def prebuildCallback( project ):
#	exportFolder = project.baseDir() + os.sep + project.scm().packageBaseName( product ) # ok, this you just have to know :-)
#	cmd = 'python ' +project.checkoutDir() + os.sep + 'admin' + os.sep + 'prepare-source-export.py'
#	step = project.postExecutomat().step( 'project-src-package' )
#	step.addMainCommand( cmd, exportFolder )
#product.registerPrebuildCallback( prebuildCallback )

product.build( jobs )
