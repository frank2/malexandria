cmake_minimum_required(VERSION 3.22)

# intercept the install macro so we only install our own targets
if(NOT DEFINED _DISABLE_INSTALLATION)
  set(_DISABLE_INSTALLATION FALSE)

  macro(install)
    if (NOT _DISABLE_INSTALLATION)
      _install(${ARGN})
    endif()
  endmacro()
endif()

project(malexandria CXX)

set(CMAKE_POLICY_DEFAULT_CMP0074 NEW)

set(_DISABLE_INSTALLATION True)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if (WIN32)
  # libssh has problems linking on Windows without this
  set(CMAKE_SHARED_LIBRARY_PREFIX "lib")
  set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
endif()

file(GLOB_RECURSE HEADER_FILES FOLLOW_SYMLINKS ${PROJECT_SOURCE_DIR}/src/*.h ${PROJECT_SOURCE_DIR}/src/*.hpp)
file(GLOB_RECURSE SRC_FILES FOLLOW_SYMLINKS ${PROJECT_SOURCE_DIR}/src/*.c ${PROJECT_SOURCE_DIR}/src/*.cpp)

source_group(TREE "${PROJECT_SOURCE_DIR}" PREFIX "Header Files" FILES ${HEADER_FILES})
source_group(TREE "${PROJECT_SOURCE_DIR}" PREFIX "Source Files" FILES ${SRC_FILES})

set(OPENSSL_USE_STATIC_LIBS TRUE CACHE BOOL "Use static libs for OpenSSL" FORCE)
set(OPENSSL_MSVC_STATIC_RT TRUE CACHE BOOL "Use static libs for MSVC" FORCE)
#set(CMAKE_FIND_DEBUG_MODE TRUE)
find_package(OpenSSL 1.1.1 REQUIRED)

set(ARGPARSE_INSTALL OFF CACHE BOOL "Do not install argparse." FORCE)
set(_DISABLE_INSTALLATION False)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/argparse)
set(_DISABLE_INSTALLATION True)

set(CRYPTOPP_SOURCES "${PROJECT_SOURCE_DIR}/lib/cryptopp" CACHE STRING "The crypto++ sources" FORCE)
set(CRYPTOPP_INSTALL OFF CACHE BOOL "Don't create the install target for crypto++" FORCE)
set(CRYPTOPP_BUILD_TESTING OFF CACHE BOOL "Don't build tests for crypto++" FORCE)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/cryptopp-cmake)
add_compile_definitions(CRYPTOPP_ENABLE_NAMESPACE_WEAK=1)

add_subdirectory(${PROJECT_SOURCE_DIR}/lib/json)

# trick find_package resolution for libzip
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/zlib)
add_library(ZLIB::ZLIB ALIAS zlibstatic)
set(ZLIB_ROOT "${CMAKE_BINARY_DIR}/lib/zlib")
set(ZLIB_LIBRARY "local")
set(ZLIB_INCLUDE_DIR "${CMAKE_BINARY_DIR}/lib/zlib")
include_directories(${PROJECT_SOURCE_DIR}/lib/zlib)
include_directories(${CMAKE_BINARY_DIR}/lib/zlib)

set(ZLIB_USE_STATIC_LIBS ON CACHE BOOL "Look for static ZLib libraries" FORCE)
set(LIBZIP_DO_INSTALL OFF CACHE BOOL "Do not perform the install step for libzip" FORCE)
set(BUILD_TOOLS OFF CACHE BOOL "Do not build zip tools" FORCE)
set(BUILD_REGRESS OFF CACHE BOOL "Do not build regression tests" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Do not build libzip examples" FORCE)
set(BUILD_DOC OFF CACHE BOOL "Do not build libzip docs" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Do not build shared libs" FORCE)
cmake_policy(SET CMP0074 NEW)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/libzip)

cmake_policy(SET CMP0074 NEW)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/sqlite3-cmake)
include_directories(${PROJECT_SOURCE_DIR}/lib/sqlite3-cmake/src)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Do not build shared libs" FORCE)
set(WITH_EXAMPLES OFF CACHE BOOL "Do not compile the examples." FORCE)
set(WITH_ZLIB OFF CACHE BOOL "Disable zlib support for ssh." FORCE)
cmake_policy(SET CMP0074 NEW)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/libssh)

set(FMT_INSTALL OFF CACHE BOOL "Do not create an install target for libfmt" FORCE)
set(FMT_TEST OFF CACHE BOOL "Do not generate tests for libfmt" FORCE)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/libfmt)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/lib/stduuid/cmake")
set(UUID_SYSTEM_GENERATOR ON CACHE BOOL "Use the system UUID generator" FORCE)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/stduuid)

set(HTTP_ONLY ON CACHE BOOL "Disable everything except HTTP in curl" FORCE)
set(BUILD_CURL_EXE OFF CACHE BOOL "Do not build the curl executable" FORCE)
set(CURL_ENABLE_EXPORT_TARGET OFF CACHE BOOL "Do not enable export targets for libcurl" FORCE)

if (WIN32)
  set(CURL_USE_SCHANNEL ON CACHE BOOL "Enable native TLS/SSL on Windows" FORCE)
else ()
  set(CURL_USE_OPENSSL ON CACHE BOOL "Enable OpenSSL on Unix platforms" FORCE)
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/lib/curl)

set(MALEXANDRIA_VERSION_MAJOR 1)
set(MALEXANDRIA_VERSION_MINOR 0)
set(MALEXANDRIA_VERSION_REVISION 0)
set(MALEXANDRIA_VERSION "${MALEXANDRIA_VERSION_MAJOR}.${MALEXANDRIA_VERSION_MINOR}.${MALEXANDRIA_VERSION_REVISION}")
add_compile_definitions(MALEXANDRIA_VERSION_MAJOR=${MALEXANDRIA_VERSION_MAJOR})
add_compile_definitions(MALEXANDRIA_VERSION_MINOR=${MALEXANDRIA_VERSION_MINOR})
add_compile_definitions(MALEXANDRIA_VERSION_REVISION=${MALEXANDRIA_VERSION_REVISION})
add_compile_definitions(MALEXANDRIA_VERSION="${MALEXANDRIA_VERSION}")

include_directories(${PROJECT_SOURCE_DIR}/src)
add_executable(malexandria ${HEADER_FILES} ${SRC_FILES})
set_target_properties(malexandria PROPERTIES LINKER_LANGUAGE CXX)

if (WIN32)
  target_link_libraries(malexandria PUBLIC argparse cryptopp nlohmann_json ZLIB::ZLIB zip sqlite3 ssh crypt32 fmt-header-only stduuid libcurl)
else()
  target_link_libraries(malexandria PUBLIC argparse cryptopp nlohmann_json ZLIB::ZLIB zip sqlite3 ssh ncurses fmt-header-only stduuid libcurl)
endif()

target_include_directories(malexandria PUBLIC
  "${CMAKE_CURRENT_BINARY_DIR}/lib/libssh/include"
)

set(_DISABLE_INSTALLATION False)
install(TARGETS malexandria DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
