cmake_minimum_required(VERSION 3.15)
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

option(ENABLE_PROGRAMS "" OFF)
option(INSTALL_MBEDTLS_HEADERS "" OFF)
option(MBEDTLS_FATAL_WARNINGS "" OFF)
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

option(BUILD_CURL_EXE "" OFF)
option(BUILD_SHARED_LIBS "" OFF)
option(BUILD_TESTING "" OFF)
option(CURL_STATIC_CRT "" ON)
option(ENABLE_INET_PTON "" OFF)
option(HTTP_ONLY "" ON)
option(CURL_DISABLE_CRYPTO_AUTH "" ON)
option(ENABLE_MANUAL "" OFF)
option(CMAKE_USE_OPENSSL "" OFF)
option(CMAKE_USE_MBEDTLS "" ON)
option(CURL_DISABLE_LDAP "" ON)
option(CURL_ZLIB "" OFF)
option(CMAKE_USE_LIBSSH2 "" OFF)
option(ENABLE_UNIX_SOCKETS "" OFF)
option(CURL_CA_BUNDLE "" none)
option(CURL_CA_PATH "" none)
option(CURL_WINDOWS_SSPI "" OFF)
add_subdirectory(curl EXCLUDE_FROM_ALL)

#[[
add_subdirectory(zlib)
add_library(ZLIB::ZLIB ALIAS zlibstatic)

SET(BUILD_SHARED_LIBS OFF)
SET(LIBZIP_DO_INSTALL OFF)
SET(ENABLE_COMMONCRYPTO OFF)
SET(ENABLE_GNUTLS OFF)
SET(ENABLE_OPENSSL OFF)
SET(ENABLE_WINDOWS_CRYPTO OFF)
SET(ENABLE_BZIP2 OFF)
SET(ENABLE_LZMA OFF)
SET(ENABLE_ZSTD OFF)
SET(ZLIB_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/zlib")
SET(ZLIB_LIBRARY "${CMAKE_SOURCE_DIR}/zlib")
add_subdirectory(libzip)
add_dependencies(zip zlibstatic)
target_include_directories(zip PRIVATE ${ZLIB_INCLUDE_DIR})
]]
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

# MemoryModule

add_library(MemoryModule
        MemoryModule/MemoryModule.c)
target_include_directories(MemoryModule PUBLIC MemoryModule)

# SokuLib
add_subdirectory(SokuLib)

# SokuEngineLib

add_library(SokuEngineLib STATIC IMPORTED)
set_target_properties(SokuEngineLib PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/SokuEngineLib.lib)
set_target_properties(SokuEngineLib PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include/)

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

module(Blacklist)
target_sources(Blacklist PRIVATE modules/Blacklist/main.cpp)
target_link_libraries(Blacklist SokuLib shlwapi ws2_32)

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
target_include_directories(InGameHostlist PRIVATE include include/directx modules/InGameHostlist/include)
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

module(NetProfileView)
target_sources(NetProfileView PRIVATE modules/NetProfileView/NetProfileView.cpp)

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

#module(SokuReplays Default)
#target_compile_definitions(SokuReplays PRIVATE PSAPI_VERSION=1)
#target_sources(SokuReplays PRIVATE modules/SokuReplays/main.cpp)
#target_link_libraries(SokuReplays CURL::libcurl psapi shlwapi user32 ws2_32)

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

module(UPnPNat)
target_sources(UPnPNat PRIVATE modules/UPnPNat/UPnPNat.cpp)
target_link_libraries(UPnPNat iphlpapi ole32 oleaut32)

module(WindowedFullscreen)
target_sources(WindowedFullscreen PRIVATE
        modules/WindowedFullscreen/WindowedFullscreen.cpp)
# passing "d3d9" conflicts with the generated d3d9 module somehow, pass an absolute path to the one in lib/
target_link_libraries(WindowedFullscreen detours "${CMAKE_SOURCE_DIR}/lib/d3d9.lib")

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

add_executable(th123eloader)
target_sources(th123eloader PRIVATE
        th123eloader/th123eloader.c
        th123eloader/th123e.rc)
target_link_libraries(th123eloader shlwapi user32)
set_target_properties(th123eloader PROPERTIES
        OUTPUT_NAME th123e)
install(TARGETS th123eloader RUNTIME DESTINATION ".")

# WIP Module

add_library(
        "TrialMode"
        MODULE
        modules/TrialMode/TrialMode.rc
        modules/TrialMode/main.cpp
        modules/TrialMode/gif_load.h
	modules/TrialMode/Menu.cpp
        modules/TrialMode/Menu.hpp
        modules/TrialMode/Pack.cpp
        modules/TrialMode/Pack.hpp
        modules/TrialMode/Trial/Trial.cpp
        modules/TrialMode/Trial/Trial.hpp
        modules/TrialMode/Trial/ComboTrial.cpp
        modules/TrialMode/Trial/ComboTrial.hpp
        modules/TrialMode/TrialEditor/TrialEditor.cpp
        modules/TrialMode/TrialEditor/TrialEditor.hpp
        modules/TrialMode/TrialEditor/ComboTrialEditor.cpp
        modules/TrialMode/TrialEditor/ComboTrialEditor.hpp
        modules/TrialMode/Actions.cpp
        modules/TrialMode/Actions.hpp
        modules/TrialMode/Animations/BattleAnimation.cpp
        modules/TrialMode/Animations/BattleAnimation.hpp
        modules/TrialMode/Animations/BattleAnimFramework.cpp
        modules/TrialMode/Animations/LuaBattleAnimation.cpp
        modules/TrialMode/Animations/LuaBattleAnimation.hpp
        modules/TrialMode/Images.cpp
        modules/TrialMode/Images.hpp
	modules/TrialMode/Animations/FakeChrMgr.cpp
        modules/TrialMode/Animations/FakeChrMgr.hpp
        modules/TrialMode/Patches.cpp
        modules/TrialMode/Patches.hpp
        modules/TrialMode/TrialBase.hpp
        modules/TrialMode/TrialBase.cpp
        modules/TrialMode/PackOutro.cpp
        modules/TrialMode/PackOutro.hpp
        modules/TrialMode/InputBox.cpp
        modules/TrialMode/InputBox.hpp
	modules/TrialMode/Explorer.cpp
        modules/TrialMode/Explorer.hpp
	modules/TrialMode/Socket.cpp
        modules/TrialMode/Socket.hpp
        modules/TrialMode/ExplorerMenu.cpp
        modules/TrialMode/ExplorerMenu.hpp
        modules/TrialMode/Crc32.cpp
        modules/TrialMode/Crc32.h
)
target_compile_options(TrialMode PRIVATE /bigobj)
target_compile_definitions("TrialMode" PRIVATE DIRECTINPUT_VERSION=0x0800 CURL_STATICLIB _CRT_SECURE_NO_WARNINGS $<$<CONFIG:Debug>:_DEBUG> WINVER=0x0601 _WIN32_WINNT=0x0601)
target_include_directories("TrialMode" PRIVATE include include/directx "modules/TrialMode/")
target_link_directories("TrialMode" PRIVATE lib)
target_link_libraries(
        TrialMode
        SokuLib
        shlwapi
        ws2_32
        user32
        lua54
        zip
	#stdc++fs
        "${CMAKE_SOURCE_DIR}/lib/d3d9.lib"
        "${CMAKE_SOURCE_DIR}/lib/d3dx9.lib"
)
install(TARGETS "TrialMode" LIBRARY DESTINATION "SWRSToys/modules/TrialMode/")
install(TARGETS "TrialMode" LIBRARY DESTINATION "SokuEngine/modules/TrialMode/")


#add_library(
        #"saku_basic_sakuyaOutro6"
        #MODULE
        #modules/TrialMode/Animations/BattleAnimation.hpp
        #modules/TrialMode/Animations/SakuyaBasicCombos/SakuyaOut6.cpp
        #modules/TrialMode/Animations/BattleAnimFramework.cpp
#)
#target_compile_options(saku_basic_sakuyaOutro6 PRIVATE /Zi)
#target_compile_definitions("saku_basic_sakuyaOutro6" PRIVATE DIRECTINPUT_VERSION=0x0800 CURL_STATICLIB _CRT_SECURE_NO_WARNINGS $<$<CONFIG:Debug>:_DEBUG>)
#target_include_directories("saku_basic_sakuyaOutro6" PRIVATE include include/directx "modules/TrialMode/")
#target_link_directories("saku_basic_sakuyaOutro6" PRIVATE lib)
#target_link_libraries(
        #saku_basic_sakuyaOutro6
        #SokuLib
        #shlwapi
        #ws2_32
        #user32
        #lua54
        #"${CMAKE_SOURCE_DIR}/lib/d3d9.lib"
        #"${CMAKE_SOURCE_DIR}/lib/d3dx9.lib"
#)
#set_target_properties(saku_basic_sakuyaOutro6 PROPERTIES OUTPUT_NAME lore6Outro)
#set_target_properties(saku_basic_sakuyaOutro6 PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/modules/TrialMode/packs/sakuya_story" PREFIX "")
#
#add_dependencies(TrialMode saku_basic_sakuyaOutro6)

#-----------------------------------------------------#

#add_library(
#        "remi_basic_bnbcorn1"
#        MODULE
#        modules/TrialMode/Animations/BattleAnimation.hpp
#        modules/TrialMode/Animations/RemiliaBasicCombos/bnbcorn1.cpp
#        modules/TrialMode/Animations/BattleAnimFramework.cpp
#)
#target_compile_options(remi_basic_bnbcorn1 PRIVATE /Zi)
#target_compile_definitions("remi_basic_bnbcorn1" PRIVATE DIRECTINPUT_VERSION=0x0800 CURL_STATICLIB _CRT_SECURE_NO_WARNINGS $<$<CONFIG:Debug>:_DEBUG>)
#target_include_directories("remi_basic_bnbcorn1" PRIVATE include include/directx "modules/TrialMode/")
#target_link_directories("remi_basic_bnbcorn1" PRIVATE lib)
#target_link_libraries(
#        remi_basic_bnbcorn1
#        SokuLib
#        shlwapi
#        ws2_32
#        user32
#        lua54
#        "${CMAKE_SOURCE_DIR}/lib/d3d9.lib"
#        "${CMAKE_SOURCE_DIR}/lib/d3dx9.lib"
#)
#set_target_properties(remi_basic_bnbcorn1 PROPERTIES OUTPUT_NAME bnbcorn1intro)
#set_target_properties(remi_basic_bnbcorn1 PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/modules/TrialMode/packs/remilia_story" PREFIX "")

#add_dependencies(TrialMode remi_basic_bnbcorn1)

#-----------------------------------------------------#

add_library(
        "PunishDisplay"
        MODULE
        modules/PunishDisplay/main.cpp
)
target_compile_options(PunishDisplay PRIVATE /Zi)
target_compile_definitions("PunishDisplay" PRIVATE DIRECTINPUT_VERSION=0x0800 CURL_STATICLIB _CRT_SECURE_NO_WARNINGS $<$<CONFIG:Debug>:_DEBUG>)
target_include_directories("PunishDisplay" PRIVATE include include/directx "modules/PunishDisplay/")
target_link_directories("PunishDisplay" PRIVATE lib)
target_link_libraries(
        PunishDisplay
        SokuLib
        shlwapi
        ws2_32
        user32
        "${CMAKE_SOURCE_DIR}/lib/d3d9.lib"
        "${CMAKE_SOURCE_DIR}/lib/d3dx9.lib"
)
install(TARGETS "PunishDisplay" LIBRARY DESTINATION "SWRSToys/modules/PunishDisplay/")
install(TARGETS "PunishDisplay" LIBRARY DESTINATION "SokuEngine/modules/PunishDisplay/")

add_library(
        "InfiniteSubsun"
        MODULE
        modules/InfiniteSubsun/main.cpp
)
target_compile_options(InfiniteSubsun PRIVATE /Zi)
target_compile_definitions("InfiniteSubsun" PRIVATE DIRECTINPUT_VERSION=0x0800 CURL_STATICLIB _CRT_SECURE_NO_WARNINGS $<$<CONFIG:Debug>:_DEBUG>)
target_include_directories("InfiniteSubsun" PRIVATE include include/directx "modules/InfiniteSubsun/")
target_link_directories("InfiniteSubsun" PRIVATE lib)
target_link_libraries(
        InfiniteSubsun
        SokuLib
        shlwapi
        ws2_32
        user32
        "${CMAKE_SOURCE_DIR}/lib/d3d9.lib"
        "${CMAKE_SOURCE_DIR}/lib/d3dx9.lib"
)
install(TARGETS "InfiniteSubsun" LIBRARY DESTINATION "SWRSToys/modules/InfiniteSubsun/")
install(TARGETS "InfiniteSubsun" LIBRARY DESTINATION "SokuEngine/modules/InfiniteSubsun/")

add_library(
        "TimerSoku"
        MODULE
        modules/TimerSoku/main.cpp
)
target_compile_options(TimerSoku PRIVATE /Zi)
target_compile_definitions("TimerSoku" PRIVATE DIRECTINPUT_VERSION=0x0800 CURL_STATICLIB _CRT_SECURE_NO_WARNINGS $<$<CONFIG:Debug>:_DEBUG>)
target_include_directories("TimerSoku" PRIVATE include include/directx "modules/TimerSoku/")
target_link_directories("TimerSoku" PRIVATE lib)
target_link_libraries(
        TimerSoku
        SokuLib
        shlwapi
        ws2_32
        user32
        "${CMAKE_SOURCE_DIR}/lib/d3d9.lib"
        "${CMAKE_SOURCE_DIR}/lib/d3dx9.lib"
)
install(TARGETS "TimerSoku" LIBRARY DESTINATION "SWRSToys/modules/TimerSoku/")
install(TARGETS "TimerSoku" LIBRARY DESTINATION "SokuEngine/modules/TimerSoku/")


add_library(
        "SuddenDeath"
        MODULE
        modules/SuddenDeath/main.cpp
)
target_compile_options(SuddenDeath PRIVATE /Zi)
target_compile_definitions("SuddenDeath" PRIVATE DIRECTINPUT_VERSION=0x0800 CURL_STATICLIB _CRT_SECURE_NO_WARNINGS $<$<CONFIG:Debug>:_DEBUG>)
target_include_directories("SuddenDeath" PRIVATE include include/directx "modules/SuddenDeath/")
target_link_directories("SuddenDeath" PRIVATE lib)
target_link_libraries(
        SuddenDeath
        SokuLib
        shlwapi
        ws2_32
        user32
        "${CMAKE_SOURCE_DIR}/lib/d3d9.lib"
        "${CMAKE_SOURCE_DIR}/lib/d3dx9.lib"
)
install(TARGETS "SuddenDeath" LIBRARY DESTINATION "SWRSToys/modules/SuddenDeath/")
install(TARGETS "SuddenDeath" LIBRARY DESTINATION "SokuEngine/modules/SuddenDeath/")


add_library(
        "DamagingCards"
        MODULE
        modules/DamagingCards/main.cpp
)
target_compile_options(DamagingCards PRIVATE /Zi)
target_compile_definitions("DamagingCards" PRIVATE DIRECTINPUT_VERSION=0x0800 CURL_STATICLIB _CRT_SECURE_NO_WARNINGS $<$<CONFIG:Debug>:_DEBUG>)
target_include_directories("DamagingCards" PRIVATE include include/directx "modules/DamagingCards/")
target_link_directories("DamagingCards" PRIVATE lib)
target_link_libraries(
        DamagingCards
        SokuLib
        shlwapi
        ws2_32
        user32
        "${CMAKE_SOURCE_DIR}/lib/d3d9.lib"
        "${CMAKE_SOURCE_DIR}/lib/d3dx9.lib"
)
install(TARGETS "DamagingCards" LIBRARY DESTINATION "SWRSToys/modules/DamagingCards/")
install(TARGETS "DamagingCards" LIBRARY DESTINATION "SokuEngine/modules/DamagingCards/")


add_library(
        "CardChaos"
        MODULE
        modules/CardChaos/main.cpp
)
target_compile_options(CardChaos PRIVATE /Zi)
target_compile_definitions("CardChaos" PRIVATE DIRECTINPUT_VERSION=0x0800 CURL_STATICLIB _CRT_SECURE_NO_WARNINGS $<$<CONFIG:Debug>:_DEBUG>)
target_include_directories("CardChaos" PRIVATE include include/directx "modules/CardChaos/")
target_link_directories("CardChaos" PRIVATE lib)
target_link_libraries(
        CardChaos
        SokuLib
        shlwapi
        ws2_32
        user32
        "${CMAKE_SOURCE_DIR}/lib/d3d9.lib"
        "${CMAKE_SOURCE_DIR}/lib/d3dx9.lib"
)
install(TARGETS "SuddenDeath" LIBRARY DESTINATION "SWRSToys/modules/CardChaos/")
install(TARGETS "SuddenDeath" LIBRARY DESTINATION "SokuEngine/modules/CardChaos/")


add_library(
        "InfiniteWeather"
        MODULE
        modules/InfiniteWeather/main.cpp
)
target_compile_options(InfiniteWeather PRIVATE /Zi)
target_compile_definitions("InfiniteWeather" PRIVATE DIRECTINPUT_VERSION=0x0800 CURL_STATICLIB _CRT_SECURE_NO_WARNINGS $<$<CONFIG:Debug>:_DEBUG>)
target_include_directories("InfiniteWeather" PRIVATE include include/directx "modules/InfiniteWeather/")
target_link_directories("InfiniteWeather" PRIVATE lib)
target_link_libraries(
        InfiniteWeather
        SokuLib
        shlwapi
        ws2_32
        user32
        "${CMAKE_SOURCE_DIR}/lib/d3d9.lib"
        "${CMAKE_SOURCE_DIR}/lib/d3dx9.lib"
)
install(TARGETS "InfiniteWeather" LIBRARY DESTINATION "SWRSToys/modules/InfiniteWeather/")
install(TARGETS "InfiniteWeather" LIBRARY DESTINATION "SokuEngine/modules/InfiniteWeather/")

add_library(
        "AssistSoku"
        MODULE
        modules/AssistSoku/main.cpp
)
target_compile_options(AssistSoku PRIVATE /Zi)
target_compile_definitions("AssistSoku" PRIVATE DIRECTINPUT_VERSION=0x0800 CURL_STATICLIB _CRT_SECURE_NO_WARNINGS $<$<CONFIG:Debug>:_DEBUG>)
target_include_directories("AssistSoku" PRIVATE include include/directx "modules/AssistSoku/")
target_link_directories("AssistSoku" PRIVATE lib)
target_link_libraries(
        AssistSoku
        SokuLib
        shlwapi
        ws2_32
        user32
        "${CMAKE_SOURCE_DIR}/lib/d3d9.lib"
        "${CMAKE_SOURCE_DIR}/lib/d3dx9.lib"
)
install(TARGETS "AssistSoku" LIBRARY DESTINATION "SWRSToys/modules/AssistSoku/")
install(TARGETS "AssistSoku" LIBRARY DESTINATION "SokuEngine/modules/AssistSoku/")


add_library(
        "ShuffledWeather"
        MODULE
        modules/ShuffledWeather/main.cpp
)
target_compile_options(ShuffledWeather PRIVATE /Zi)
target_compile_definitions("ShuffledWeather" PRIVATE DIRECTINPUT_VERSION=0x0800 CURL_STATICLIB _CRT_SECURE_NO_WARNINGS $<$<CONFIG:Debug>:_DEBUG>)
target_include_directories("ShuffledWeather" PRIVATE include include/directx "modules/ShuffledWeather/")
target_link_directories("ShuffledWeather" PRIVATE lib)
target_link_libraries(
        ShuffledWeather
        SokuLib
        shlwapi
        ws2_32
        user32
        "${CMAKE_SOURCE_DIR}/lib/d3d9.lib"
        "${CMAKE_SOURCE_DIR}/lib/d3dx9.lib"
)
install(TARGETS "ShuffledWeather" LIBRARY DESTINATION "SWRSToys/modules/ShuffledWeather/")
install(TARGETS "ShuffledWeather" LIBRARY DESTINATION "SokuEngine/modules/ShuffledWeather/")


add_library(
        "BurningCards"
        MODULE
        modules/BurningCards/main.cpp
)
target_compile_options(BurningCards PRIVATE /Zi)
target_compile_definitions("BurningCards" PRIVATE DIRECTINPUT_VERSION=0x0800 CURL_STATICLIB _CRT_SECURE_NO_WARNINGS $<$<CONFIG:Debug>:_DEBUG>)
target_include_directories("BurningCards" PRIVATE include include/directx "modules/BurningCards/")
target_link_directories("BurningCards" PRIVATE lib)
target_link_libraries(
        BurningCards
        SokuLib
        shlwapi
        ws2_32
        user32
        "${CMAKE_SOURCE_DIR}/lib/d3d9.lib"
        "${CMAKE_SOURCE_DIR}/lib/d3dx9.lib"
)
install(TARGETS "BurningCards" LIBRARY DESTINATION "SWRSToys/modules/BurningCards/")
install(TARGETS "BurningCards" LIBRARY DESTINATION "SokuEngine/modules/BurningCards/")

add_library(
        "HostInBackground"
        MODULE
        modules/HostInBackground/main.cpp
)
target_compile_options(HostInBackground PRIVATE /Zi)
target_compile_definitions("HostInBackground" PRIVATE DIRECTINPUT_VERSION=0x0800 CURL_STATICLIB _CRT_SECURE_NO_WARNINGS $<$<CONFIG:Debug>:_DEBUG> ${NET})
target_include_directories("HostInBackground" PRIVATE include include/directx "modules/HostInBackground/")
target_link_directories("HostInBackground" PRIVATE lib)
target_link_libraries(
        HostInBackground
        SokuLib
        shlwapi
        ws2_32
	d3dx9
)
install(TARGETS "HostInBackground" LIBRARY DESTINATION "SWRSToys/modules/HostInBackground/")
install(TARGETS "HostInBackground" LIBRARY DESTINATION "SokuEngine/modules/HostInBackground/")
