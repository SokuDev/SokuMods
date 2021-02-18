cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)
project(swrstoys C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/install")

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

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
target_link_libraries(swrstoys shlwapi user32 advapi32 shell32)
set_target_properties(swrstoys PROPERTIES
        OUTPUT_NAME d3d9)
install(TARGETS swrstoys LIBRARY DESTINATION "SWRSToys/")
install(TARGETS swrstoys LIBRARY DESTINATION "SokuEngine/")

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
#add_subdirectory(curl EXCLUDE_FROM_ALL)

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
            set(SWRSTOYS_MODULE_PATHS "${SWRSTOYS_MODULE_PATHS}${name}=modules/${name}/${name}.dll\n" PARENT_SCOPE)
        else()
            set(SWRSTOYS_MODULE_PATHS "${SWRSTOYS_MODULE_PATHS}; ${name}=modules/${name}/${name}.dll\n" PARENT_SCOPE)
        endif()
    endif()
    install(DIRECTORY "modules/${name}" DESTINATION "SokuEngine/modules"
            FILES_MATCHING
            REGEX "modules/${name}/[^/.]+(/|$)" EXCLUDE
            PATTERN "*.ini")
    set(SOKUENGINE_MODULE_PATHS "${SOKUENGINE_MODULE_PATHS}${name} = ${Default}\n" PARENT_SCOPE)
endfunction()

module(Autopunch Default)
target_sources(Autopunch PRIVATE modules/Autopunch/Autopunch.c)
#target_link_libraries(Autopunch CURL::libcurl shlwapi version user32 ws2_32)

module(BGMChanger)
target_sources(BGMChanger PRIVATE modules/BGMChanger/BGMChanger.cpp)

module(CharactersInForeground)
target_sources(CharactersInForeground PRIVATE modules/CharactersInForeground/CharactersInForeground.cpp)

# DiscordIntegration main module
set(DISCORD_CLIENT_ID "771968286199382056" CACHE STRING "DiscordIntegration mod Discord client ID")
module(DiscordIntegration)
target_compile_definitions(DiscordIntegration PRIVATE _CRT_SECURE_NO_WARNINGS $<$<CONFIG:Debug>:_DEBUG> ClientID="${DISCORD_CLIENT_ID}")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND "${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
    target_compile_options(DiscordIntegration PRIVATE /EHsc)
endif ()
target_link_libraries(DiscordIntegration DiscordSDK SokuLib shlwapi user32 ws2_32)
set_target_properties(DiscordIntegration PROPERTIES
        SUFFIX ".dat")
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
        modules/DiscordIntegration/CompiledString/Vars/vars.cpp
        modules/DiscordIntegration/CompiledString/CompiledStringFactory.cpp)
install(DIRECTORY "modules/DiscordIntegration/langs" DESTINATION "SWRSToys/modules/DiscordIntegration")
install(DIRECTORY "modules/DiscordIntegration/langs" DESTINATION "SokuEngine/modules/DiscordIntegration")
# DiscordIntegration loader
add_library(DiscordIntegrationLoader MODULE)
target_sources(DiscordIntegrationLoader PRIVATE
        modules/DiscordIntegration/Loader.c)
target_link_libraries(DiscordIntegrationLoader shlwapi user32 advapi32)
set_target_properties(DiscordIntegrationLoader PROPERTIES
        OUTPUT_NAME "DiscordIntegration")
install(TARGETS DiscordIntegrationLoader LIBRARY DESTINATION "SWRSToys/modules/DiscordIntegration/")
install(TARGETS DiscordIntegrationLoader LIBRARY DESTINATION "SokuEngine/modules/DiscordIntegration/")
# DiscordIntegration Discord sdk
install(FILES bin/discord_game_sdk.dll DESTINATION "SWRSToys/modules/DiscordIntegration/")
install(FILES bin/discord_game_sdk.dll DESTINATION "SokuEngine/modules/DiscordIntegration/")

module(DPadFix)
target_sources(DPadFix PRIVATE modules/DPadFix/DPadFix.cpp)
target_link_libraries(DPadFix dinput8 detours)

module(Hostlist SokuEngine)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND "${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
    target_compile_options(Hostlist PRIVATE /EHsc)
endif ()
target_sources(Hostlist PRIVATE
        modules/Hostlist/dllmain.cpp)
#target_link_libraries(Hostlist CURL::libcurl SokuEngineLib ws2_32 shlwapi)

install(FILES modules/Hostlist/hostlistBG.png DESTINATION "SokuEngine/modules/Hostlist")
install(FILES modules/Hostlist/romanan.ttf DESTINATION "SokuEngine/modules/Hostlist")

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

module(SkipIntro Default)
target_sources(SkipIntro PRIVATE
        modules/SkipIntro/Soku-SkipIntro/dllmain.cpp)

module(FixMeiling236 Default)
target_sources(FixMeiling236 PRIVATE
        modules/FixMeiling236/main.cpp)

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

module(WindowResizer)
target_sources(WindowResizer PRIVATE
        modules/WindowResizer/WindowResizer.cpp)
target_link_libraries(WindowResizer detours)

# binary SokuEngine modules

module(GetIP Binary SokuEngine)
module(NetGameScore Binary SokuEngine)
module(PracticeEx Binary SokuEngine)
module(shady-loader Binary SokuEngine)
module(shady-lua Binary SokuEngine)
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



# WIP Module

add_library(
        "AdvancedPracticeMode"
        MODULE
        modules/AdvancedPracticeMode/main.cpp
	modules/AdvancedPracticeMode/Moves.cpp
        modules/AdvancedPracticeMode/Moves.hpp
        modules/AdvancedPracticeMode/Gui.cpp
        modules/AdvancedPracticeMode/Gui.hpp
)
target_compile_definitions("AdvancedPracticeMode" PRIVATE DIRECTINPUT_VERSION=0x0800 CURL_STATICLIB _CRT_SECURE_NO_WARNINGS $<$<CONFIG:Debug>:_DEBUG>)
target_include_directories("AdvancedPracticeMode" PRIVATE include include/directx "modules/AdvancedPracticeMode/" "${SFML_DIR}/include" "${TGUI_DIR}/include")
target_link_directories("AdvancedPracticeMode" PRIVATE lib)
if (${CMAKE_BUILD_TYPE} STREQUAL Debug)
    set(TGUILIB "${TGUI_DIR}/lib/tgui.lib")
else ()
    set(TGUILIB "${TGUI_DIR}/lib/tgui.lib")
endif ()
target_link_libraries(
        AdvancedPracticeMode
        SokuLib
        shlwapi
        ws2_32
        user32
        "${SFML_DIR}/lib/sfml-graphics.lib"
        "${SFML_DIR}/lib/sfml-system.lib"
        "${SFML_DIR}/lib/sfml-window.lib"
        "${TGUILIB}"
)
message(${TGUILIB})
install(TARGETS "AdvancedPracticeMode" LIBRARY DESTINATION "SWRSToys/modules/AdvancedPracticeMode/")
install(TARGETS "AdvancedPracticeMode" LIBRARY DESTINATION "SokuEngine/modules/AdvancedPracticeMode/")