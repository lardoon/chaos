set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
enable_language(ASM)
add_executable(chaos.elf
    input.c
    main.c
    platform.c
    saves.c
    screen.c
    scroll.c
    soundfx.c
    sprites.c
    text.c
    vba.h
    vba.S)

set_target_properties(chaos.elf
    PROPERTIES LINK_FLAGS "-specs=gba.specs -Wl,-gc-sections"
)
target_link_libraries(chaos.elf chaos_core sfx gfx translate ${LIBGBA} ${LIBMM})

add_gba(chaos.gba ${CMAKE_CURRENT_BINARY_DIR}/chaos.elf)
