set(CMAKE_LEGACY_CYGWIN_WIN32 0)
cmake_minimum_required(VERSION 2.6)
project(websocketservice)

enable_testing()

set(WARNINGS)
set(ANDROID_STL_FLAGS)

# Platform (not compiler) specific settings
if(IOS)
  set(IOS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/../Build_iOS")
  set(Boost_FRAMEWORK "-F ${IOS_SOURCE_DIR} -framework boost")
  set(Boost_INCLUDE_DIR "${IOS_SOURCE_DIR}/boost.framework/Headers")

  set(OPENSSL_FOUND 1)
  set(OPENSSL_INCLUDE_DIR "${IOS_SOURCE_DIR}/openssl/include")
  set(OPENSSL_LIBRARIES
    "${IOS_SOURCE_DIR}/openssl/lib/libcrypto.a"
    "${IOS_SOURCE_DIR}/openssl/lib/libssl.a"
    )

  # The cxx_flags must be reset here, because the ios-cmake toolchain file unfortunately sets "-headerpad_max_install_names" which is not a valid clang flag.
  set(CMAKE_CXX_FLAGS "-fvisibility=hidden -fvisibility-inlines-hidden")
elseif(ANDROID)
  set(Boost_COMPILER "-clang")
  set(Boost_USE_STATIC_LIBS ON)
  if(ARM)
    set(BOOST_ROOT "${CMAKE_BINARY_DIR}/../Boost-for-Android/build")
    set(BOOST_LIBRARYDIR "${CMAKE_BINARY_DIR}/../Boost-for-Android/build/lib")
  else()
    set(BOOST_ROOT "${CMAKE_BINARY_DIR}/../Boost-for-Android-x86/build")
    set(BOOST_LIBRARYDIR "${CMAKE_BINARY_DIR}/../Boost-for-Android-x86/build/lib")
  endif()
  find_host_package(Boost 1.55 EXACT REQUIRED COMPONENTS random system thread locale filesystem chrono atomic)

  set(OPENSSL_FOUND 1)
  if(ARM)
    set(OPENSSL_INCLUDE_DIR "${CMAKE_BINARY_DIR}/../openssl/armeabi-v7a/include")
    set(OPENSSL_LIBRARIES
      "${CMAKE_BINARY_DIR}/../openssl/armeabi-v7a/lib/libssl.a"
      "${CMAKE_BINARY_DIR}/../openssl/armeabi-v7a/lib/libcrypto.a"
      )
  else()
    set(OPENSSL_INCLUDE_DIR "${CMAKE_BINARY_DIR}/../openssl/x86/include")
    set(OPENSSL_LIBRARIES
      "${CMAKE_BINARY_DIR}/../openssl/x86/lib/libssl.a"
      "${CMAKE_BINARY_DIR}/../openssl/x86/lib/libcrypto.a"
      )
  endif()

  if(ARM)
    set(LIBCXX_STL "${ANDROID_NDK}/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi-v7a/thumb/libgnustl_static.a")
  else()
    set(LIBCXX_STL "${ANDROID_NDK}/sources/cxx-stl/gnu-libstdc++/4.8/libs/x86/thumb/libgnustl_static.a")
  endif()
  # These are used in the shared library case
  set(ANDROID_STL_FLAGS
    ${LIBCXX_STL}
    atomic
    dl
    gcc
    c
    m
    -nodefaultlibs
    )

elseif(UNIX) # This includes OSX
  find_package(Boost REQUIRED COMPONENTS chrono date_time system thread filesystem log log_setup)
  find_package(Threads REQUIRED)
  if(APPLE AND NOT OPENSSL_ROOT_DIR)
    # Prefer a homebrew version of OpenSSL over the one in /usr/lib
    file(GLOB OPENSSL_ROOT_DIR /usr/local/Cellar/openssl/*)
    # Prefer the latest (make the latest one first)
    list(REVERSE OPENSSL_ROOT_DIR)
  endif()
  # This should prevent linking against the system provided 0.9.8y
  find_package(OpenSSL REQUIRED)
  find_library(Casablanca_LIBRARY NAMES cpprest)

elseif(WIN32)
  option(BUILD_SHARED_LIBS "Build shared Libraries." ON)
  option(BUILD_TESTS "Build tests." ON)
  option(BUILD_SAMPLES "Build samples." ON)

  add_definitions(-DUNICODE)

  if(NOT BUILD_SHARED_LIBS)
    # This causes cmake to not link the test libraries separately, but instead hold onto their object files.
    set(TEST_LIBRARY_TARGET_TYPE OBJECT)
  endif()
else()
  message(FATAL_ERROR "-- Unsupported Build Platform.")
endif()

# Compiler (not platform) specific settings
if(ANDROID)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-strict-aliasing")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-pedantic")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-attributes -Wno-pointer-arith")
  include_directories(
    "${ANDROID_NDK}/sources/cxx-stl/gnu-libstdc++/4.8/include"
    "${ANDROID_NDK}/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi-v7a/include"
    "${ANDROID_NDK}/sources/cxx-stl/gnu-libstdc++/4.8/include/backward"
    )
elseif(("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang") OR IOS)
  message("-- Setting clang options")

  set(WARNINGS "-Wall -Wextra -Wcast-qual -Wconversion -Wformat=2 -Winit-self -Winvalid-pch -Wmissing-format-attribute -Wmissing-include-dirs -Wpacked -Wredundant-decls")
  set(OSX_SUPPRESSIONS "-Wno-overloaded-virtual -Wno-sign-conversion -Wno-deprecated -Wno-unknown-pragmas -Wno-reorder -Wno-char-subscripts -Wno-switch -Wno-unused-parameter -Wno-unused-variable -Wno-deprecated -Wno-unused-value -Wno-unknown-warning-option -Wno-return-type-c-linkage -Wno-unused-function -Wno-sign-compare -Wno-shorten-64-to-32 -Wno-reorder")
  set(WARNINGS "${WARNINGS} ${OSX_SUPPRESSIONS}")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wno-return-type-c-linkage -Wno-unneeded-internal-declaration")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-strict-aliasing")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  message("-- Setting gcc options")

  set(WARNINGS "-Wall -Wextra -Wunused-parameter -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winit-self -Winvalid-pch -Wmissing-format-attribute -Wmissing-include-dirs -Wpacked -Wredundant-decls -Wunreachable-code")
  set(LINUX_SUPPRESSIONS "-Wno-deprecated -Wno-unknown-pragmas -Wno-reorder -Wno-unused-function -Wno-char-subscripts -Wno-switch -Wno-unused-but-set-parameter -Wno-unused-value -Wno-unused-local-typedefs")

  set(WARNINGS "${WARNINGS} ${LINUX_SUPPRESSIONS}")
  set(LD_FLAGS "${LD_FLAGS} -Wl,-z,defs")

#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-strict-aliasing")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing")
else()
  message("-- Unknown compiler, success is doubtful.")
endif()

add_definitions(-DBOOST_LOG_DYN_LINK)

# Reconfigure final output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/Binaries)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/Binaries)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/Binaries)

set(Casablanca_LIBRARY ${LIB}cpprest)

# Everything in the project needs access to the casablanca include directories
set(Common_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${Common_INCLUDE_DIR})

add_subdirectory(src)
add_subdirectory(tests)