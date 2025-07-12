# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2025 Jonathan Brady <jtjbrady@users.noreply.github.com>
#
# SPDX-License-Identifier: MIT
#

project(ranges)

set(ranges_SRCS test_ranges.cpp)
set(EXTRA_LIBS kdsoap-server)
add_unittest(${ranges_SRCS})
