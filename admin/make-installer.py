# script to create installer
import os, sys, platform, re
from AutobuildCore.Installers.runner import makeInstaller
from AutobuildCore.Installers.config import Config
from AutobuildCore.helpers.Commands import platformRmDir

config = Config()
config.parseCommandLineOptions()

# product setup
config.setProduct( "KDSoap" )
config.setVendor( "KDAB" )
config.setRelease( "1.2" )
# the directory that is packed into the installer
config.setDirectory( config.targetDir() )
# FIXME (Thomas) map to QMAKESPEC

config.setDescription("KDAB's Reporting Tool for Qt applications")
config.setSummary("KD Soap")
config.setVersion("1")

# set application specific themes and/or scripts dir if none was specified via command line
if not config.hasThemesDir() or not config.hasScriptDir() :
    pathname, scriptname = os.path.split(sys.argv[0])
    # The directory structure that "admin" is parallel to "internal"
    # so adding '..' works fine since abspath() will clean the path nicely:
    if not pathname.endswith( os.sep ):
        pathname = pathname + os.sep
    pathname = os.path.abspath( pathname + '..' )
    if not pathname.endswith( os.sep ):
        pathname = pathname + os.sep
    if not config.hasThemesDir() :
        config.setThemesDir( pathname  + 'internal' + os.sep + 'themes' )
    #
    # comenting this out for now: KD Soap does not using an extra version of buildwindows.bat
    #
    #if not config.hasScriptDir() :
    #    config.setScriptDir( pathname  + 'internal' + os.sep + 'scripts' )

makeInstaller( config )

# now we have the installers in packages/. On Linux, the extensions
# are .bin. On Windows. they are .exes.
# On Mac, those are directories ending in .app. To make them easier to 
# download, we zip them: 
if 'Darwin-' in platform.platform():
    for app in os.listdir(config.packagesDir()):
        if app.endswith('.app'):
            oldPwd = os.getcwd()
            try:
                os.chdir(config.packagesDir())
                appFile = app
                zipFile = config.packagesDir() + os.sep + re.sub( '.app', '.zip', app)
                cmd = 'zip -r ' + zipFile + ' ' + appFile
                if os.system(cmd) != 0:
                    raise Exception('Cannot zip installer file')
                if os.system(platformRmDir() + appFile) != 0:
                    raise Exception('Cannot remove app bundle after zipping')
            finally:
                os.chdir(oldPwd)
