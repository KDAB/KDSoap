#############################################################################
## Copyright (C) 2010-2020 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
## All rights reserved.
##
## This file is part of the KD Soap library.
##
## Licensees holding valid commercial KD Soap licenses may use this file in
## accordance with the KD Soap Commercial License Agreement provided with
## the Software.
##
## This file may be distributed and/or modified under the terms of the
## GNU Lesser General Public License version 2.1 and version 3 as published by the
## Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
##
## This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
## WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
##
## Contact info@kdab.com if any conditions of this licensing are not
## clear to you.
##
######################################################################/

from conans import ConanFile, CMake, tools

class KdsoapConan(ConanFile):
    name = "KDSoap"
    version = "1.9.0"
    license = ("https://raw.githubusercontent.com/KDAB/KDSoap/kdsoap-{0}-release/LICENSE.txt,"
               "https://raw.githubusercontent.com/KDAB/KDSoap/kdsoap-{0}-release/LICENSE.AGPL3-modified.txt,"
               "https://raw.githubusercontent.com/KDAB/KDSoap/kdsoap-{0}-release/LICENSE.GPL.txt,"
               "https://raw.githubusercontent.com/KDAB/KDSoap/kdsoap-{0}-release/LICENSE.LGPL.txt").format(version)
    author = "Klaralvdalens Datakonsult AB (KDAB) info@kdab.com"
    url = "https://github.com/KDAB/KDSoap.git"
    description = "KD Soap is a Qt-based client-side and server-side SOAP component."
    generators = "cmake"
    options = dict({
        "build_static": [True, False],
        "build_examples": [True, False],
        "build_tests": [True, False],
    })
    default_options = dict({
        "build_static": False,
        "build_examples": True,
        "build_tests": False,
    })
    settings = "build_type"

    def requirements(self):
        self.requires("qt/5.13.2@kdab/stable")

    def source(self):
        git = tools.Git(folder="")
        git.clone(self.url)
        git.checkout("kdsoap-%s-release"%self.version)

    def configure(self):
        # Use same qt configuration for all kdab packages
        # ~$ conan create -ks -o qt:qttools=True -o qt:qtsvg=True -o qt:qtdeclarative=True -o qt:qtremoteobjects=True -o qt:qtscxml=True . 5.13.2@kdab/stable
        self.options["qt"].qtsvg = True
        self.options["qt"].qtdeclarative = True
        self.options["qt"].qtremoteobjects = True
        self.options["qt"].qtscxml = True
        self.options["qt"].qttools = True

    def build(self):
        self.cmake = CMake(self)
        self.cmake.definitions["KDSoap_STATIC"] = self.options.build_static
        self.cmake.definitions["KDSoap_EXAMPLES"] = self.options.build_examples
        self.cmake.definitions["KDSoap_TESTS"] = self.options.build_tests
        self.cmake.configure()
        self.cmake.build()

    def package(self):
        self.cmake.install()

    def package_info(self):
        self.env_info.CMAKE_PREFIX_PATH.append(self.package_folder)
