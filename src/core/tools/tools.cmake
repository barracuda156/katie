set(CORE_HEADERS
    ${CORE_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qalgorithms.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qbitarray.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qbytearray.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qbytearraymatcher.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qbytedata_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qcache.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qchar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qcontainerfwd.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qcryptographichash.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qdatetime.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qdatetime_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qeasingcurve.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qhash.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qline.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qlinkedlist.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qlist.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qlocale.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qlocale_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qlocale_tools_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qlocale_data_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qmap.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qmargins.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qcontiguouscache.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qpodlist_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qpoint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qqueue.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qrect.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qregexp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qringbuffer_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qtools_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qscopedpointer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qscopedpointer_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qscopedvaluerollback.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qshareddata.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qsharedpointer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qsharedpointer_impl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qset.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qsimd_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qsize.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qstack.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qstring.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qstringbuilder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qstringlist.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qstringmatcher.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qtextboundaryfinder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qtimeline.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qelapsedtimer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qunicodetables_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qvarlengtharray.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qvector.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qcommandlineoption.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qcommandlineparser.h

    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qiterator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qpair.h

    # XXX: obsolete?
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qmultihash.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qscopedarraypointer.h
)

set(CORE_SOURCES
    ${CORE_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qbitarray.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qbytearray.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qbytearraymatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qcryptographichash.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qdatetime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qeasingcurve.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qelapsedtimer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qhash.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qlinkedlist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qlist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qlocale.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qlocale_tools.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qpoint.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qmap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qmargins.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qcontiguouscache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qrect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qregexp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qshareddata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qsharedpointer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qsimd.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qsize.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qstring.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qstringbuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qstringlist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qtextboundaryfinder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qtimeline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qvector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qvsnprintf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qcommandlineoption.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/qcommandlineparser.cpp
)

if(KATIE_PLATFORM STREQUAL "mac")
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/qelapsedtimer_mac.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/qlocale_mac.mm
    )
elseif(KATIE_PLATFORM STREQUAL "blackberry")
    set(CORE_HEADERS
        ${CORE_HEADERS}
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/qlocale_blackberry.h
    )
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/qelapsedtimer_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/qlocale_blackberry.cpp
    )
elseif(KATIE_PLATFORM STREQUAL "win32")
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/qelapsedtimer_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/qlocale_win.cpp
    )
elseif(KATIE_PLATFORM STREQUAL "integrity" OR UNIX)
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/qelapsedtimer_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/qlocale_unix.cpp
    )
else()
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/qelapsedtimer_generic.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/qlocale_unix.cpp
    )
endif()

if(WITH_ICU AND ICU_FOUND)
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/qlocale_icu.cpp
    )
    add_definitions(-DQT_USE_ICU)
endif()

if(WITH_HARFBUZZ AND HARFBUZZ_FOUND)
    set(EXTRA_CORE_LIBS
        ${EXTRA_CORE_LIBS}
        ${HARFBUZZ_LIBRARIES}
    )
    include_directories(
        ${HARFBUZZ_INCLUDE_DIRS}
    )
else()
    # TODO: move to main CMakeLists?
    add_definitions(-DHB_EXPORT=Q_CORE_EXPORT)
    set(CORE_HEADERS
        ${CORE_HEADERS}
        ${CMAKE_SOURCE_DIR}/src/3rdparty/harfbuzz/src/harfbuzz.h
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/qharfbuzz_p.h
    )
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_SOURCE_DIR}/src/3rdparty/harfbuzz/src/harfbuzz-buffer.c
        ${CMAKE_SOURCE_DIR}/src/3rdparty/harfbuzz/src/harfbuzz-gdef.c
        ${CMAKE_SOURCE_DIR}/src/3rdparty/harfbuzz/src/harfbuzz-gsub.c
        ${CMAKE_SOURCE_DIR}/src/3rdparty/harfbuzz/src/harfbuzz-gpos.c
        ${CMAKE_SOURCE_DIR}/src/3rdparty/harfbuzz/src/harfbuzz-impl.c
        ${CMAKE_SOURCE_DIR}/src/3rdparty/harfbuzz/src/harfbuzz-open.c
        ${CMAKE_SOURCE_DIR}/src/3rdparty/harfbuzz/src/harfbuzz-stream.c
        ${CMAKE_SOURCE_DIR}/src/3rdparty/harfbuzz/src/harfbuzz-shaper-all.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/qharfbuzz.cpp
    )
    include_directories(
        ${CMAKE_SOURCE_DIR}/src/3rdparty/harfbuzz/src
    )
endif()

# Note: libm should be present by default becaue this is C++
if(NOT KATIE_PLATFORM MATCHES "(mac|vxworks)" AND UNIX)
    set(EXTRA_CORE_LIBS
        ${EXTRA_CORE_LIBS}
        m
    )
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}/src/3rdparty/md5
    ${CMAKE_SOURCE_DIR}/src/3rdparty/md4
    ${CMAKE_SOURCE_DIR}/src/3rdparty/sha3
)
