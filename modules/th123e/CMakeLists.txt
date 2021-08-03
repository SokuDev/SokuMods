cmake_minimum_required(VERSION 3.15)
find_package(Git REQUIRED)

macro(add_git_module target uri branch)
    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/${target})
        message(STATUS "Cloning ${target} from ${uri}")
        execute_process(COMMAND ${GIT_EXECUTABLE} clone --branch ${branch} --depth 1
            ${uri} ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/${target} ERROR_QUIET)
        if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/${target})
            message(FATAL_ERROR "Dependency error: Failed to download ${target}")
        endif()
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --depth 1 ${ARGN}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/${target})
    endif()
endmacro()

# zlib Library
add_git_module(zlib git://github.com/madler/zlib.git v1.2.11)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/zlib EXCLUDE_FROM_ALL)
if (MSVC)
    set(ZLIB_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/thirdparty/zlib/Release/zlibstatic.lib)
else()
    set(ZLIB_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/thirdparty/zlib/libzlibstatic.a)
endif()
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/zlib/zlib.h _zlib_h_contents)
string(REGEX REPLACE ".*#define[ \t]+ZLIB_VERSION[ \t]+\"([-0-9A-Za-z.]+)\".*" "\\1" ZLIB_VERSION_STRING ${_zlib_h_contents})
find_package(ZLIB REQUIRED)
#set_target_properties(ZLIB::ZLIB PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${ZLIB_INCLUDE_DIRS};${CMAKE_CURRENT_BINARY_DIR}/thirdparty/zlib")
#add_dependencies(ZLIB::ZLIB zlibstatic)

# libpng Library
add_git_module(libpng git://git.code.sf.net/p/libpng/code libpng12)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libpng EXCLUDE_FROM_ALL)
target_include_directories(png12_static PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/thirdparty/zlib)

set(TH123E_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/zlib
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libpng
    ${CMAKE_CURRENT_BINARY_DIR}/thirdparty/zlib 
    ${CMAKE_CURRENT_BINARY_DIR}/thirdparty/libpng)

add_library(th123e MODULE)
target_sources(th123e PRIVATE th123edll.cpp ioapi.c unzip.c)
target_link_libraries(th123e zlibstatic png12_static shlwapi)
target_include_directories(th123e PUBLIC ${TH123E_INCLUDE_DIRS})

add_library(th123w MODULE)
target_sources(th123w PRIVATE th123wdll.cpp ioapi.c unzip.c)
target_link_libraries(th123w zlibstatic png12_static shlwapi)
target_include_directories(th123w PUBLIC ${TH123E_INCLUDE_DIRS})

#install(TARGETS th123e th123w LIBRARY DESTINATION "SWRSToys/modules/th123e/")
