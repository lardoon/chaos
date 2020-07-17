
set(ANT ant CACHE FILEPATH "ANT build program")
if(NOT ANDROID_NDK)
    set(ANDROID_NDK /opt/android-ndk CACHE FILEPATH "Android NDK root directory")
endif()
if(NOT ANDROID_SDK)
    set(ANDROID_SDK /opt/android-sdk CACHE FILEPATH "Android SDK root directory")
endif()
if(NOT EXISTS ${ANDROID_NDK})
    message(FATAL_ERROR "dir ${ANDROID_NDK} does not exist")
endif()
if(NOT EXISTS ${ANDROID_SDK})
    message(FATAL_ERROR "dir ${ANDROID_SDK} does not exist")
endif()
set(NDKBUILD ${ANDROID_NDK}/ndk-build)
set(ANDROID_TOOL ${ANDROID_SDK}/tools/android)
set(ANDROID_BUILD debug CACHE STRING "Android build type")
set(NDKBUILD_VERBOSE 0 CACHE BOOL "Android ndk build verbosity")
set(ANDROID_APP_ABI armeabi armeabi-v7a x86
    CACHE STRING "Android application ABIs to build")
set(ANDROID_TARGET android-21 CACHE STRING "Android Target Version")
set_property(CACHE ANDROID_BUILD PROPERTY STRINGS debug release)

file(RELATIVE_PATH
    RELATIVE_CMAKE_BINARY_DIR
    ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR})
file(RELATIVE_PATH
    RELATIVE_BUILD_FROM_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR})
file(GLOB CHAOS_SRC
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/chaos/*.c)
list(SORT CHAOS_SRC)
string(REGEX REPLACE ";" " " CHAOS_SRC "${CHAOS_SRC}")
string(REGEX REPLACE ";" " " ANDROID_APP_ABI "${ANDROID_APP_ABI}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/jni/Android.mk.in
    ${CMAKE_CURRENT_BINARY_DIR}/jni/Android.mk @ONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/jni/Application.mk.in
    ${CMAKE_CURRENT_BINARY_DIR}/jni/Application.mk @ONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/eclipse.project
    ${CMAKE_CURRENT_BINARY_DIR}/.project @ONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/eclipse.cproject
    ${CMAKE_CURRENT_BINARY_DIR}/.cproject @ONLY)

set(symlinks src
    AndroidManifest.xml
    lint.xml
    res/drawable-ldpi
    res/drawable-mdpi
    res/drawable-hdpi
    res/drawable-xhdpi
    res/drawable-xxhdpi
    res/layout
    res/xml
    res/values)

set(symlinktargets "")
foreach(symlink ${symlinks})
    set(t ${CMAKE_CURRENT_BINARY_DIR}/${symlink})
    add_custom_command(OUTPUT ${t}
        COMMAND ${CMAKE_COMMAND}
        ARGS -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/res
        COMMAND ${CMAKE_COMMAND}
        ARGS -E create_symlink
        ${CMAKE_CURRENT_SOURCE_DIR}/${symlink} ${t})
    set(symlinktargets ${symlinktargets} ${t})
endforeach()
add_custom_target(chaos_sym_links DEPENDS ${symlinktargets})

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/build.xml
    COMMAND ${ANDROID_TOOL}
    ARGS update project
    -n ${PROJECT_NAME}
    -p ${CMAKE_CURRENT_BINARY_DIR}
    --target ${ANDROID_TARGET})

set(APK ${CMAKE_CURRENT_BINARY_DIR}/bin/${PROJECT_NAME}-${ANDROID_BUILD}.apk)
if(ANDROID_BUILD STREQUAL release)
    set(APK ${CMAKE_CURRENT_BINARY_DIR}/bin/${PROJECT_NAME}.apk)
endif()

add_custom_command(OUTPUT ${APK}
    COMMAND ${ANT} ${ANDROID_BUILD}
    DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/build.xml
    oggify chaos_ndk_build)


add_custom_target(chaos_apk ALL DEPENDS ${APK})

if(NDKBUILD_VERBOSE)
    set(NDKBUILD_VERBOSE_VAL 1)
else()
    set(NDKBUILD_VERBOSE_VAL 0)
endif()

add_custom_target(chaos_ndk_build ALL
    ${NDKBUILD} V=${NDKBUILD_VERBOSE_VAL}
    DEPENDS chaos_sym_links genspells ${CMAKE_CURRENT_BINARY_DIR}/build.xml)

add_custom_target(installdevice
    COMMAND ${ANT} ${ANDROID_BUILD} install
    DEPENDS chaos_ndk_build)
