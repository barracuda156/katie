# - Try to find MySQL
# Once done this will define
#
#  MYSQL_FOUND - system has MySQL
#  MYSQL_INCLUDES - the MySQL include directory
#  MYSQL_LIBRARIES - The libraries needed to use MySQL
#
# Copyright (c) 2015, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.

if(MYSQL_INCLUDES AND MYSQL_LIBRARIES)
    set(MYSQL_FIND_QUIETLY TRUE)
endif()

# Neither MySQL nor MariaDB provide pkg-config files
# However, they provide mysql_config
find_program(MYSQL_CONFIG
    NAMES
    mysql_config
    HINTS
    $ENV{MYSQLDIR}/bin
)

if(MYSQL_CONFIG)
    message(STATUS "Using ${MYSQL_CONFIG} to get package variables")
    execute_process(
        COMMAND ${MYSQL_CONFIG} --variable=pkgincludedir
        RESULT_VARIABLE procerror1
        OUTPUT_VARIABLE MYSQL_INCLUDES
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND ${MYSQL_CONFIG} --libmysqld-libs
        RESULT_VARIABLE proceerror2
        OUTPUT_VARIABLE MYSQL_LIBRARIES
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    # just in case
    if(NOT procerror1 STREQUAL "0" OR NOT proceerror2 STREQUAL "0")
        set(MYSQL_INCLUDES)
        set(MYSQL_LIBRARIES)
    endif()
else()
    find_path(MYSQL_INCLUDES
        NAMES
        mysql.h
        PATH_SUFFIXES mysql
        HINTS
        $ENV{MYSQLDIR}/include
        ${INCLUDE_INSTALL_DIR}
    )

    find_library(MYSQL_LIBRARIES
        mysqld
        HINTS
        $ENV{MYSQLDIR}/lib
        ${LIB_INSTALL_DIR}
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MySQL DEFAULT_MSG MYSQL_INCLUDES MYSQL_LIBRARIES)

mark_as_advanced(MYSQL_INCLUDES MYSQL_LIBRARIES)
