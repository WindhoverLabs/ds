
add_airliner_app_def(ds
    FILE DS
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_app.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_app.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_appdefs.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_cmds.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_cmds.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_file.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_file.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_msg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_msgdefs.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_table.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_table.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_verify.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_version.h
    INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/

    USER_DOCS   ${CMAKE_CURRENT_SOURCE_DIR}/user_doxy.in
    DESIGN_DOCS ${CMAKE_CURRENT_SOURCE_DIR}/detail_doxy.in
)


add_airliner_app_unit_test(ds-ut
    UTASSERT
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/ds_app_test.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/ds_app_test.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/ds_cmds_test.c 
        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/ds_cmds_test.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/ds_file_test.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/ds_file_test.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/ds_msgids.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/ds_platform_cfg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/ds_table_test.c 
        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/ds_table_test.h 
        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/ds_test_utils.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/ds_test_utils.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/ds_testrunner.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_app.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_app.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_appdefs.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_cmds.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_cmds.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_file.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_file.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_msg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_msgdefs.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_table.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_table.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_verify.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/ds_version.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../cfs_lib/fsw/src/cfs_utils.c
        
    INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/
)
