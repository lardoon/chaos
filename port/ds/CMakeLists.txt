add_executable(chaos.elf
    input.c
    main.c
    routines.c
    saves.c
    ${CMAKE_SOURCE_DIR}/port/linux/utils.c
    soundfx.c
    unused.c)

set_target_properties(chaos.elf
    PROPERTIES LINK_FLAGS "-specs=ds_arm9.specs -Wl,-gc-sections"
)
target_link_libraries(chaos.elf chaos_core sfx gfx translate
    ${FAT} ${NDS9} ${MM9})
add_nds(chaos.nds ${CMAKE_CURRENT_BINARY_DIR}/chaos.elf
    "Chaos;Chaos DS;By Quirky"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/icon.bmp")
