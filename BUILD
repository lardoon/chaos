I have only tested the compilation thoroughly on Linux.

To compile you will need at least the following:

 Python          www.python.org
 CMake           www.cmake.org
 devkitpro       www.devkitpro.org
 gcc             gcc.gnu.org

For the Android build, you will also need the Android SDK *and* NDK. The NDK is
a separate download, the SDK uses a GUI to download the target platforms. Use
the latest version of both.

 Android         developer.android.com/sdk

The following variables can be used to control the build:

    ANDROID_NDK: Android NDK path
    ANDROID_SDK: Android SDK path
    DEVKITARM: devkitARM path
    DEVKITPRO: devkitpro path for libgba, libnds
    CC: The C Compiler

Use cases:

The build uses autotools, and the configure step should be run in a separate
build directory. This makes life easier to compile a native version and an
embedded platform version too.

Configure for Android with the NDK and SDK in your home directory, you need to
put grit and raw2c in your path too. These are in the devkitARM bundle:

  mkdir build-android && cd build-android
  env PATH=$DEVKITARM/bin cmake \
    -DANDROID_SDK=~/android-sdk-linux \
    -DANDROID_NDK=~/android-ndk \
    -DANDROID=1 ..

Configure GBA, with devkitARM and libgba installed in ~/tools:

  mkdir build-gba && cd build-gba
  env DEVKITARM=~/tools/devkitARM \
  DEVKITPRO=~/tools \
  cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/GBA.cmake ..

For the Nintendo DS, like the GBA except use its toolchain file:

  mkdir build-nds && cd build-nds
  env DEVKITARM=~/tools/devkitARM \
  DEVKITPRO=~/tools \
  cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/NDS.cmake ..

Configure for Linux only:

  mkdir build-linux && cd build-linux
  cmake ..

After that, in each case run "make" to compile the code.

If you want to port the game to a new platform, see the PORTING file.
