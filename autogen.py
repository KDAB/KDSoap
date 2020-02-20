#!/usr/bin/env python
from autogen.autogen import autogen

project = "KDSoap"
version = "1.9.50"
subprojects = ["KDSoapClient", "KDSoapServer"]
prefixed = False

autogen(project, version, subprojects, prefixed, policyVersion = 2)
