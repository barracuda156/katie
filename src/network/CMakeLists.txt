# add_definitions()
set(EXTRA_NETWORK_LIBS KtCore)

set(NETWORK_PUBLIC_HEADERS
    QAbstractSocket
    QHostAddress
    QHostInfo
    QLocalServer
    QLocalSocket
    QNetworkInterface
    QTcpServer
    QTcpSocket
    QUdpSocket
    QCryptographicHash
)

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/privateinclude
    ${CMAKE_BINARY_DIR}/include/QtCore
    ${CMAKE_BINARY_DIR}/include/QtNetwork
)

set(NETWORK_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qcryptographichash.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostaddress.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostaddress_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostinfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostinfo_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkinterface_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/socket/qabstractsocketengine_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/socket/qabstractsocket.h
    ${CMAKE_CURRENT_SOURCE_DIR}/socket/qabstractsocket_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/socket/qtcpsocket.h
    ${CMAKE_CURRENT_SOURCE_DIR}/socket/qtcpsocket_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/socket/qudpsocket.h
    ${CMAKE_CURRENT_SOURCE_DIR}/socket/qtcpserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/socket/qlocalserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/socket/qlocalserver_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/socket/qlocalsocket.h
    ${CMAKE_CURRENT_SOURCE_DIR}/socket/qlocalsocket_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/socket/qnet_unix_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qnetworkcommon_p.h
)

set(NETWORK_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qcryptographichash.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostaddress.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkinterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostinfo_unix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkinterface_unix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/socket/qabstractsocketengine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/socket/qabstractsocketengine_unix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/socket/qabstractsocket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/socket/qtcpsocket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/socket/qudpsocket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/socket/qtcpserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/socket/qlocalsocket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/socket/qlocalserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/socket/qlocalsocket_unix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/socket/qlocalserver_unix.cpp
)

if(WITH_XXHASH AND XXHASH_FOUND)
    set(EXTRA_NETWORK_LIBS
        ${EXTRA_NETWORK_LIBS}
        ${XXHASH_LIBRARIES}
    )
    include_directories(${XXHASH_INCLUDES})
else()
    add_definitions(-DXXH_INLINE_ALL)
    include_directories(${CMAKE_SOURCE_DIR}/src/3rdparty/xxHash)
endif()

katie_generate_misc("${NETWORK_HEADERS}" QtNetwork)
katie_generate_public("${NETWORK_PUBLIC_HEADERS}" QtNetwork)
katie_generate_package(KtNetwork "KtCore")
katie_setup_target(KtNetwork ${NETWORK_SOURCES} ${NETWORK_HEADERS})

add_library(KtNetwork ${KATIE_TYPE} ${KtNetwork_SOURCES})
target_link_libraries(KtNetwork PRIVATE ${EXTRA_NETWORK_LIBS})
set_target_properties(KtNetwork PROPERTIES
    VERSION ${KATIE_MAJOR}.${KATIE_MINOR}
    SOVERSION ${KATIE_VERSION}
    EXPORT_NAME Network
)

install(
    TARGETS KtNetwork
    EXPORT KatieTargets
    DESTINATION ${KATIE_LIBRARIES_PATH}
    COMPONENT Runtime
)

install(
    DIRECTORY ${CMAKE_BINARY_DIR}/include/QtNetwork
    DESTINATION ${KATIE_HEADERS_PATH}
    COMPONENT Devel
)

katie_optimize_headers(${KATIE_HEADERS_PATH}/QtNetwork)
