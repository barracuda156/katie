set(CORE_HEADERS
    ${CORE_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qfactoryinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qpluginloader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qlibrary.h
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qlibrary_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qplugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/quuid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qfactoryloader_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qsystemlibrary_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qelfparser_p.h
)

set(CORE_SOURCES
    ${CORE_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qpluginloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qfactoryloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/quuid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qlibrary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qelfparser_p.cpp
)

if(KATIE_PLATFORM STREQUAL "win32")
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qlibrary_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qsystemlibrary.cpp
    )
endif()

if(UNIX OR KATIE_PLATFORM STREQUAL "integrity")
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qlibrary_unix.cpp
    )
endif()

