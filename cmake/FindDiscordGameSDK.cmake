#
# A CMake module to find Discord Game SDK
#
# Once done this module will define:
#  DiscordGameSDK_FOUND - system has Discord Game SDK
#  DiscordGameSDK_LIBRARIES - Link these to use Discord Game SDK
#

IF (NOT DiscordGameSDK_LIBRARIES)
    IF (MSVC)      # Visual Studio
        SET(CMAKE_FIND_LIBRARY_PREFIXES ";lib")
        SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib;.dll.a;.dll;.a")
    ELSEIF (MINGW) # Windows
        SET(CMAKE_FIND_LIBRARY_PREFIXES ";lib")
        SET(CMAKE_FIND_LIBRARY_SUFFIXES ".dll.a;.dll;.a;.lib")
    ELSE (MSVC)    # Linux
        SET(CMAKE_FIND_LIBRARY_PREFIXES "lib")
        SET(CMAKE_FIND_LIBRARY_SUFFIXES ".so;.a")
    ENDIF(MSVC)

    FIND_LIBRARY(DiscordGameSDK_LIBRARIES
        NAMES
            discord_game_sdk
        PATHS
            ${DiscordGameSDK_DIR}/lib/    # Discord Game SDK root directory (if provided)
            ${DiscordGameSDK_DIR}         # Discord Game SDK root directory (if provided)
            /usr/lib64/                   # Default Fedora28 library path
            /usr/lib/                     # Some more Linux library path
            /usr/lib/x86_64-linux-gnu/    # Some more Linux library path
            /usr/local/lib/               # Some more Linux library path
            /usr/local/lib64/             # Some more Linux library path
    )
ENDIF (NOT DiscordGameSDK_LIBRARIES)

IF (DiscordGameSDK_LIBRARIES)
    SET(DiscordGameSDK_FOUND TRUE)
ELSE (DiscordGameSDK_LIBRARIES)
    SET(DiscordGameSDK_FOUND FALSE)
ENDIF (DiscordGameSDK_LIBRARIES)

IF (DiscordGameSDK_FIND_REQUIRED AND NOT DiscordGameSDK_FOUND)
    MESSAGE(FATAL_ERROR
            "  Discord Game SDK not found.\n"
            "      Fill CMake variable DiscordGameSDK_DIR to the provided directory.\n"
            )
ENDIF (DiscordGameSDK_FIND_REQUIRED AND NOT DiscordGameSDK_FOUND)
