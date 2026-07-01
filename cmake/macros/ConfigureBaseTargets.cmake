# This file is part of the SyphrenaCore Project. See AUTHORS file for Copyright information
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

# An interface library to make the target com available to other targets
add_library(syphrena-compile-option-interface INTERFACE)

# Use -std=c++11 instead of -std=gnu++11
set(CMAKE_CXX_EXTENSIONS OFF)

# An interface library to make the target features available to other targets
add_library(syphrena-feature-interface INTERFACE)

target_compile_features(syphrena-feature-interface
  INTERFACE
    cxx_std_20)

# An interface library to make the warnings level available to other targets
# This interface taget is set-up through the platform specific script
add_library(syphrena-warning-interface INTERFACE)

# An interface used for all other interfaces
add_library(syphrena-default-interface INTERFACE)
target_link_libraries(syphrena-default-interface
  INTERFACE
    syphrena-compile-option-interface
    syphrena-feature-interface)

# An interface used for silencing all warnings
add_library(syphrena-no-warning-interface INTERFACE)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_compile_options(syphrena-no-warning-interface
    INTERFACE
      /W0)
else()
  target_compile_options(syphrena-no-warning-interface
    INTERFACE
      -w)
endif()

# An interface library to change the default behaviour
# to hide symbols automatically.
add_library(syphrena-hidden-symbols-interface INTERFACE)

# An interface amalgamation which provides the flags and definitions
# used by the dependency targets.
add_library(syphrena-dependency-interface INTERFACE)
target_link_libraries(syphrena-dependency-interface
  INTERFACE
    syphrena-default-interface
    syphrena-no-warning-interface
    syphrena-hidden-symbols-interface)

# An interface amalgamation which provides the flags and definitions
# used by the core targets.
add_library(syphrena-core-interface INTERFACE)
target_link_libraries(syphrena-core-interface
  INTERFACE
    syphrena-default-interface
    syphrena-warning-interface)
