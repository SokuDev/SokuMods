cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)
project(swrstoys C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/install")

set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" AND "x${CMAKE_CXX_SIMULATE_ID}" STREQUAL "xMSVC")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++11-narrowing -Wno-microsoft-cast")
endif ()

add_library(swrstoys MODULE
        swrstoys/d3d9.def
        swrstoys/d3d9_dummy.cpp
        swrstoys/dummy.cpp
        swrstoys/SWRSToys.cpp)
target_compile_definitions(swrstoys PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(swrstoys PRIVATE include)
target_link_libraries(swrstoys shlwapi user32 advapi32)
set_target_properties(swrstoys PROPERTIES
        OUTPUT_NAME d3d9)
install(TARGETS swrstoys LIBRARY DESTINATION ".")

# detours

add_library(detours
        detours/src/creatwth.cpp
        detours/src/detours.cpp
        detours/src/disasm.cpp
        detours/src/disolarm.cpp
        detours/src/disolarm64.cpp
        detours/src/disolia64.cpp
        detours/src/disolx86.cpp
        detours/src/disolx64.cpp
        detours/src/image.cpp
        detours/src/modules.cpp)
target_include_directories(detours PUBLIC detours/src)

# MemoryModule

add_library(MemoryModule
        MemoryModule/MemoryModule.c)
target_include_directories(MemoryModule PUBLIC MemoryModule)

# modules

function(module name)
    add_library("${name}" MODULE)
    target_compile_definitions("${name}" PRIVATE DIRECTINPUT_VERSION=0x0800 _CRT_SECURE_NO_WARNINGS)
    target_include_directories("${name}" PRIVATE include include/directx "modules/${name}/")
    target_link_directories("${name}" PRIVATE lib)
    target_link_libraries("${name}" shlwapi user32 advapi32)
    install(TARGETS "${name}" LIBRARY DESTINATION "modules/${name}/")
    set(MODULE_PATHS "${MODULE_PATHS}; ${name}=modules/${name}/${name}.dll\n" PARENT_SCOPE)
endfunction()

module(BGMChanger)
target_sources(BGMChanger PRIVATE modules/BGMChanger/BGMChanger.cpp)

module(CharactersInForeground)
target_sources(CharactersInForeground PRIVATE modules/CharactersInForeground/CharactersInForeground.cpp)

module(DPadFix)
target_sources(DPadFix PRIVATE modules/DPadFix/DPadFix.cpp)
target_link_libraries(DPadFix dinput8 detours)

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

module(SWRSokuRoll)
target_sources(SWRSokuRoll PRIVATE modules/SWRSokuRoll/SWRSokuRoll.cpp)
target_link_libraries(SWRSokuRoll MemoryModule)

module(UPnPNat)
target_sources(UPnPNat PRIVATE modules/UPnPNat/UPnPNat.cpp)
target_link_libraries(UPnPNat iphlpapi ole32 oleaut32)

module(WindowedFullscreen)
target_sources(WindowedFullscreen PRIVATE
        modules/WindowedFullscreen/WindowedFullscreen.cpp)
# passing "d3d9" conflicts with the generated d3d9 module somehow, pass an absolute path to the one in lib/
target_link_libraries(WindowedFullscreen detours "${CMAKE_SOURCE_DIR}/lib/d3d9.lib")

module(WindowResizer)
target_sources(WindowResizer PRIVATE
        modules/WindowResizer/WindowResizer.cpp)
target_link_libraries(WindowResizer detours)

configure_file("swrstoys/SWRSToys.ini.in" "SWRSToys.ini" @ONLY NEWLINE_STYLE CRLF)

install(DIRECTORY modules/ DESTINATION "modules"
        FILES_MATCHING PATTERN "*.ini")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/SWRSToys.ini" DESTINATION ".")
install(FILES README.md DESTINATION "."
        RENAME "SWRSToys.txt")
