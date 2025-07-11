# add_definitions()
set(EXTRA_SVG_LIBS
    KtCore KtXml KtGui
    ${DEFLATE_LIBRARIES}
)

set(SVG_PUBLIC_HEADERS
    QSvgRenderer
)

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/privateinclude
    ${CMAKE_BINARY_DIR}/include/QtCore
    ${CMAKE_BINARY_DIR}/include/QtXml
    ${CMAKE_BINARY_DIR}/include/QtGui
    ${CMAKE_BINARY_DIR}/include/QtSvg
    ${DEFLATE_INCLUDES}
)

set(SVG_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvggraphics_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvghandler_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvgnode_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvgstructure_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvgstyle_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvgfont_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvgtinydocument_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvgrenderer.h
)

set(SVG_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvggraphics.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvghandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvgnode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvgstructure.cpp  
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvgstyle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvgfont.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvgtinydocument.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvgrenderer.cpp
)

katie_generate_misc("${SVG_HEADERS}" QtSvg)
katie_generate_public("${SVG_PUBLIC_HEADERS}" QtSvg)
katie_generate_package(KtSvg "KtCore KtXml KtGui")
katie_setup_target(KtSvg ${SVG_SOURCES} ${SVG_HEADERS})

add_library(KtSvg ${KATIE_TYPE} ${KtSvg_SOURCES})
target_link_libraries(KtSvg PRIVATE ${EXTRA_SVG_LIBS})
set_target_properties(KtSvg PROPERTIES
    VERSION ${KATIE_MAJOR}.${KATIE_MINOR}
    SOVERSION ${KATIE_VERSION}
    EXPORT_NAME Svg
)

install(
    TARGETS KtSvg
    EXPORT KatieTargets
    DESTINATION ${KATIE_LIBRARIES_PATH}
    COMPONENT Runtime
)

install(
    DIRECTORY ${CMAKE_BINARY_DIR}/include/QtSvg
    DESTINATION ${KATIE_HEADERS_PATH}
    COMPONENT Devel
)

katie_optimize_headers(${KATIE_HEADERS_PATH}/QtSvg)
