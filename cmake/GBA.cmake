get_filename_component(cwd ${CMAKE_CURRENT_LIST_FILE} PATH)
include(${cwd}/DevkitARM.cmake)

set(GBA 1)
include_directories(${DEVKITPRO}/libgba/include)
find_program(GBAFIX NAMES gbafix
    PATHS ${DEVKITARM}/bin)
set(CMAKE_FIND_ROOT_PATH ${DEVKITPRO}/libgba)
find_library(LIBGBA NAMES gba
    PATHS ${DEVKITPRO}/libgba/lib)

find_library(LIBMM NAMES mm
    PATHS ${DEVKITPRO}/libgba/lib)

set(THUMB_FLAGS_LIST
    -mthumb
    -mthumb-interwork
    -mcpu=arm7tdmi
    -mtune=arm7tdmi
    -fomit-frame-pointer
    -ffast-math)
string(REPLACE ";" " " THUMB_FLAGS_STR "${THUMB_FLAGS_LIST}")
set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} ${THUMB_FLAGS_STR}")
add_definitions(-DGBA)

function(add_gba output input)
    add_custom_command(OUTPUT ${output}
        COMMAND ${CMAKE_OBJCOPY} -O binary
        ${input} ${output}
        COMMAND ${GBAFIX} ${output}
        COMMENT "Convert ELF to GBA file"
        DEPENDS ${input})
    get_filename_component(_output_name ${output} NAME_WE)
    add_custom_target(${_output_name}_gba ALL DEPENDS ${output})
endfunction()
