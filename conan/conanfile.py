#############################################################################
## Copyright (C) 2010-2019 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
## All rights reserved.
##
## This file is part of the KD Soap library.
##
## Licensees holding valid commercial KD Soap licenses may use this file in
## accordance with the KD Soap Commercial License Agreement provided with
## the Software.
##
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
    version = "1.8.0"
    license = "https://raw.githubusercontent.com/KDAB/KDSoap/kdsoap-1.8.0-release/LICENSE.txt"
    author = "Renato Araujo Oliveira Filho renato.araujo@kdab.com"
    url = "https://github.com/KDAB/KDSoap.git"
    description = "KD Soap is a Qt-based client-side and server-side SOAP component."
    generators = "cmake"

    def requirements(self):
        self.requires("Qt/5.11.2@bincrafters/stable")

    def source(self):
        git = tools.Git(folder="")
        git.clone(self.url)
        git.checkout("kdsoap-%s-release"%self.version)

    def build(self):
        self.cmake = CMake(self)
        self.cmake.configure()
        self.cmake.build()

    def package(self):
        self.cmake.install()
