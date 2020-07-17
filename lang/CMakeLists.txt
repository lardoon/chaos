
set(TRANSLATE_SRC
    chaos_pot.c
    es_po.c)

if(NOT ANDROID)
    add_library(translate ${TRANSLATE_SRC})
endif()

add_custom_command(OUTPUT es_po.c
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/poc2c.sh
    ARGS ${CMAKE_CURRENT_SOURCE_DIR}/es.po > ${CMAKE_CURRENT_BINARY_DIR}/es_po.c
    MAIN_DEPENDENCY es.po
    DEPENDS poc2c.sh)

add_custom_command(OUTPUT chaos_pot.c
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/poc2c.sh
    ARGS ${CMAKE_CURRENT_SOURCE_DIR}/chaos.pot
    > ${CMAKE_CURRENT_BINARY_DIR}/chaos_pot.c
    MAIN_DEPENDENCY chaos.pot
    DEPENDS poc2c.sh)

add_custom_target(gentranslate ALL DEPENDS ${TRANSLATE_SRC})
