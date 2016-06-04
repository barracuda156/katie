set(NETWORK_HEADERS
    ${NETWORK_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qssl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslcertificate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslcertificate_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslconfiguration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslconfiguration_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslcipher.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslcipher_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslerror.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslkey.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslsocket.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslsocket_openssl_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslsocket_p.h
)

set(NETWORK_SOURCES
    ${NETWORK_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qssl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslcertificate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslconfiguration.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslcipher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslerror.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslkey.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslsocket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslsocket_openssl.cpp
)

if(WITH_OPENSSL AND OPENSSL_FOUND)
    set(EXTRA_NETWORK_LIBS
        ${EXTRA_NETWORK_LIBS}
        ${OPENSSL_LIBRARIES}
    )
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()
