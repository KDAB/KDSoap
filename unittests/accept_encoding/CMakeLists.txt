# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2025 Jonathan Brady <jtjbrady@users.noreply.github.com>
#
# SPDX-License-Identifier: MIT
#

project(accept_encoding)

set(accept_encoding_SRCS test_accept_encoding.cpp resources.qrc)
set(EXTRA_LIBS kdsoap-server)
#add_unittest(${accept_encoding_SRCS})

# Resources don't seem to work with add_unittest so set up target manually.
set(_name test_accept_encoding)
# What I'd like to do is:
#qt_add_resources(test_accept_encoding resources.qrc)
#if(QT_FEATURE_zstd)
#    qt_add_resources(test_accept_encoding zstd_resources.qrc)
#endif()
# But I can't get that to work.
add_executable(
    ${_name}
    ${accept_encoding_SRCS}
)
add_test(NAME kdsoap-${_name} COMMAND ${_name})
target_link_libraries(
    ${_name}
    ${QT_LIBRARIES}
    kdsoap
    testtools
    ${EXTRA_LIBS}
)
