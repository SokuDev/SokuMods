cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)
project(swrstoys C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/install")

set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" AND "x${CMAKE_CXX_SIMULATE_ID}" STREQUAL "xMSVC")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++11-narrowing")
endif ()

add_library(swrstoys MODULE
        swrstoys/d3d9.def
        swrstoys/d3d9_dummy.cpp
        swrstoys/dummy.cpp
        swrstoys/SWRSToys.cpp)
target_link_libraries(swrstoys shlwapi user32 advapi32)
set_target_properties(swrstoys PROPERTIES
        OUTPUT_NAME d3d9)
install(TARGETS swrstoys LIBRARY DESTINATION ".")

# modules

function(module name)
    add_library("${name}" MODULE)
    target_include_directories("${name}" PRIVATE include include/directx "modules/${name}/")
    target_link_libraries("${name}" shlwapi user32 advapi32)
    install(TARGETS "${name}" LIBRARY DESTINATION "modules/${name}/")
    set(MODULE_PATHS "${MODULE_PATHS}; ${name}=modules/${name}/${name}.dll\n" PARENT_SCOPE)
endfunction()

module(BGMChanger)
target_sources(BGMChanger PRIVATE modules/BGMChanger/BGMChanger.cpp)

module(MemoryPatch)
target_sources(MemoryPatch PRIVATE modules/MemoryPatch/MemoryPatch.cpp)

module(NetBattleCounter)
target_sources(NetBattleCounter PRIVATE modules/NetBattleCounter/NetBattleCounter.cpp)
target_link_libraries(NetBattleCounter winmm)

module(NetBellChanger)
target_sources(NetBellChanger PRIVATE modules/NetBellChanger/NetBellChanger.cpp)
target_link_libraries(NetBellChanger winmm)

module(NetProfileView)
target_sources(NetProfileView PRIVATE modules/NetProfileView/NetProfileView.cpp)

module(ReplayDnD)
target_sources(ReplayDnD PRIVATE modules/ReplayDnD/ReplayDnD.cpp)

module(ReplayInputView)
target_sources(ReplayInputView PRIVATE
        modules/ReplayInputView/ReplayInputView.cpp
        modules/ReplayInputView/ReplayInputView.rc)

module(ReplayInputView+)
target_sources(ReplayInputView+ PRIVATE
        modules/ReplayInputView+/ReplayInputView.cpp
        modules/ReplayInputView+/Text.cpp
        modules/ReplayInputView+/ReplayInputView.rc)

module(WindowResizer)
target_sources(WindowResizer PRIVATE
        modules/WindowResizer/APIHook.cpp
        modules/WindowResizer/WindowResizer.cpp)

configure_file("swrstoys/SWRSToys.ini.in" "SWRSToys.ini" @ONLY NEWLINE_STYLE CRLF)

install(DIRECTORY modules/ DESTINATION "modules"
        FILES_MATCHING PATTERN "*.ini")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/SWRSToys.ini" DESTINATION ".")
install(FILES README.md DESTINATION "."
        RENAME "SWRSToys.txt")
