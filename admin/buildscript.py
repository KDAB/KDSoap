#!/usr/bin/python
import copy, platform, os, re
from AutobuildCore.Configuration import Configuration
from AutobuildCore.Project import Project
from AutobuildCore.helpers.build_script_helpers import DebugN
from AutobuildCore.helpers.exceptdefs import AutobuildException
from AutobuildCore import autobuildRoot

buildSequenceSwitches = 'enable-conf-create-installer'
hiddenFileIdent = '.'

if 'Windows' in platform.platform():
	buildSequenceSwitches += ',disable-conf-bin-package'
	hiddenFileIdent = '_'

scmPath = 'svn+ssh://svn.kdab.net/home/SVN-klaralv/products/kdsoap'
product = Project( 'KD Soap' )

product.setScmUrl( scmPath + '/trunk' )
product.setPackageLocation( 'svn.kdab.com:/home/build/autobuild/packages/kdsoap' )
product.setBuildSequenceSwitches( 's', buildSequenceSwitches )
product.setBuildSequenceSwitches( 'f', buildSequenceSwitches )

# pre-build hook functions that copy the documentation and the license into the
# target folder, if they are checked into internal/extra/<configname>:
def preBuildJobHook( buildJob ):
        DebugN( 1, 'kdsoap build script: pre-build hook called for build job ' + buildJob.fileSystemName() )
	step = buildJob.executomat().step( 'conf-make-install' )
	targetDir = os.path.normpath( buildJob.targetDir() )
	exportTool = autobuildRoot() + os.sep + '..' + os.sep + 'Tools' + os.sep + 'SvnExport.py'
	buildDir = os.path.normpath(buildJob.buildDir())
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
		# copy source code into install folder for commercial builds:
		excludedFiles = ' -d lib,manual,Licenses,config -x g++.pri,kdsoap.dox,.qmake-cache'
		cmd = 'python ' + exportTool + ' -f -s ' + buildJob.configuration().project().srcDir() + excludedFiles
		cmd += ' -t ' + targetDir
		step.addPostCommand( cmd, buildJob.configuration().project().srcDir() )
		# clean up the source 
		# (not there in KD Soap, yet)
		# cmd = 'python ' + buildJob.configuration().project().checkoutDir() \
		#		+ os.sep + 'admin' + os.sep + 'prepare-source-export.py'
		# step.addPostCommand( cmd, targetDir )
		# rm the lib directory, the user will compile the source by himself:
		cmd = 'rm -Rf lib'
		step.addPostCommand( cmd, targetDir )
		DebugN( 2, 'kdsoap build script: added command to copy docs and licenses into target folder' )
	stepExportSrc = buildJob.executomat().step('conf-export-src')
	cmd3 = 'touch .license.accepted'
	stepExportSrc.addPostCommand(cmd3, buildDir)
	DebugN(2, 'kdsoap build script: added command to copy license into build folder')

product = Project( 'KD Soap' )
product.setScmUrl( scmPath + '/trunk' )
product.setPackageLocation( 'svn.kdab.com:/home/build/autobuild/packages/kdsoap' )
product.setBuildSequenceSwitches( 's', buildSequenceSwitches )
product.setBuildSequenceSwitches( 'f', buildSequenceSwitches )

SharedDebug = Configuration( product, 'Shared-Debug' )
SharedDebug.setPreBuildHook( preBuildJobHook )
SharedDebug.setPackageDependencies( [ 'Qt-4.[4-9].?-Shared-Debug' ] )
SharedDebug.setBuildMode( 'inSource' )
#SharedDebug.setPlatFormWhiteList( ['win32-msvc2005', 'macx-g++','linux-g++','linux-g++-64'] )
SharedDebug.setOptions( '-unittests -debug' )
SharedDebug.setBuildTypes( 'MCDF' ) # debug is not built for snapshots

commercial = Configuration( product , 'commercial')
commercial.setPreBuildHook( preBuildJobHook )
commercial.setBuilder('autotools')
commercial.setPackageDependencies( [ 'Qt-4.[4-9].?-Shared-Release' ] )
commercial.setBuildMode( 'inSource' )
commercial.setOptions( '-unittests -release' )
commercial.setBuildTypes( 'MCDSF' ) # snapshots are release builds

StaticDebug = Configuration( product, 'Static-Debug')
StaticDebug.setPreBuildHook( preBuildJobHook )
StaticDebug.setBuilder('autotools')
StaticDebug.setPackageDependencies( [ 'Qt-4.[4-9].?-Static-Debug' ] )
StaticDebug.setBuildMode( 'inSource' )
StaticDebug.setOptions( '-unittests -static -debug' )
StaticDebug.setBuildTypes('MCDF')

StaticRelease = Configuration( product, 'Static-Release')
StaticRelease.setPreBuildHook( preBuildJobHook )
StaticRelease.setBuilder('autotools')
StaticRelease.setPackageDependencies( [ 'Qt-4.[4-9].?-Static-Release' ] )
StaticRelease.setBuildMode( 'inSource' )
StaticRelease.setOptions( '-unittests -static -release' )
StaticRelease.setBuildTypes('MCDF')

commercial_us = Configuration( product, 'commercial_us')
commercial_us.setPreBuildHook( preBuildJobHook )
commercial_us.setBuilder('autotools')
commercial_us.setPackageDependencies( [ 'Qt-4.[4-9].?-Shared-Release' ] )
commercial_us.setBuildMode( 'inSource' )
commercial_us.setOptions( '-unittests -release' )
commercial_us.setBuildTypes( 'MSF' ) # snapshots are release builds

gpl = Configuration( product, 'gpl')
gpl.setPreBuildHook( preBuildJobHook )
gpl.setBuilder('autotools')
gpl.setPackageDependencies( [ 'Qt-4.[4-9].?-Shared-Release' ] )
gpl.setBuildMode( 'inSource' )
gpl.setOptions( '-unittests -release' )
gpl.setBuildTypes( 'MSF' ) # snapshots are release builds

if 'Darwin' in platform.platform():
	jobs = [ SharedDebug, commercial,commercial_us, gpl ]
else:
	jobs = [ SharedDebug, commercial, StaticDebug, StaticRelease, commercial_us, gpl ]

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

def preSetupCallback( project ):
	if len( scmPath ) < len( project.getScmUrl() ):
		project.setPackageSubDir(project.getScmUrl()[len( scmPath ):])
product.registerPresetupCallback(preSetupCallback)

product.build( jobs )
