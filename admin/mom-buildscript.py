#!/usr/bin/env python
#
# KDAB confidential
#
# This file is part of KDAB's internal build system setup. It is the
# build script for KD SOAP, which integrates it into the continuous
# integration system. 
#
# Author: Mirko Boehm, mirko.boehm@kdab.com


from core.plugins.packagers.CPack import CPack
from core.helpers.BoilerPlate import getBuildProject
from products.ProductBoilerPlate import getConfigurations

build, project = getBuildProject( minimumMomVersion = "0.5.0",
	projectName = "KD SOAP", projectVersionNumber = '1.0.0',
	scmUrl = 'svn+ssh://svn.kdab.com/home/SVN-klaralv/products/kdsoap' )

staticDebug, staticRelease, sharedDebug, sharedRelease = getConfigurations( project )

sharedRelease.addPlugin( CPack( sourcePackage = True ) )

build.build()
