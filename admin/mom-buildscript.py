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

build, project = BuildProject( 'KD SOAP', version = '1.0.0', url = 'svn+ssh://svn.kdab.com/home/SVN-klaralv/products/kdsoap',
                               branch = "kdsoap-1.0", minimumMomVersion = "0.5.0" )
configs = ProductConfigurations( project )
build.build()
