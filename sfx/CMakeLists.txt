set(WAVS
    ${CMAKE_CURRENT_SOURCE_DIR}/attack.wav
    ${CMAKE_CURRENT_SOURCE_DIR}/beam.wav
    ${CMAKE_CURRENT_SOURCE_DIR}/chosen.wav
    ${CMAKE_CURRENT_SOURCE_DIR}/cpustart.wav
    ${CMAKE_CURRENT_SOURCE_DIR}/electro.wav
    ${CMAKE_CURRENT_SOURCE_DIR}/fire.wav
    ${CMAKE_CURRENT_SOURCE_DIR}/gooey.wav
    ${CMAKE_CURRENT_SOURCE_DIR}/justice.wav
    ${CMAKE_CURRENT_SOURCE_DIR}/menu.wav
    ${CMAKE_CURRENT_SOURCE_DIR}/range.wav
    ${CMAKE_CURRENT_SOURCE_DIR}/scream.wav
    ${CMAKE_CURRENT_SOURCE_DIR}/spellstep.wav
    ${CMAKE_CURRENT_SOURCE_DIR}/spellsuccess.wav
    ${CMAKE_CURRENT_SOURCE_DIR}/urgh.wav
    ${CMAKE_CURRENT_SOURCE_DIR}/walk.wav)

if(GBA OR NDS)
    # maxmod
    set(SOUNDBANK_BIN soundbank.bin)
    set(SOUNDBANK_H soundbank.h)
    set(SOUNDBANK_C soundbank.c)
    set(SOUNDBANKMM_H soundbankmm.h)

    find_program(RAW2C raw2c ${DEVKITARM}/bin)
    if(NOT RAW2C)
        message(FATAL_ERROR "unable to find raw2c")
    endif()

    find_program(MMUTIL mmutil ${DEVKITARM}/bin)
    if(NOT MMUTIL)
        message(FATAL_ERROR "unable to find mmutil")
    endif()

    if(NDS)
        set(MMFLAGS -d)
    endif()

    # from *.wav, generate soundbank.bin with maxmodutil
    add_custom_command(OUTPUT ${SOUNDBANK_BIN} ${SOUNDBANKMM_H}
        COMMAND ${MMUTIL}
        ARGS ${WAVS} ${MMFLAGS} -o${SOUNDBANK_BIN} -h${SOUNDBANKMM_H}
        COMMENT "Run maxmodutil to create soundbank.bin"
        DEPENDS ${WAVS})

    # from soundbank.bin generate soundbank.c
    add_custom_command(OUTPUT ${SOUNDBANK_H} ${SOUNDBANK_C}
        COMMAND ${RAW2C}
        ARGS ${SOUNDBANK_BIN}
        COMMENT "Run rawc2 to create soundbank.c/h"
        DEPENDS ${SOUNDBANK_BIN})

    add_library(sfx ${SOUNDBANK_C} ${SOUNDBANK_H})

else()
    set(res_dir ${CMAKE_CURRENT_BINARY_DIR})
    if(ANDROID)
        set(res_dir ${CMAKE_BINARY_DIR}/port/android/res/raw)
        set(extradeps chaos_sym_links)
    elseif(EMSCRIPTEN)
        set(res_dir ${CMAKE_BINARY_DIR}/port/linux/sfx)
    endif()
    # wav2ogg
    find_program(AVCONV avconv
        DOC "Path to avconv or ffmpeg for wav to ogg conversion")
    set(OGGFLAGS -v quiet -y -acodec libvorbis -aq 20 -vn -ac 2)
    if(NOT AVCONV)
        find_program(FFMPEG ffmpeg)
        set(wav2ogg ${FFMPEG})
    else()
        set(wav2ogg ${AVCONV})
    endif()
    file(MAKE_DIRECTORY ${res_dir})
    set(ogg_files "")
    foreach(WAV ${WAVS})
        get_filename_component(FN ${WAV} NAME_WE)
        set(T ${res_dir}/${FN}.ogg)
        list(APPEND ogg_files ${T})
        add_custom_command(OUTPUT ${T}
            COMMAND ${wav2ogg}
            ARGS -i ${WAV} ${OGGFLAGS} ${T}
            MAIN_DEPENDENCY ${WAV})
    endforeach()
    add_custom_target(oggify ALL DEPENDS ${ogg_files} ${extradeps})
endif()
