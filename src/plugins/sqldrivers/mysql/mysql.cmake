include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/sqldrivers/mysql
)

set(MYSQLCDRIVER_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/sqldrivers/mysql/qsql_mysql.h
)

set(MYSQLCDRIVER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/sqldrivers/mysql/mysqlmain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sqldrivers/mysql/qsql_mysql.cpp
)

include_directories(${MYSQL_INCLUDES})

katie_setup_target(qsqlmysqlplugin ${MYSQLCDRIVER_SOURCES} ${MYSQLCDRIVER_HEADERS})

add_library(qsqlmysqlplugin MODULE ${qsqlmysqlplugin_SOURCES})
target_link_libraries(qsqlmysqlplugin KtSql ${MYSQL_LIBRARIES})
set_target_properties(qsqlmysqlplugin PROPERTIES OUTPUT_NAME qsqlmysql)

install(
    TARGETS qsqlmysqlplugin
    DESTINATION ${KATIE_PLUGINS_RELATIVE}/sqldrivers
)
