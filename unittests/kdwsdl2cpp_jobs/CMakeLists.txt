# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2014 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(kdwsdl2cpp_jobs_SRCS test_jobs.cpp)
set(WSDL_FILES jobs.wsdl)
add_unittest(${kdwsdl2cpp_jobs_SRCS})
