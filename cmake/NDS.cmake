get_filename_component(cwd ${CMAKE_CURRENT_LIST_FILE} PATH)
include(${cwd}/DevkitARM.cmake)

set(NDS 1)

include_directories(${DEVKITPRO}/libnds/include)

find_file(DEFAULT_ARM7 NAMES default.arm7 default.elf
    PATHS ${DEVKITPRO}/libnds
    DOC "default ARM7 executable"
    NO_CMAKE_FIND_ROOT_PATH)
set(CMAKE_FIND_ROOT_PATH ${DEVKITPRO}/libnds)
find_library(NDS9 NAMES nds9 PATHS ${DEVKITPRO}/libnds/lib)
find_library(NDS7 NAMES nds7 PATHS ${DEVKITPRO}/libnds/lib)
find_library(FAT NAMES fat PATHS ${DEVKITPRO}/libnds/lib)
find_library(MM9 NAMES mm9 PATHS ${DEVKITPRO}/libnds/lib)
find_library(MM7 NAMES mm7 PATHS ${DEVKITPRO}/libnds/lib)
find_program(NDSTOOL NAMES ndstool
    PATHS ${DEVKITARM}/bin)
find_program(DLDITOOL NAMES dlditool
    PATHS ${DEVKITARM}/bin)

add_definitions(-DARM9)
set(THUMB_FLAGS_LIST
    -mthumb
    -mthumb-interwork
    -march=armv5te
    -mtune=arm946e-s
    -fomit-frame-pointer
    -ffast-math)
string(REPLACE ";" " " THUMB_FLAGS_STR "${THUMB_FLAGS_LIST}")
set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} ${THUMB_FLAGS_STR}")

if(NOT DEFAULT_ARM7)
    message(FATAL_ERROR "Cannot find default ARM7 executable in ${DEVKITPRO}/libnds")
endif()

function(add_nds output input)
    # elf > 9
    # 9 + 7 > nds
    get_filename_component(_output_name ${output} NAME_WE)
    get_filename_component(arm9
        ${CMAKE_CURRENT_BINARY_DIR}/${_output_name}.arm9 NAME_WE)
    add_custom_command(OUTPUT ${output}
        COMMAND ${CMAKE_OBJCOPY} -O binary
        ${input} ${arm9}
        COMMAND ${NDSTOOL}
        ARGS -c ${output} -9 ${arm9} -7 ${DEFAULT_ARM7}
        DEPENDS ${input})
    add_custom_target(${_output_name}_nds ALL DEPENDS ${output})
endfunction()

function(dldi_nds output input dldifile)
    add_custom_command(OUTPUT ${output}
        COMMAND ${CMAKE_COMMAND} -E copy ${input} ${output}
        COMMAND ${DLDITOOL} ${dldifile} ${output}
        MAIN_DEPENDENCY ${input}
        DEPENDS ${dldifile})
    get_filename_component(_output_name ${output} NAME_WE)
    add_custom_target(dldi DEPENDS ${output})
endfunction()
