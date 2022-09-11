#!/usr/bin/env python
import os
import stat

#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

# This script, when run in KD Soap's source and build directory containing the results of a full
# build, writes a list of files to be excluded from packaging to CPackIgnores.txt.
# autogen reads that list and passes it to CPack.

# You should re-generate CPackIgnores.txt before every release and also check that no new binary
# slips through the cracks using
# find . -type f -executable

sourceDirectory = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))


def ignoredFiles():
    ret = []

    def findExecutables(top):
        for f in os.listdir(top):
            pathname = os.path.join(top, f)
            if stat.S_ISDIR(os.stat(pathname).st_mode):
                findExecutables(pathname)
            elif os.access(pathname, os.X_OK):
                # The file is executable for us
                ret.append(pathname + '$')
                # for OS X
                ret.append(pathname + '.app/')

    # With one exception, the executables in those paths are binaries and we're making a SOURCE package.
    for path in ['unittests']:
        findExecutables(os.path.join(sourceDirectory, path))
    # The exception!
    def isGoodExclude(s): return not s.startswith(os.path.join(sourceDirectory, 'unittests/runTest.bat'))
    return sorted(filter(isGoodExclude, ret))


f = open('CPackIgnores.txt', 'w')
for ign in ignoredFiles():
    # write paths relative to the source dir, one per line
    f.write(ign[len(sourceDirectory):] + '\n')
