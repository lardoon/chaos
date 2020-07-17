include(CTest)

set(test_names
    select_spell_test
    order_table_test
    parse_options_test
    save_players_test
    save_arena_test
    save_globalstate_test
    random_test)

add_definitions(-DSRCDIR="${CMAKE_CURRENT_SOURCE_DIR}")
add_library(dummytestroutines STATIC
    ${CMAKE_SOURCE_DIR}/port/dummy/routines.c
    ${CMAKE_SOURCE_DIR}/port/linux/utils.c)

foreach(test_name ${test_names})
    add_executable(${test_name} ${test_name}.c)
    target_link_libraries(${test_name}
        chaos_core dummytestroutines gfx translate)
    add_test(${test_name} ${CMAKE_CURRENT_BINARY_DIR}/${test_name})
endforeach()
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS ${test_names})
