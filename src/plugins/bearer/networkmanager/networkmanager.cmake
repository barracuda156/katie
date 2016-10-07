include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/bearer/networkmanager
)

set(QNMBEARERPLUGIN_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/bearer/networkmanager/qnmdbushelper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/bearer/networkmanager/qnetworkmanagerservice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/bearer/networkmanager/qnetworkmanagerengine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/bearer/qnetworksession_impl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/bearer/qbearerengine_impl.h
)

set(QNMBEARERPLUGIN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/bearer/networkmanager/nmmain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bearer/networkmanager/qnmdbushelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bearer/networkmanager/qnetworkmanagerservice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bearer/networkmanager/qnetworkmanagerengine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bearer/qnetworksession_impl.cpp
)

katie_setup_target(qnmbearerplugin ${QNMBEARERPLUGIN_SOURCES} ${QNMBEARERPLUGIN_HEADERS})

add_library(qnmbearerplugin MODULE ${qnmbearerplugin_SOURCES})
target_link_libraries(qnmbearerplugin KtCore KtNetwork KtDBus)
set_target_properties(qnmbearerplugin PROPERTIES OUTPUT_NAME qnmbearer)

install(
    TARGETS qnmbearerplugin
    DESTINATION ${KATIE_PLUGINS_RELATIVE}/bearer
)
