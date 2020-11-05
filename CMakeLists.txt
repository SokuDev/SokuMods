cmake_minimum_required(VERSION 3.17)
project(HisoutensokuDiscordIntegration)

set(CMAKE_CXX_STANDARD 14)

if ("${CMAKE_BUILD_TYPE}" STREQUAL Debug)
	add_compile_definitions(_DEBUG=1)
	if (CMAKE_COMPILER_IS_GNUCC)
		add_compile_options(-Wall -Wextra -W -g)
	endif (CMAKE_COMPILER_IS_GNUCC)
endif ()

include_directories(src/DiscordSDK include SokuLib/src)

add_subdirectory(SokuLib)

add_library(
	DiscordSDK
	src/DiscordSDK/achievement_manager.cpp
	src/DiscordSDK/achievement_manager.h
	src/DiscordSDK/activity_manager.cpp
	src/DiscordSDK/activity_manager.h
	src/DiscordSDK/application_manager.cpp
	src/DiscordSDK/application_manager.h
	src/DiscordSDK/core.cpp
	src/DiscordSDK/core.h
	src/DiscordSDK/discord.h
	src/DiscordSDK/event.h
	src/DiscordSDK/ffi.h
	src/DiscordSDK/image_manager.cpp
	src/DiscordSDK/image_manager.h
	src/DiscordSDK/lobby_manager.cpp
	src/DiscordSDK/lobby_manager.h
	src/DiscordSDK/network_manager.cpp
	src/DiscordSDK/network_manager.h
	src/DiscordSDK/overlay_manager.cpp
	src/DiscordSDK/overlay_manager.h
	src/DiscordSDK/relationship_manager.cpp
	src/DiscordSDK/relationship_manager.h
	src/DiscordSDK/storage_manager.cpp
	src/DiscordSDK/storage_manager.h
	src/DiscordSDK/store_manager.cpp
	src/DiscordSDK/store_manager.h
	src/DiscordSDK/types.cpp
	src/DiscordSDK/types.h
	src/DiscordSDK/user_manager.cpp
	src/DiscordSDK/user_manager.h
	src/DiscordSDK/voice_manager.cpp
	src/DiscordSDK/voice_manager.h
)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
find_package(DiscordGameSDK REQUIRED)

if (NOT DEFINED CLIENT_ID)
	set(CLIENT_ID 771968286199382056)
	message(WARNING "CLIENT_ID is not set")
endif ()

add_library(
	DiscordIntegration MODULE
	src/Library/Network/getPublicIp.cpp
	src/Library/Network/getPublicIp.hpp
	src/Library/main.cpp
	src/Library/SokuUtils/SokuState.cpp
	src/Library/SokuUtils/SokuState.hpp
	src/Library/logger.cpp
        src/Library/logger.hpp
	src/Library/Network/Socket.cpp
	src/Library/Network/Socket.hpp
        src/Library/Exceptions.hpp
)

if (DEFINED VERSION_110a)
	target_compile_definitions(
		DiscordIntegration PRIVATE
		ClientID="${CLIENT_ID}"
		ClientSecret="${CLIENT_SECRET}"
		SOKU_VER_110a=${VERSION_110a}
	)
else ()
	target_compile_definitions(
		DiscordIntegration PRIVATE
		ClientID="${CLIENT_ID}"
		ClientSecret="${CLIENT_SECRET}"
	)
endif ()

target_link_libraries(
	DiscordIntegration
	DiscordSDK
	SokuLib
	ws2_32
	shlwapi
	${DiscordGameSDK_LIBRARIES}
)
