set(GENSPELL_TARGETS
    ${CMAKE_CURRENT_BINARY_DIR}/spellenums.h
    ${CMAKE_CURRENT_BINARY_DIR}/spelldata.c)
set(GENSFX_TARGETS
    ${CMAKE_CURRENT_BINARY_DIR}/sound_data.h
    ${CMAKE_CURRENT_BINARY_DIR}/sound_data.c)

if(NOT ANDROID)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(chaos_core
    arena.c
    arena.h
    casting.c
    casting.h
    chaos.c
    chaos.h
    computer.c
    computer.h
    creature.c
    creature.h
    cursor_defines.h
    editname.c
    editname.h
    examine.c
    examine.h
    gamemenu.c
    gamemenu.h
    gfx.c
    gfx.h
    images.h
    input.c
    input.h
    magic.c
    magic.h
    main.c
    movement.c
    movement.h
    options.c
    options.h
    palettes.h
    pathfinder.c
    pathfinder.h
    platform.h
    players.c
    players.h
    porting.h
    rand.c
    rand.h
    spelldata.h
    spellselect.c
    spellselect.h
    splash.c
    splash.h
    translate.c
    translate.h
    text16.c
    text16.h
    touch.c
    touch.h
    wizard_data.h
    wizards.c
    wizards.h
    ${GENSFX_TARGETS}
    ${GENSPELL_TARGETS})

add_library(chaos_core_headless
    arena.c
    arena.h
    casting.c
    casting.h
    chaos.c
    chaos.h
    computer.c
    computer.h
    creature.c
    creature.h
    cursor_defines.h
    editname.c
    editname.h
    examine.c
    examine.h
    gamemenu.c
    gamemenu.h
    gfx.c
    gfx.h
    images.h
    input.c
    input.h
    magic.c
    magic.h
    main.c
    movement.c
    movement.h
    options.c
    options.h
    palettes.h
    pathfinder.c
    pathfinder.h
    platform.h
    players.c
    players.h
    porting.h
    rand.c
    rand.h
    spelldata.h
    spellselect.c
    spellselect.h
    splash.c
    splash.h
    translate.c
    translate.h
    text16.c
    text16.h
    touch.c
    touch.h
    wizard_data.h
    wizards.c
    wizards.h
    jWrite.h
    jWrite.c
    output.h
    output.c
    ${GENSFX_TARGETS}
    ${GENSPELL_TARGETS})
target_compile_definitions(chaos_core_headless PRIVATE _HEADLESS)
if(EMSCRIPTEN)
add_library(chaos_core_replay
    arena.c
    arena.h
    casting.c
    casting.h
    chaos.c
    chaos.h
    computer.c
    computer.h
    creature.c
    creature.h
    cursor_defines.h
    editname.c
    editname.h
    examine.c
    examine.h
    gamemenu.c
    gamemenu.h
    gfx.c
    gfx.h
    images.h
    input.c
    input.h
    magic.c
    magic.h
    main.c
    movement.c
    movement.h
    options.c
    options.h
    palettes.h
    pathfinder.c
    pathfinder.h
    platform.h
    players.c
    players.h
    porting.h
    rand.c
    rand.h
    spelldata.h
    spellselect.c
    spellselect.h
    splash.c
    splash.h
    translate.c
    translate.h
    text16.c
    text16.h
    touch.c
    touch.h
    wizard_data.h
    wizards.c
    wizards.h
    ${GENSFX_TARGETS}
    ${GENSPELL_TARGETS})
target_compile_definitions(chaos_core_replay PRIVATE _REPLAY)
endif()
endif()

set(GENSPELL ${CMAKE_SOURCE_DIR}/tools/genspellenum.py)
set(GENSFX ${CMAKE_SOURCE_DIR}/tools/gensfx.py)

set(GENSPELL_SOURCE ${CMAKE_SOURCE_DIR}/data/spelldata.csv)

add_custom_command(OUTPUT ${GENSPELL_TARGETS}
    COMMAND ${PYTHON_EXECUTABLE}
    ARGS ${GENSPELL} ${GENSPELL_SOURCE} ${GENSPELL_TARGETS}
    MAIN_DEPENDENCY ${GENSPELL_SOURCE}
    DEPENDS ${GENSPELL})
add_custom_target(genspells ALL DEPENDS ${GENSPELL_TARGETS})

set(GENSFX_SOURCE
    ${CMAKE_SOURCE_DIR}/sfx/attack.wav
    ${CMAKE_SOURCE_DIR}/sfx/beam.wav
    ${CMAKE_SOURCE_DIR}/sfx/chosen.wav
    ${CMAKE_SOURCE_DIR}/sfx/cpustart.wav
    ${CMAKE_SOURCE_DIR}/sfx/electro.wav
    ${CMAKE_SOURCE_DIR}/sfx/fire.wav
    ${CMAKE_SOURCE_DIR}/sfx/gooey.wav
    ${CMAKE_SOURCE_DIR}/sfx/justice.wav
    ${CMAKE_SOURCE_DIR}/sfx/menu.wav
    ${CMAKE_SOURCE_DIR}/sfx/range.wav
    ${CMAKE_SOURCE_DIR}/sfx/scream.wav
    ${CMAKE_SOURCE_DIR}/sfx/spellstep.wav
    ${CMAKE_SOURCE_DIR}/sfx/spellsuccess.wav
    ${CMAKE_SOURCE_DIR}/sfx/urgh.wav
    ${CMAKE_SOURCE_DIR}/sfx/walk.wav)

add_custom_command(OUTPUT ${GENSFX_TARGETS}
    COMMAND ${PYTHON_EXECUTABLE}
    ARGS ${GENSFX} ${GENSFX_SOURCE} ${GENSFX_TARGETS}
    MAIN_DEPENDENCY ${GENSFX_SOURCE}
    DEPENDS ${GENSFX})
add_custom_target(gensfx ALL DEPENDS ${GENSFX_TARGETS})
set_source_files_properties(${GENSPELL_TARGETS} ${GENSFX_TARGETS}
    PROPERTIES GENERATED TRUE)
