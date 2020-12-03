cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)
project(swrstoys C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/install")

set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND "${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++11-narrowing -Wno-microsoft-cast")
    # libcurl uses a copyright symbol escape in its file description string for its resource control file
    # work around this undefined behaviour by setting a global codepage for all RC files
    SET(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} /C 1252")
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
option(CURL_DISABLE_LDAP "" ON)
option(CURL_ZLIB "" OFF)
option(CMAKE_USE_LIBSSH2 "" OFF)
option(ENABLE_UNIX_SOCKETS "" OFF)
option(CURL_CA_BUNDLE "" none)
option(CURL_CA_PATH "" none)
option(CURL_WINDOWS_SSPI "" OFF)
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
target_link_libraries(DiscordSDK INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/bin/discord_game_sdk.dll.lib)

# MemoryModule

add_library(MemoryModule
        MemoryModule/MemoryModule.c)
target_include_directories(MemoryModule PUBLIC MemoryModule)

# SokuLib

set(VERSION_110a "1")
add_subdirectory(SokuLib)

# modules

function(module name)
    add_library("${name}" MODULE)
    target_compile_definitions("${name}" PRIVATE DIRECTINPUT_VERSION=0x0800 CURL_STATICLIB _CRT_SECURE_NO_WARNINGS $<$<CONFIG:Debug>:_DEBUG>)
    target_compile_options("${name}" PRIVATE $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall>)
    target_include_directories("${name}" PRIVATE include include/directx "modules/${name}/")
    target_link_directories("${name}" PRIVATE lib)
    target_link_libraries("${name}" shlwapi user32 advapi32)
    install(TARGETS "${name}" LIBRARY DESTINATION "modules/${name}/")
    set(MODULE_PATHS "${MODULE_PATHS}; ${name}=modules/${name}/${name}.dll\n" PARENT_SCOPE)
endfunction()

module(Autopunch)
target_sources(Autopunch PRIVATE modules/Autopunch/Autopunch.c)
target_link_libraries(Autopunch CURL::libcurl shlwapi version user32 ws2_32)

module(BGMChanger)
target_sources(BGMChanger PRIVATE modules/BGMChanger/BGMChanger.cpp)

module(CharactersInForeground)
target_sources(CharactersInForeground PRIVATE modules/CharactersInForeground/CharactersInForeground.cpp)

set(DISCORD_CLIENT_ID "771968286199382056" CACHE STRING "DiscordIntegration mod Discord client ID")
module(DiscordIntegration)
target_compile_definitions(DiscordIntegration PRIVATE _CRT_SECURE_NO_WARNINGS $<$<CONFIG:Debug>:_DEBUG> ClientID="${DISCORD_CLIENT_ID}" SOKU_VER_110a)
target_sources(DiscordIntegration PRIVATE
        modules/DiscordIntegration/Network/getPublicIp.cpp
        modules/DiscordIntegration/Network/Socket.cpp
        modules/DiscordIntegration/SokuUtils/SokuState.cpp
        modules/DiscordIntegration/logger.cpp
        modules/DiscordIntegration/main.cpp
        modules/DiscordIntegration/ShiftJISDecoder.cpp)
target_link_libraries(DiscordIntegration DiscordSDK SokuLib shlwapi user32 ws2_32)

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

module(SkipIntro)
target_sources(SkipIntro PRIVATE
        modules/SkipIntro/Soku-SkipIntro/dllmain.cpp)

module(SWRSokuRoll)
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

module(WindowResizer)
target_sources(WindowResizer PRIVATE
        modules/WindowResizer/WindowResizer.cpp)
target_link_libraries(WindowResizer detours)

configure_file("swrstoys/SWRSToys.ini.in" "SWRSToys.ini" @ONLY NEWLINE_STYLE CRLF)

install(DIRECTORY modules/ DESTINATION "modules"
        FILES_MATCHING
        REGEX "modules/[^/]+/[^/.]+(/|$)" EXCLUDE
        PATTERN "*.ini")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/SWRSToys.ini" DESTINATION ".")
install(FILES README.md DESTINATION "."
        RENAME "SWRSToys.txt")
