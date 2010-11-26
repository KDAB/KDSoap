#!/usr/bin/env python
#
# KDAB confidential
#
# This file is part of KDAB's internal build system setup. It is the
# build script for KD SOAP, which integrates it into the continuous
# integration system.
#
# Author: Mirko Boehm, mirko.boehm@kdab.com


from core.helpers.BoilerPlate import BuildProject
from products.ProductBoilerPlate import ProductConfigurations
from core.plugins.packagers.CPack import CPack

build, project = BuildProject( 'KD SOAP', 'svn+ssh://svn.kdab.com/home/SVN-klaralv/products/kdsoap', '1.0.0',
								branch = "kdsoap-1.0", minimumMomVersion = "0.5.0" )

staticDebug, staticRelease, sharedDebug, sharedRelease = ProductConfigurations( project )

sharedRelease.addPlugin( CPack( sourcePackage = True ) )

build.build()
