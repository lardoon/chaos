if(EMSCRIPTEN)
    set(platform2 emscripten.c)
    get_filename_component(EMSCRIPTEN_HOME ${CMAKE_C_COMPILER} PATH)
    set(SDL_INCLUDE_DIR ${EMSCRIPTEN_HOME}/system)
    # creates bitcode
    set(CMAKE_EXECUTABLE_SUFFIX ".bc")
else()
    set(platform2 platform2.c saves.c)
    find_package(SDL REQUIRED)
    find_package(SDL_mixer REQUIRED)
endif()
include_directories(${SDL_INCLUDE_DIR})

add_executable(chaos
                cmdline.c
                cmdline.h
                main.c
                platform.c
                ${platform2}
                screen.c
                scroll.c
                sprites.c
                sprites.h
                utils.c
                utils.h
                winkeys.c
                winkeys.h
                winsound.c
                winsound.h
                wmhandler.c
                wmhandler.h)

add_executable(chaos_headless
                cmdline.c
                cmdline.h
                main.c
                platform.c
                ${platform2}
                screen.c
                scroll.c
                sprites.c
                sprites.h
                utils.c
                utils.h
                winkeys.c
                winkeys.h
                winsound.c
                winsound.h
                wmhandler.c
                wmhandler.h)
target_compile_definitions(chaos_headless PRIVATE _HEADLESS)
target_link_libraries(chaos chaos_core gfx translate
    ${SDL_LIBRARY} ${SDLMIXER_LIBRARY})
target_link_libraries(chaos_headless chaos_core_headless gfx translate
    ${SDL_LIBRARY} ${SDLMIXER_LIBRARY})

if(EMSCRIPTEN)
    add_executable(chaos_replay
                cmdline.c
                cmdline.h
                main.c
                platform.c
                ${platform2}
                screen.c
                scroll.c
                sprites.c
                sprites.h
                utils.c
                utils.h
                winkeys.c
                winkeys.h
                winsound.c
                winsound.h
                wmhandler.c
                wmhandler.h)
    target_compile_definitions(chaos_replay PRIVATE _REPLAY)
    target_link_libraries(chaos_replay chaos_core_replay gfx translate
        ${SDL_LIBRARY} ${SDLMIXER_LIBRARY})


    set(SOUNDFX_ARGS  --preload-file sfx)
    set(BC_FLAGS -O2)

    add_custom_command(OUTPUT chaos.js
        COMMAND ${CMAKE_C_COMPILER}
        ARGS ${BC_FLAGS} chaos.bc -o ${CMAKE_CURRENT_BINARY_DIR}/chaos.js
        -s EXPORTED_FUNCTIONS="['_main']"
        -s USE_SDL
        -s ASYNCIFY=1
        ${SOUNDFX_ARGS}
        --pre-js ${CMAKE_CURRENT_SOURCE_DIR}/module-pre.js
        --js-library ${CMAKE_CURRENT_SOURCE_DIR}/library.js
        DEPENDS chaos module-pre.js library.js ${CMAKE_CURRENT_BINARY_DIR}/sfx)
    add_custom_target(createjs ALL DEPENDS chaos.js)

    add_custom_command(OUTPUT chaos_replay.js
        COMMAND ${CMAKE_C_COMPILER}
        ARGS ${BC_FLAGS} chaos_replay.bc -o ${CMAKE_CURRENT_BINARY_DIR}/chaos_replay.js
        -s EXPORTED_FUNCTIONS="['_main']"
        -s USE_SDL
        -s ASYNCIFY=1
        ${SOUNDFX_ARGS}
        --pre-js ${CMAKE_CURRENT_SOURCE_DIR}/module-pre.js
        --js-library ${CMAKE_CURRENT_SOURCE_DIR}/library.js
        DEPENDS chaos_replay module-pre.js library.js ${CMAKE_CURRENT_BINARY_DIR}/sfx)
    add_custom_target(createreplayjs ALL DEPENDS chaos_replay.js)

    # create html from js and footer:
    #chaos.html : $(srcdir)/header.html chaos.js $(srcdir)/footer.html
    #cat $^ > $@
    add_custom_command(OUTPUT chaos.html
        COMMAND cat
        ARGS
        ${CMAKE_CURRENT_SOURCE_DIR}/header.html
        ${CMAKE_CURRENT_BINARY_DIR}/chaos.js
        ${CMAKE_CURRENT_SOURCE_DIR}/footer.html
        > ${CMAKE_CURRENT_BINARY_DIR}/chaos.html
        DEPENDS chaos.js header.html footer.html)

    add_custom_target(createhtml ALL DEPENDS chaos.html)

    add_custom_command(OUTPUT chaos_replay.html
        COMMAND cat
        ARGS
        ${CMAKE_CURRENT_SOURCE_DIR}/header.html
        ${CMAKE_CURRENT_BINARY_DIR}/chaos_replay.js
        ${CMAKE_CURRENT_SOURCE_DIR}/footer.html
        > ${CMAKE_CURRENT_BINARY_DIR}/chaos_replay.html
        DEPENDS chaos_replay.js header.html footer.html)

    add_custom_target(createreplayhtml ALL DEPENDS chaos_replay.html)

    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/img)
    set(resizedicons "")
    foreach(size 16 32 48 64 128)
        set(iconfile ${CMAKE_CURRENT_BINARY_DIR}/img/icon${size}.png)
        add_custom_command(OUTPUT ${iconfile}
            COMMAND convert
            ARGS ${CMAKE_SOURCE_DIR}/port/android/extras/design_assets/gooey-blob-512.png
            -resize ${size}x${size}
            ${iconfile})
        list(APPEND resizedicons ${iconfile})
    endforeach()
    add_custom_target(converticons ALL DEPENDS ${resizedicons})
endif()
