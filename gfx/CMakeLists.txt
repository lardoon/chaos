
# glob each palette directory
# for each directory
#    run grit on the pcx files there

set(PAL_FLAGS -mRT -gB4)
find_program(GRIT grit /usr/bin $ENV{DEVKITARM}/bin /opt/devkitpro/tools/bin)
if(NOT GRIT)
    message(FATAL_ERROR "unable to find grit")
endif()
set(GRITWRAPPER ${CMAKE_SOURCE_DIR}/tools/gritwrapper.py)

file(GLOB DIRS palette*)
list(SORT DIRS)
set(generated_src "")
foreach(DIR IN LISTS DIRS)
    get_filename_component(DIRNAME ${DIR} NAME)
    file(GLOB PCXES ${DIR}/*.pcx)
    list(SORT PCXES)
    set(GENGFX_TARGETS ${CMAKE_CURRENT_BINARY_DIR}/${DIRNAME}/${DIRNAME}.c)
    foreach(PCX IN LISTS PCXES)
        get_filename_component(FN ${PCX} NAME_WE)
        list(APPEND GENGFX_TARGETS
            ${CMAKE_CURRENT_BINARY_DIR}/${DIRNAME}/${FN}.c)
    endforeach()
    set(FLAGS ${PAL_FLAGS})
    if(${DIRNAME} STREQUAL palettea OR ${DIRNAME} STREQUAL palettec)
        set(FLAGS -mR! -m! -gB4)
    elseif(${DIRNAME} STREQUAL paletteb)
        set(FLAGS -mRt -gB8)
    endif()
    set(GRIT_FLAGS ${FLAGS} -ftc -q -gu16 -pS -O${DIRNAME})
    set(BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/${DIRNAME})
    # transform from %.pcx to %.c
    add_custom_command(OUTPUT ${GENGFX_TARGETS}
        COMMAND ${PYTHON_EXECUTABLE}
        ARGS ${GRITWRAPPER} ${BUILD_DIR} ${GRIT}
        ${PCXES} ${GRIT_FLAGS}
        MAIN_DEPENDENCY ${PCXES}
        DEPENDS ${GRITWRAPPER})
    add_custom_target(gengfx_${DIRNAME} ALL DEPENDS ${GENGFX_TARGETS})
    list(APPEND generated_src ${GENGFX_TARGETS})
endforeach()
set_source_files_properties(${generated_src}
    PROPERTIES GENERATED TRUE)
if(NOT ANDROID)
add_library(gfx ${generated_src})
endif()
