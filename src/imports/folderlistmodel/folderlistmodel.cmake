# add_definitions()
set(EXTRA_FOLDERLISTMODEL_LIBS KtDeclarative)

set(FOLDERLISTMODEL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/folderlistmodel/qdeclarativefolderlistmodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/folderlistmodel/folderlistmodel.cpp
)

set(FOLDERLISTMODEL_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/folderlistmodel/qdeclarativefolderlistmodel.h
)

katie_resources(${FOLDERLISTMODEL_SOURCES})
katie_resources(${FOLDERLISTMODEL_HEADERS})
katie_setup_flags()

add_library(qmlfolderlistmodelplugin ${KATIE_TYPE} ${FOLDERLISTMODEL_SOURCES} ${FOLDERLISTMODEL_HEADERS})
target_link_libraries(qmlfolderlistmodelplugin ${EXTRA_FOLDERLISTMODEL_LIBS})

install(
    TARGETS qmlfolderlistmodelplugin
    DESTINATION ${QT_IMPORTS_PATH_INST}/Qt/labs/folderlistmodel
)
install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/folderlistmodel/qmldir
    DESTINATION ${QT_IMPORTS_PATH_INST}/Qt/labs/folderlistmodel
)
