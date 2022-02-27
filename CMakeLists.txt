cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
cmake_policy(SET CMP0091 NEW)
project(swrstoys C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/install")

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_definitions(-DWINVER=0x0501 -D_WIN32_WINNT=0x0501)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND "${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++11-narrowing -Wno-microsoft-cast")
    # libcurl uses a copyright symbol escape in its file description string for its resource control file
    # work around this undefined behaviour by setting a global codepage for all RC files
    SET(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} /C 1252")
endif ()
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /Brepro")
SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /Brepro")
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /Brepro")

add_library(swrstoys MODULE
        swrstoys/d3d9.def
        swrstoys/d3d9_dummy.cpp
        swrstoys/dummy.cpp
        swrstoys/SWRSToys.cpp)
target_compile_definitions(swrstoys PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(swrstoys PRIVATE include)
target_link_libraries(swrstoys shlwapi user32 advapi32 shell32)
set_target_properties(swrstoys PROPERTIES
        OUTPUT_NAME d3d9)
install(TARGETS swrstoys LIBRARY DESTINATION "SWRSToys/")
install(TARGETS swrstoys LIBRARY DESTINATION "SokuEngine/")

# mbedTLS

set(ENABLE_PROGRAMS OFF)
set(INSTALL_MBEDTLS_HEADERS OFF)
set(MBEDTLS_FATAL_WARNINGS OFF)
add_subdirectory(mbedtls EXCLUDE_FROM_ALL)

add_library(mbedtls_shim INTERFACE)
target_link_libraries(mbedtls_shim INTERFACE mbedtls mbedx509 mbedcrypto)

# workaround for curl: link against our local mbedtls rather than search for it
function(find_package)
    if(ARGV0 STREQUAL "MbedTLS")
        set(MBEDTLS_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/mbedtls/include" CACHE STRING "")
        set(MBEDTLS_LIBRARIES "mbedtls_shim" CACHE STRING "")
        mark_as_advanced(MBEDTLS_INCLUDE_DIRS MBEDTLS_LIBRARIES)
    else()
        _find_package(${ARGV})
    endif()
endfunction()

# workaround for curl: don't try installing any export files
function(install)
    if(ARGV0 STREQUAL "EXPORT")
    else()
        _install(${ARGV})
    endif()
endfunction()

# cURL

set(BUILD_CURL_EXE OFF)
set(BUILD_SHARED_LIBS OFF)
set(BUILD_TESTING OFF)
set(CURL_STATIC_CRT ON)
set(ENABLE_INET_PTON OFF)
set(HTTP_ONLY ON)
set(CURL_DISABLE_CRYPTO_AUTH ON)
set(ENABLE_MANUAL OFF)
set(CMAKE_USE_OPENSSL OFF)
set(CMAKE_USE_MBEDTLS ON)
set(CURL_DISABLE_LDAP ON)
set(CURL_ZLIB OFF)
set(CMAKE_USE_LIBSSH2 OFF)
set(ENABLE_UNIX_SOCKETS OFF)
set(CURL_CA_BUNDLE none)
set(CURL_CA_PATH none)
set(CURL_WINDOWS_SSPI OFF)
add_subdirectory(curl EXCLUDE_FROM_ALL)

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

# DiscordSDK

add_library(DiscordSDK
        DiscordSDK/achievement_manager.cpp
        DiscordSDK/activity_manager.cpp
        DiscordSDK/application_manager.cpp
        DiscordSDK/core.cpp
        DiscordSDK/image_manager.cpp
        DiscordSDK/lobby_manager.cpp
        DiscordSDK/network_manager.cpp
        DiscordSDK/overlay_manager.cpp
        DiscordSDK/relationship_manager.cpp
        DiscordSDK/storage_manager.cpp
        DiscordSDK/store_manager.cpp
        DiscordSDK/types.cpp
        DiscordSDK/user_manager.cpp
        DiscordSDK/voice_manager.cpp
)
target_include_directories(DiscordSDK INTERFACE DiscordSDK)
target_link_libraries(DiscordSDK INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lib/discord_game_sdk.dll.lib)

# imgui

add_library(imgui
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        imgui/backends/imgui_impl_dx9.cpp
        imgui/backends/imgui_impl_win32.cpp)
target_include_directories(imgui PUBLIC imgui/ imgui/backends/)

# implot

add_library(implot
        implot/implot.cpp
        implot/implot_items.cpp)
target_include_directories(implot PUBLIC implot/)
target_link_libraries(implot PUBLIC imgui)

# lua

set(LUA_SOURCES
        lua/lapi.c
        lua/lauxlib.c
        lua/lbaselib.c
        lua/lcode.c
        lua/lcorolib.c
        lua/lctype.c
        lua/ldblib.c
        lua/ldebug.c
        lua/ldo.c
        lua/ldump.c
        lua/lfunc.c
        lua/lgc.c
        lua/linit.c
        lua/liolib.c
        lua/llex.c
        lua/lmathlib.c
        lua/lmem.c
        lua/loadlib.c
        lua/lobject.c
        lua/lopcodes.c
        lua/loslib.c
        lua/lparser.c
        lua/lstate.c
        lua/lstring.c
        lua/lstrlib.c
        lua/ltable.c
        lua/ltablib.c
        lua/ltm.c
        lua/lundump.c
        lua/lutf8lib.c
        lua/lvm.c
        lua/lzio.c)
add_library(lua ${LUA_SOURCES})
target_include_directories(lua PUBLIC lua)

# MemoryModule

add_library(MemoryModule
        MemoryModule/MemoryModule.c)
target_include_directories(MemoryModule PUBLIC MemoryModule)

# png

add_library(png
        png/png.c
        png/pngerror.c
        png/pngget.c
        png/pngmem.c
        png/pngpread.c
        png/pngread.c
        png/pngrio.c
        png/pngrtran.c
        png/pngrutil.c
        png/pngset.c
        png/pngtrans.c
        png/pngwio.c
        png/pngwrite.c
        png/pngwtran.c
        png/pngwutil.c)
target_link_libraries(png PUBLIC zlib)
target_include_directories(png PUBLIC png/)

# SokuLib
add_subdirectory(SokuLib)

# SokuEngineLib

add_library(SokuEngineLib STATIC IMPORTED)
set_target_properties(SokuEngineLib PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/SokuEngineLib.lib)
set_target_properties(SokuEngineLib PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include/)

# zlib

configure_file(zlib/zconf.h.cmakein
        ${CMAKE_CURRENT_BINARY_DIR}/zlib/zconf.h @ONLY)
add_library(zlib
        zlib/adler32.c
        zlib/compress.c
        zlib/crc32.c
        zlib/deflate.c
        zlib/gzclose.c
        zlib/gzlib.c
        zlib/gzread.c
        zlib/gzwrite.c
        zlib/inflate.c
        zlib/infback.c
        zlib/inftrees.c
        zlib/inffast.c
        zlib/trees.c
        zlib/uncompr.c
        zlib/zutil.c)
target_include_directories(zlib PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/zlib/ zlib/)

# modules

function(module name)
    set(SWRSToys 1)
    set(Default 0)
    set(Target 1)
    foreach(arg IN LISTS ARGN)
        if(arg STREQUAL "SokuEngine")
            set(SWRSToys 0)
        elseif(arg STREQUAL "Default")
            set(Default 1)
        elseif(arg STREQUAL "Binary")
            set(Target 0)
        endif()
    endforeach()
    if(Target)
        add_library("${name}" MODULE)
        target_compile_definitions("${name}" PRIVATE DIRECTINPUT_VERSION=0x0800 CURL_STATICLIB _CRT_SECURE_NO_WARNINGS $<$<CONFIG:Debug>:_DEBUG>)
        target_include_directories("${name}" PRIVATE include include/directx "modules/${name}/")
        target_link_directories("${name}" PRIVATE lib)
        target_link_libraries("${name}" shlwapi user32 advapi32)
        if(SWRSToys)
            install(TARGETS "${name}" LIBRARY DESTINATION "SWRSToys/modules/${name}/")
        endif()
        install(TARGETS "${name}" LIBRARY DESTINATION "SokuEngine/modules/${name}/")
    else()
        if(SWRSToys)
            install(FILES "bin/${name}.dll" DESTINATION "SWRSToys/modules/${name}")
        endif()
        install(FILES "bin/${name}.dll" DESTINATION "SokuEngine/modules/${name}")
    endif()
    if(SWRSToys)
        install(DIRECTORY "modules/${name}" DESTINATION "SWRSToys/modules"
                FILES_MATCHING
                REGEX "modules/${name}/[^/.]+(/|$)" EXCLUDE
                PATTERN "*.ini")
        if(Default)
            set(SWRSTOYS_MODULE_PATHS "${SWRSTOYS_MODULE_PATHS}${name}=modules/${name}/${name}.dll\r\n" PARENT_SCOPE)
        else()
            set(SWRSTOYS_MODULE_PATHS "${SWRSTOYS_MODULE_PATHS}; ${name}=modules/${name}/${name}.dll\r\n" PARENT_SCOPE)
        endif()
    endif()
    install(DIRECTORY "modules/${name}" DESTINATION "SokuEngine/modules"
            FILES_MATCHING
            REGEX "modules/${name}/[^/.]+(/|$)" EXCLUDE
            PATTERN "*.ini")
    set(SOKUENGINE_MODULE_PATHS "${SOKUENGINE_MODULE_PATHS}${name} = ${Default}\r\n" PARENT_SCOPE)
endfunction()

module(Autopunch Default)
target_sources(Autopunch PRIVATE modules/Autopunch/Autopunch.c)
target_link_libraries(Autopunch CURL::libcurl shlwapi version user32 ws2_32)
set(INSTALL_AUTOPUNCH_DAT false CACHE BOOL "Whether to pre-install the current Autopunch dat")
if(INSTALL_AUTOPUNCH_DAT)
    file(DOWNLOAD "http://autopunch-update.delthas.fr/update" "${CMAKE_BINARY_DIR}/autopunch.dat")
    install(FILES "${CMAKE_BINARY_DIR}/autopunch.dat" DESTINATION "SWRSToys/modules/Autopunch/")
    install(FILES "${CMAKE_BINARY_DIR}/autopunch.dat" DESTINATION "SokuEngine/modules/Autopunch/")
endif()

module(BGMChanger)
target_sources(BGMChanger PRIVATE modules/BGMChanger/BGMChanger.cpp)

set(BUILD_BLACKLIST true CACHE BOOL "Whether to build the Blacklist mod")
if(BUILD_BLACKLIST)
    module(Blacklist)
    target_sources(Blacklist PRIVATE modules/Blacklist/main.cpp)
    target_link_libraries(Blacklist SokuLib shlwapi ws2_32)
endif()

module(CharactersInForeground)
target_sources(CharactersInForeground PRIVATE modules/CharactersInForeground/CharactersInForeground.cpp)

# DiscordIntegration main module
set(DISCORD_CLIENT_ID "771968286199382056" CACHE STRING "DiscordIntegration mod Discord client ID")
module(DiscordIntegration Default)
target_compile_definitions(DiscordIntegration PRIVATE _CRT_SECURE_NO_WARNINGS $<$<CONFIG:Debug>:_DEBUG> ClientID="${DISCORD_CLIENT_ID}" WINVER=0x0601 _WIN32_WINNT=0x0601)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND "${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
    target_compile_options(DiscordIntegration PRIVATE /EHsc)
endif ()
target_link_libraries(DiscordIntegration DiscordSDK SokuLib shlwapi user32 ws2_32)
if (NOT ${CMAKE_BUILD_TYPE} STREQUAL Debug)
set_target_properties(DiscordIntegration PROPERTIES
        SUFFIX ".dat")
endif ()
target_sources(DiscordIntegration PRIVATE
        modules/DiscordIntegration/Network/getPublicIp.cpp
        modules/DiscordIntegration/Network/Socket.cpp
        modules/DiscordIntegration/SokuUtils/SokuState.cpp
        modules/DiscordIntegration/logger.cpp
        modules/DiscordIntegration/main.cpp
        modules/DiscordIntegration/ShiftJISDecoder.cpp
        modules/DiscordIntegration/CompiledString/BasicCompiledString.cpp
        modules/DiscordIntegration/CompiledString/ComplexCompiledString.cpp
        modules/DiscordIntegration/CompiledString/Vars/LeftChrShortNameString.cpp
        modules/DiscordIntegration/CompiledString/Vars/LeftChrFullNameString.cpp
        modules/DiscordIntegration/CompiledString/Vars/LeftChrImgString.cpp
        modules/DiscordIntegration/CompiledString/Vars/LeftProfileNameString.cpp
        modules/DiscordIntegration/CompiledString/Vars/LeftWinsString.cpp
        modules/DiscordIntegration/CompiledString/Vars/RightChrShortNameString.cpp
        modules/DiscordIntegration/CompiledString/Vars/RightChrFullNameString.cpp
        modules/DiscordIntegration/CompiledString/Vars/RightChrImgString.cpp
        modules/DiscordIntegration/CompiledString/Vars/RightProfileNameString.cpp
        modules/DiscordIntegration/CompiledString/Vars/RightWinsString.cpp
        modules/DiscordIntegration/CompiledString/Vars/MyChrShortNameString.cpp
        modules/DiscordIntegration/CompiledString/Vars/MyChrFullNameString.cpp
        modules/DiscordIntegration/CompiledString/Vars/MyChrImgString.cpp
        modules/DiscordIntegration/CompiledString/Vars/MyProfileNameString.cpp
        modules/DiscordIntegration/CompiledString/Vars/MyWrString.cpp
        modules/DiscordIntegration/CompiledString/Vars/OpponentChrShortNameString.cpp
        modules/DiscordIntegration/CompiledString/Vars/OpponentChrFullNameString.cpp
        modules/DiscordIntegration/CompiledString/Vars/OpponentChrImgString.cpp
        modules/DiscordIntegration/CompiledString/Vars/OpponentProfileNameString.cpp
        modules/DiscordIntegration/CompiledString/Vars/OpponentWrString.cpp
        modules/DiscordIntegration/CompiledString/Vars/SubmenuString.cpp
        modules/DiscordIntegration/CompiledString/Vars/StageImgString.cpp
        modules/DiscordIntegration/CompiledString/Vars/StageNameString.cpp
        modules/DiscordIntegration/CompiledString/Vars/TrialName.cpp
        modules/DiscordIntegration/CompiledString/Vars/TrialPack.cpp
        modules/DiscordIntegration/CompiledString/Vars/TrialAttempt.cpp
        modules/DiscordIntegration/CompiledString/Vars/vars.cpp
        modules/DiscordIntegration/CompiledString/CompiledStringFactory.cpp)
install(DIRECTORY "modules/DiscordIntegration/langs" DESTINATION "SWRSToys/modules/DiscordIntegration")
install(DIRECTORY "modules/DiscordIntegration/langs" DESTINATION "SokuEngine/modules/DiscordIntegration")
# DiscordIntegration loader
if (NOT ${CMAKE_BUILD_TYPE} STREQUAL Debug)
add_library(DiscordIntegrationLoader MODULE)
target_sources(DiscordIntegrationLoader PRIVATE
        modules/DiscordIntegration/Loader.c)
target_link_libraries(DiscordIntegrationLoader shlwapi user32 advapi32)
set_target_properties(DiscordIntegrationLoader PROPERTIES
        OUTPUT_NAME "DiscordIntegration")
install(TARGETS DiscordIntegrationLoader LIBRARY DESTINATION "SWRSToys/modules/DiscordIntegration/")
install(TARGETS DiscordIntegrationLoader LIBRARY DESTINATION "SokuEngine/modules/DiscordIntegration/")
endif ()
# DiscordIntegration Discord sdk
install(FILES bin/discord_game_sdk.dll DESTINATION "SWRSToys/modules/DiscordIntegration/")
install(FILES bin/discord_game_sdk.dll DESTINATION "SokuEngine/modules/DiscordIntegration/")

module(DPadFix Default)
target_sources(DPadFix PRIVATE modules/DPadFix/DPadFix.cpp)
target_link_libraries(DPadFix dinput8 detours)

module(EasyDecks Default)
target_sources(EasyDecks PRIVATE
        modules/EasyDecks/EasyDecks.cpp)

module(FixMeiling236 Default)
target_sources(FixMeiling236 PRIVATE
        modules/FixMeiling236/main.cpp)

module(GetIP)
target_sources(GetIP PRIVATE modules/GetIP/main.c)
target_link_libraries(GetIP CURL::libcurl shlwapi user32 ws2_32)

module(HostInBackground)
target_sources(HostInBackground PRIVATE modules/HostInBackground/main.cpp)
target_link_libraries(HostInBackground SokuLib shlwapi user32 ws2_32 d3dx9)

module(InfiniteDecks Default)
target_sources(
        "InfiniteDecks" PRIVATE
        modules/InfiniteDecks/InfiniteDecks.rc
        modules/InfiniteDecks/main.cpp
)
target_link_libraries(
        InfiniteDecks
        SokuLib
        shlwapi
        ws2_32
        user32
        "${CMAKE_SOURCE_DIR}/lib/d3d9.lib"
        "${CMAKE_SOURCE_DIR}/lib/d3dx9.lib"
)

add_executable(
        ProfileConverter
        modules/InfiniteDecks/ProfileConverter.c
)
target_link_libraries(ProfileConverter shlwapi)
install(TARGETS ProfileConverter RUNTIME DESTINATION "SWRSToys/modules/InfiniteDecks")
install(TARGETS ProfileConverter RUNTIME DESTINATION "SokuEngine/modules/InfiniteDecks")

module(InGameHostlist Default)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND "${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
    target_compile_options(InGameHostlist PRIVATE /EHsc)
endif ()
target_sources(InGameHostlist PRIVATE
        modules/InGameHostlist/src/dllmain.cpp
        modules/InGameHostlist/imgui/imgui.cpp
        modules/InGameHostlist/imgui/imgui_draw.cpp
        modules/InGameHostlist/imgui/imgui_impl_dx9.cpp
        modules/InGameHostlist/imgui/imgui_impl_win32.cpp
        modules/InGameHostlist/imgui/imgui_widgets.cpp)
target_compile_definitions(InGameHostlist PRIVATE DIRECTINPUT_VERSION=0x0800 CURL_STATICLIB _CRT_SECURE_NO_WARNINGS $<$<CONFIG:Debug>:_DEBUG> _HAS_STD_BYTE=0)
# force include curl includes first so that we use our own curl rather than the one bundled in InGameHostlist
get_target_property(CURL_EXPORTED_INCLUDE_DIRECTORIES CURL::libcurl INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(InGameHostlist PRIVATE ${CURL_EXPORTED_INCLUDE_DIRECTORIES} modules/InGameHostlist/include include include/directx)
target_link_libraries(InGameHostlist CURL::libcurl d3dx9 "${CMAKE_SOURCE_DIR}/lib/XInput.lib" winmm SokuEngineLib ws2_32 advapi32 shlwapi)

install(FILES modules/InGameHostlist/hostlistBG.png DESTINATION "SWRSToys/modules/InGameHostlist")
install(FILES modules/InGameHostlist/hostlistBG.png DESTINATION "SokuEngine/modules/InGameHostlist")
install(FILES modules/InGameHostlist/romanan.ttf DESTINATION "SWRSToys/modules/InGameHostlist")
install(FILES modules/InGameHostlist/romanan.ttf DESTINATION "SokuEngine/modules/InGameHostlist")
install(FILES modules/InGameHostlist/NewHostSFX.wav DESTINATION "SWRSToys/modules/InGameHostlist")
install(FILES modules/InGameHostlist/NewHostSFX.wav DESTINATION "SokuEngine/modules/InGameHostlist")

module(LabTool)
target_sources(LabTool PRIVATE
        modules/LabTool/dllmain.cpp
        modules/LabTool/functions.cpp
        modules/LabTool/joystick.cpp
        modules/LabTool/LabTool.cpp
        modules/LabTool/labtool_manager.cpp)
target_link_libraries(LabTool dinput8 shlwapi user32)

module(MemoryPatch)
target_sources(MemoryPatch PRIVATE modules/MemoryPatch/MemoryPatch.cpp)

module(NetBattleCounter Default)
target_sources(NetBattleCounter PRIVATE modules/NetBattleCounter/NetBattleCounter.cpp)
target_link_libraries(NetBattleCounter winmm)

module(NetBellChanger)
target_sources(NetBellChanger PRIVATE modules/NetBellChanger/NetBellChanger.cpp)
target_link_libraries(NetBellChanger winmm)

module(NetInfo)
target_sources(NetInfo PRIVATE
        modules/NetInfo/NetInfo.cpp)
target_compile_definitions(NetInfo PRIVATE DIRECTINPUT_VERSION=0x0800)
target_include_directories(NetInfo PRIVATE include include/directx)
target_link_libraries(NetInfo SokuLib imgui implot d3dx9 "${CMAKE_SOURCE_DIR}/lib/XInput.lib")

module(NetProfileView)
target_sources(NetProfileView PRIVATE modules/NetProfileView/NetProfileView.cpp)

module(PaletteTrueColors)
target_sources(PaletteTrueColors PRIVATE modules/PaletteTrueColors/module.cpp)

add_executable(
        PaletteConverter
        modules/PaletteTrueColors/converter.cpp
)
set_target_properties(PaletteConverter PROPERTIES
        OUTPUT_NAME convpal)
install(TARGETS PaletteConverter RUNTIME DESTINATION "SWRSToys/modules/PaletteTrueColors")
install(TARGETS PaletteConverter RUNTIME DESTINATION "SokuEngine/modules/PaletteTrueColors")

module(PunishDisplay)
target_sources(PunishDisplay PRIVATE modules/PunishDisplay/main.cpp)
target_link_libraries(
        PunishDisplay
        SokuLib
        shlwapi
        ws2_32
        user32
        "${CMAKE_SOURCE_DIR}/lib/d3d9.lib"
        "${CMAKE_SOURCE_DIR}/lib/d3dx9.lib"
)

module(ReDelay Default)
target_sources(ReDelay PRIVATE
        modules/ReDelay/ReDelay.cpp)
target_link_libraries(ReDelay SokuLib)

module(ReplayDnD Default)
target_sources(ReplayDnD PRIVATE modules/ReplayDnD/ReplayDnD.cpp)
target_link_libraries(ReplayDnD detours shlwapi shell32)

module(ReplayReSync Default)
target_sources(ReplayReSync PRIVATE
        modules/ReplayReSync/ReplayReSync.cpp)
target_link_libraries(ReplayReSync detours shlwapi shell32)

module(ReplayInputView+)
target_sources(ReplayInputView+ PRIVATE
        modules/ReplayInputView+/ReplayInputView.cpp
        modules/ReplayInputView+/Text.cpp
        modules/ReplayInputView+/ReplayInputView.rc)

module(SkipIntro)
target_sources(SkipIntro PRIVATE
        modules/SkipIntro/Soku-SkipIntro/dllmain.cpp)

module(SokuReplays Default)
target_compile_definitions(SokuReplays PRIVATE PSAPI_VERSION=1)
target_sources(SokuReplays PRIVATE modules/SokuReplays/main.cpp)
target_link_libraries(SokuReplays CURL::libcurl psapi shlwapi user32 ws2_32)

set(BUILD_SOKU_STREAMING true CACHE BOOL "Whether to build the SokuStreaming mod")
if(BUILD_SOKU_STREAMING)
    module(SokuStreaming)
    target_sources(SokuStreaming PRIVATE
            modules/SokuStreaming/Network/Socket.cpp
            modules/SokuStreaming/Network/WebServer.cpp
            modules/SokuStreaming/Network/WebSocket.cpp
            modules/SokuStreaming/Utils/ShiftJISDecoder.cpp
            modules/SokuStreaming/main.cpp
            modules/SokuStreaming/State.cpp
            modules/SokuStreaming/Network/Handlers.cpp
            modules/SokuStreaming/Utils/InputBox.cpp
            )
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND "${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
        target_compile_options(SokuStreaming PRIVATE /EHsc)
    endif ()
    target_link_libraries(SokuStreaming SokuLib shlwapi ws2_32 user32 advapi32 crypt32)
    install(DIRECTORY "modules/SokuStreaming/static" DESTINATION "SWRSToys/modules/SokuStreaming")
    install(DIRECTORY "modules/SokuStreaming/static" DESTINATION "SokuEngine/modules/SokuStreaming")
endif()

module(SWRSokuRoll Default)
target_sources(SWRSokuRoll PRIVATE modules/SWRSokuRoll/SWRSokuRoll.cpp)
file(READ "${CMAKE_SOURCE_DIR}/bin/SokuRoll.dll" SokuRollData HEX)
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," SokuRollData ${SokuRollData})
file(WRITE "${CMAKE_BINARY_DIR}/SokuRoll.inl" "const unsigned char sokuroll_data[] = {${SokuRollData}};\nconst unsigned sokuroll_size = sizeof(sokuroll_data);\n")
target_include_directories(SWRSokuRoll PRIVATE "${CMAKE_BINARY_DIR}")
target_link_libraries(SWRSokuRoll MemoryModule)

module(th123e)
target_sources(th123e PRIVATE
        modules/th123e/th123edll.cpp
        modules/th123e/unzip.c
        modules/th123e/ioapi.c)
if(MSVC)
    # th123e does some black magic that calls functions that break ESP.
    # this makes msvc debug checks fail and makes th123 crash when in debug.
    # -> force these checks to be disabled by activating a dumb optimization.
    # /RTC- doesnt work, neither does #pragma runtime_checks, so this is the best we can do.
    target_compile_options(th123e PRIVATE $<$<CONFIG:Debug>:/Os>)
endif()
target_link_libraries(th123e png zlib shlwapi gdi32)

add_custom_target(th123e_zip COMMAND
        ${CMAKE_COMMAND} -E tar cf "${CMAKE_CURRENT_BINARY_DIR}/th123e.zip" --format=zip -- .
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/modules/th123e/resources)
add_dependencies(th123e th123e_zip)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/th123e.zip DESTINATION "SWRSToys/modules/th123e")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/th123e.zip DESTINATION "SokuEngine/modules/th123e")

module(TrialMode)
target_sources(TrialMode PRIVATE
        modules/TrialMode/TrialMode.rc
        modules/TrialMode/main.cpp
        modules/TrialMode/gif_load.h
        modules/TrialMode/Menu.cpp
        modules/TrialMode/Pack.cpp
        modules/TrialMode/Trial/Trial.cpp
        modules/TrialMode/Trial/ComboTrial.cpp
        modules/TrialMode/TrialEditor/TrialEditor.cpp
        modules/TrialMode/TrialEditor/ComboTrialEditor.cpp
        modules/TrialMode/Actions.cpp
        modules/TrialMode/Animations/BattleAnimation.cpp
        modules/TrialMode/Animations/BattleAnimFramework.cpp
        modules/TrialMode/Animations/LuaBattleAnimation.cpp
        modules/TrialMode/Images.cpp
        modules/TrialMode/Animations/FakeChrMgr.cpp
        modules/TrialMode/Patches.cpp
        modules/TrialMode/TrialBase.cpp
        modules/TrialMode/PackOutro.cpp
        modules/TrialMode/InputBox.cpp
        modules/TrialMode/Explorer.cpp
)
target_compile_options(TrialMode PRIVATE /bigobj)
target_link_libraries(TrialMode
        SokuLib
        shlwapi
        ws2_32
        user32
        lua
        "${CMAKE_SOURCE_DIR}/lib/d3d9.lib"
        "${CMAKE_SOURCE_DIR}/lib/d3dx9.lib"
)
install(DIRECTORY "modules/TrialMode/packs" DESTINATION "SWRSToys/modules/TrialMode")
install(DIRECTORY "modules/TrialMode/packs" DESTINATION "SokuEngine/modules/TrialMode")

module(UPnPNat)
target_sources(UPnPNat PRIVATE modules/UPnPNat/UPnPNat.cpp)
target_link_libraries(UPnPNat iphlpapi ole32 oleaut32)

module(WindowResizer Default)
target_sources(WindowResizer PRIVATE
        modules/WindowResizer/WindowResizer.cpp)
target_link_libraries(WindowResizer detours)

# binary modules

module(shady-loader Binary)
install(FILES bin/shady-manage.exe DESTINATION "SWRSToys/modules/shady-loader")
install(FILES bin/shady-manage.exe DESTINATION "SokuEngine/modules/shady-loader")

module(NetGameScore Binary SokuEngine)
module(PracticeEx Binary SokuEngine)
module(SokuEngine Binary Default SokuEngine)

# install

configure_file("swrstoys/SWRSToys.ini.in" "SWRSToys.ini" @ONLY NEWLINE_STYLE CRLF)
configure_file("modules/SokuEngine/SokuEngine.ini.in" "SokuEngine.ini" @ONLY NEWLINE_STYLE CRLF)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/SWRSToys.ini" DESTINATION "SWRSToys")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/SokuEngine.ini" DESTINATION "SokuEngine/modules/SokuEngine")
install(FILES "swrstoys/SWRSToys_SokuEngine.ini" DESTINATION "SokuEngine"
        RENAME "SWRSToys.ini")
install(FILES README.md DESTINATION "SWRSToys"
        RENAME "SokuMods.txt")
install(FILES README.md DESTINATION "SokuEngine"
        RENAME "SokuMods.txt")

# th123e loader
set(BUILD_TH123E_LOADER false CACHE BOOL "Whether to build the th123e loader executable")
if(BUILD_TH123E_LOADER)
    add_executable(th123eloader)
    target_sources(th123eloader PRIVATE
            th123eloader/th123eloader.c
            th123eloader/th123e.rc)
    target_link_libraries(th123eloader shlwapi user32)
    set_target_properties(th123eloader PROPERTIES
            OUTPUT_NAME th123e)
    install(TARGETS th123eloader RUNTIME DESTINATION ".")
endif()
