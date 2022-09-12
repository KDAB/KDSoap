#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

from conans import ConanFile, CMake, tools

class KdsoapConan(ConanFile):
    name = "KDSoap"
    version = "2.2.0"
    license = ("https://raw.githubusercontent.com/KDAB/KDSoap/master/LICENSES/MIT.txt")
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
