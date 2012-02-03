import os, sys, subprocess

from cpack import CPackGenerateConfiguration 
from configure import ConfigureScriptGenerator
from header import ForwardHeaderGenerator

def autogen(project, version, subprojects, prefixed, policyVersion = 1):
	global __policyVersion
	__policyVersion = policyVersion
	sourceDirectory = os.path.abspath( os.path.dirname( os.path.dirname( __file__ ) ) )
	buildDirectory = os.getcwd()

	print( "-- Using source directory: {0}".format( sourceDirectory ) )

	# check repository URL
	p = subprocess.Popen( ["svn", "info"], cwd = sourceDirectory, stdout = subprocess.PIPE, stderr = subprocess.PIPE )
	( stdout, stderr ) = p.communicate()
	if p.returncode != 0:
		p = subprocess.Popen( ["git", "svn", "info"], cwd = sourceDirectory, stdout = subprocess.PIPE, stderr = subprocess.PIPE )
		( stdout, stderr ) = p.communicate()
	if p.returncode != 0:
		print_stderr( "Error: Not an SVN repository: {0}".format( sourceDirectory ) )
		sys.exit( 1 )

	repositoryUrl = stdout.splitlines()[1].split( ':', 1 )[1]
	repositoryRevision = stdout.splitlines()[4].split( ':', 1 )[1].strip()
	isTagged = repositoryUrl.find('/tags/') != -1

	cpackConfigurationGenerator = CPackGenerateConfiguration( project, version, buildDirectory, repositoryRevision,
	                                                          isTaggedRevision = isTagged )
	cpackConfigurationGenerator.run()

	configureScriptGenerator = ConfigureScriptGenerator( project, buildDirectory, version )
	configureScriptGenerator.run()

	includePath = os.path.join( sourceDirectory, "include" )
	srcPath = os.path.join( sourceDirectory, "src" )

	if subprojects:
		forwardHeaderGenerator = ForwardHeaderGenerator( 
			copy = True, path = sourceDirectory, includepath = includePath, srcpath = srcPath,
			project = project, subprojects = subprojects, prefix = "$$INSTALL_PREFIX", prefixed = prefixed 
		)
		forwardHeaderGenerator.run()

	print( "-- Auto-generation done." )

	with file( ".license.accepted", 'a' ):
		os.utime( ".license.accepted", None )
	print( "-- License marked as accepted." )

	print( "-- Wrote build files to: {0}".format( buildDirectory ) )
	print( "-- Now running configure script." )
	print( "" )
	sys.stdout.flush()

	if sys.platform == 'win32':
		os.execvp( './configure.bat', ['configure.bat'] + sys.argv[1:] )
	else:
		os.execvp( './configure.sh', ['configure.sh'] + sys.argv[1:] )

def policyVersion():
	global __policyVersion
	return __policyVersion
