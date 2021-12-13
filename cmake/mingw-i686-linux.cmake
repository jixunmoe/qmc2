# Derived from: http://www.cmake.org/Wiki/CMake_Cross_Compiling

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

SET(CMAKE_CROSSCOMPILING 1)

# CMake's feature test does not work...
set(CMAKE_C_ABI_COMPILED 1)
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_ABI_COMPILED 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

# which compilers to use for C and C++
# sudo apt install wine {gcc,g++}-mingw-w64-i686
SET(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
SET(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH  /usr/x86_64-w64-mingw32 )

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
