
add_airliner_app_def(ds
    FILE DS
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_app.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_cmds.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_file.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_table.c
   
    INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/

    USER_DOCS   ${CMAKE_CURRENT_SOURCE_DIR}/user_doxy.in
    DESIGN_DOCS ${CMAKE_CURRENT_SOURCE_DIR}/detail_doxy.in
)


add_airliner_app_unit_test(ds-ut
    UTASSERT
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/ds_app_test.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/ds_cmds_test.c 
        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/ds_file_test.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/ds_table_test.c 
        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/ds_test_utils.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/ds_testrunner.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_app.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_cmds.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_file.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_table.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../cfs_lib/fsw/src/cfs_utils.c
        
    INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/
        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/
)
