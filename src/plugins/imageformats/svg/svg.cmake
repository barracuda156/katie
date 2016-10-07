set(QSVGPLUGIN_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/imageformats/svg/qsvgiohandler.h
)

set(QSVGPLUGIN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/imageformats/svg/qsvgplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imageformats/svg/qsvgiohandler.cpp
)

katie_setup_target(qsvgplugin ${QSVGPLUGIN_SOURCES} ${QSVGPLUGIN_HEADERS})

add_library(qsvgplugin MODULE ${qsvgplugin_SOURCES})
target_link_libraries(qsvgplugin KtCore KtGui KtSvg)
set_target_properties(qsvgplugin PROPERTIES OUTPUT_NAME qsvg)

install(
    TARGETS qsvgplugin
    DESTINATION ${KATIE_PLUGINS_RELATIVE}/imageformats
)
