#!/usr/bin/env python
#
# KDAB confidential
#
# This file is part of KDAB's internal build system setup.
# Author: Mirko Boehm, mirko.boehm@kdab.com
#
# This script sets up Make-O-Matic and Autobuild in the current folder.
# It generates a shell script that can be sourced to use the created environment.

from core.plugins.packagers.CPack import CPack
from core.helpers.BoilerPlate import getBuildProject
from products.ProductBoilerPlate import getSharedConfigurations

build, project = getBuildProject( minimumMomVersion = "0.5.0",
	projectName = "KD SOAP", projectVersionNumber = '1.0.0',
	scmUrl = 'svn+ssh://svn.kdab.com/home/SVN-klaralv/products/kdsoap/branches/kdsoap-1.0.0-release' )

sharedDebug, sharedRelease = getSharedConfigurations( project )

sharedRelease.addPlugin( CPack( sourcePackage = True ) )

build.build()
