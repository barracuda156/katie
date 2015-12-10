string(TIMESTAMP KATIE_DATE "%Y-%m-%d")
set(KATIE_MAJOR "4")
set(KATIE_MINOR "8")
set(KATIE_MICRO "7")
set(KATIE_NAME "Katie")
set(KATIE_VERSION "${KATIE_MAJOR}.${KATIE_MINOR}.${KATIE_MICRO}")
set(KATIE_STRING "qt4 ${KATIE_MAJOR}.${KATIE_MINOR}.${KATIE_MICRO}")
set(KATIE_BUGREPORT "xakepa10@gmail.com")
set(KATIE_URL "http://github.com/fluxer/katie")
set(KATIE_COMPONENTS "Core Gui DBus Declarative Designer Help Multimedia Network OpenGL Phonon Sql Svg Xml XmlPatterns Script ScriptTools Test UiTools")
# TODO: make dbus tools optional
set(KATIE_TOOLS "moc uic rcc qdbusxml2cpp qdbuscpp2xml qhelpgenerator qcollectiongenerator lupdate lrelease lconvert")
set(QT_LICENSE "Open Source")
set(QT_PRODUCT "OpenSauce") # it's not a bug, it's a feature!

# in case a it's being include by something other than Katie build system
if(NOT KATIE_PLATFORM)
    set(KATIE_PLATFORM "auto")
endif()
if(NOT KATIE_COMPILER)
    set(KATIE_COMPILER "auto")
endif()
if(NOT KATIE_ARCHITECTURE)
    set(KATIE_ARCHITECTURE "auto")
endif()
if(NOT KATIE_TYPE)
    set(KATIE_TYPE "auto")
endif()
if(NOT KATIE_KEY)
    set(KATIE_KEY "auto")
endif()
if(NOT KATIE_KEY_COMPAT)
    set(KATIE_KEY_COMPAT "auto")
endif()
if(NOT KATIE_MKSPECS_DIR)
    set(KATIE_MKSPECS_DIR ${CMAKE_SOURCE_DIR}/mkspecs)
endif()

# TODO: more platforms/architectures support
include_directories(${KATIE_MKSPECS_DIR})
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND KATIE_PLATFORM STREQUAL "auto")
    include_directories(${KATIE_MKSPECS_DIR}/linux)
    set(KATIE_PLATFORM "linux")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows" AND KATIE_PLATFORM STREQUAL "auto")
    include_directories(${KATIE_MKSPECS_DIR}/win32)
    set(KATIE_PLATFORM "win32")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND KATIE_PLATFORM STREQUAL "auto")
    include_directories(${KATIE_MKSPECS_DIR}/mac)
    set(KATIE_PLATFORM "mac")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD" AND KATIE_PLATFORM STREQUAL "auto")
    include_directories(${KATIE_MKSPECS_DIR}/freebsd)
    set(KATIE_PLATFORM "freebsd")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD" AND KATIE_PLATFORM STREQUAL "auto")
    include_directories(${KATIE_MKSPECS_DIR}/openbsd)
    set(KATIE_PLATFORM "openbsd")
elseif(KATIE_PLATFORM STREQUAL "auto")
    MESSAGE(FATAL_ERROR "Unknown platform '${CMAKE_SYSTEM_NAME}'")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND KATIE_COMPILER STREQUAL "auto")
    set(KATIE_COMPILER "clang")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND KATIE_COMPILER STREQUAL "auto")
    set(KATIE_COMPILER "gcc")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel" AND KATIE_COMPILER STREQUAL "auto")
    set(KATIE_COMPILER "icc")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" AND KATIE_COMPILER STREQUAL "auto")
    set(KATIE_COMPILER "msvc")
elseif(KATIE_COMPILER STREQUAL "auto")
    MESSAGE(FATAL_ERROR "Unknown compiler '${CMAKE_CXX_COMPILER_ID}'")
endif()

if(CMAKE_SYSTEM_PROCESSOR)
    string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} LOWERCASE_CMAKE_SYSTEM_PROCESSOR)
elseif(CMAKE_HOST_SYSTEM_PROCESSOR)
    string(TOLOWER ${CMAKE_HOST_SYSTEM_PROCESSOR} LOWERCASE_CMAKE_SYSTEM_PROCESSOR)
endif()
if(LOWERCASE_CMAKE_SYSTEM_PROCESSOR MATCHES "^arm" AND KATIE_ARCHITECTURE STREQUAL "auto")
    set(KATIE_ARCHITECTURE "arm")
elseif(LOWERCASE_CMAKE_SYSTEM_PROCESSOR MATCHES "^mips" AND KATIE_ARCHITECTURE STREQUAL "auto")
    set(KATIE_ARCHITECTURE "mips")
elseif(LOWERCASE_CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64|amd64)" AND KATIE_ARCHITECTURE STREQUAL "auto")
    set(KATIE_ARCHITECTURE "x86_64")
elseif(LOWERCASE_CMAKE_SYSTEM_PROCESSOR MATCHES "(i[3-6]86|x86)" AND KATIE_ARCHITECTURE STREQUAL "auto")
    set(KATIE_ARCHITECTURE "i386")
elseif(KATIE_ARCHITECTURE STREQUAL "auto")
    MESSAGE(FATAL_ERROR "Unknown CPU '${CMAKE_SYSTEM_PROCESSOR}'")
endif()

# XXX: Plan9 does not supporting dynamic libraries
if(${KATIE_TYPE} STREQUAL "auto")
    set(KATIE_TYPE SHARED)
    katie_definition(-DQT_SHARED)
endif()

if(${KATIE_PLATFORM} MATCHES "(win32|wince)" AND NOT ${KATIE_TYPE} STREQUAL SHARED)
    katie_definition(-DQT_MAKEDLL)
endif()

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    katie_definition(-DQT_NO_DEBUG -DNDEBUG)
endif()

include(${KATIE_MKSPECS_DIR}/tests/tests.cmake)

if(${KATIE_KEY} STREQUAL "auto")
    set(KATIE_KEY "${KATIE_ARCHITECTURE} ${KATIE_PLATFORM} ${KATIE_COMPILER} full-config")
endif()

if(EXISTS ${KATIE_MKSPECS_DIR}/${KATIE_PLATFORM}/${KATIE_PLATFORM}.cmake)
    include(${KATIE_MKSPECS_DIR}/${KATIE_PLATFORM}/${KATIE_PLATFORM}.cmake)
endif()

# for distributions to override build specifications
if(EXISTS ${KATIE_MKSPECS_DIR}/${KATIE_PLATFORM}/vendor.cmake)
    include(${KATIE_MKSPECS_DIR}/${KATIE_PLATFORM}/vendor.cmake)
endif()
