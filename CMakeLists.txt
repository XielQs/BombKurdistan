cmake_minimum_required(VERSION 3.16)
project(bombkurdistan LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE Release)

file(GLOB SRCS "${PROJECT_SOURCE_DIR}/src/*.cpp")

set(ZIP_NAME "bombkurdistan_linux.zip")
set(ZIP_NAME_WIN "bombkurdistan_windows.zip")

# TODO: add pedantic
add_compile_options(
        -Wall -Wextra
)

option(DEBUG_MODE "Enable debug mode" OFF)
option(DISCORD_RPC "Enable Discord RPC" ON)

if (DEBUG_MODE)
    add_compile_definitions(DEBUG_MODE)
    add_compile_options(-O0 -g3 -ggdb3)
else ()
    add_compile_options(-O3)
    add_link_options(-s)
endif ()

if (DISCORD_RPC)
    if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        message(WARNING "Discord RPC is not supported on Windows. Disabling Discord RPC.")
        set(DISCORD_RPC OFF)
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        add_compile_definitions(DISCORD_RPC_ENABLED)
        message(STATUS "Discord RPC enabled")
    endif ()
else ()
    message(STATUS "Discord RPC disabled")
endif ()

add_compile_definitions(PLATFORM_DESKTOP)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Download and set up raylib for Windows
    set(RAYLIB_WIN_LIB "${PROJECT_SOURCE_DIR}/lib/raylib/src/libraylib.win.a")

    if (NOT EXISTS ${RAYLIB_WIN_LIB})
        message(STATUS "Downloading raylib for Windows...")

        set(RAYLIB_URL "https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_win64_mingw-w64.zip")
        set(RAYLIB_ZIP "${CMAKE_BINARY_DIR}/raylib.zip")

        # Download
        file(DOWNLOAD ${RAYLIB_URL} ${RAYLIB_ZIP}
                SHOW_PROGRESS
                STATUS download_status)

        list(GET download_status 0 status_code)
        if (NOT status_code EQUAL 0)
            message(FATAL_ERROR "Failed to download raylib")
        endif ()

        # Extract
        file(ARCHIVE_EXTRACT INPUT ${RAYLIB_ZIP}
                DESTINATION ${CMAKE_BINARY_DIR})

        # Move
        file(RENAME ${CMAKE_BINARY_DIR}/raylib-5.5_win64_mingw-w64/lib/libraylib.a
                ${RAYLIB_WIN_LIB})

        # Cleanup
        file(REMOVE ${RAYLIB_ZIP})
        file(REMOVE_RECURSE ${CMAKE_BINARY_DIR}/raylib-5.5_win64_mingw-w64)

        message(STATUS "Raylib for Windows downloaded and extracted")
    endif ()

    add_library(raylib STATIC IMPORTED)
    set_target_properties(raylib PROPERTIES
            IMPORTED_LOCATION ${RAYLIB_WIN_LIB}
    )

    link_directories(${PROJECT_SOURCE_DIR}/lib/raylib/src)
else ()
    # Build raylib
    find_package(raylib QUIET)
    if (NOT raylib_FOUND)
        add_subdirectory(lib/raylib)
    endif ()
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Windows
    set(LINK_LIBS raylib.win gdi32 winmm)
    add_link_options(-static)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Linux
    set(LINK_LIBS raylib)

    if (DISCORD_RPC)
        list(APPEND LINK_LIBS discordrpc)
        link_directories(${PROJECT_SOURCE_DIR}/lib/discordrpc/build)
    endif ()
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # macOS
    set(LINK_LIBS raylib
        "-framework Cocoa"
    )
endif ()

add_executable(${PROJECT_NAME} ${SRCS})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${PROJECT_SOURCE_DIR}/inc
)

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
        ${PROJECT_SOURCE_DIR}/lib/raylib/src
)

if (DISCORD_RPC)
    target_include_directories(${PROJECT_NAME} PRIVATE
            ${PROJECT_SOURCE_DIR}/lib/discordrpc/inc
    )
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${LINK_LIBS})

if (DISCORD_RPC)
    # Build discordrpc
    add_custom_target(discordrpc_build
            COMMAND make -C ${PROJECT_SOURCE_DIR}/lib/discordrpc lib
            COMMENT "Building discordrpc..."
    )

    add_dependencies(${PROJECT_NAME} discordrpc_build)
endif ()

find_program(CMAKE_ZIP_EXECUTABLE zip)

if (CMAKE_ZIP_EXECUTABLE)
    # Linux zip
    add_custom_target(zip_linux
            COMMAND ${CMAKE_ZIP_EXECUTABLE} -r ${ZIP_NAME} assets bombkurdistan
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            COMMENT "Creating Linux ZIP package"
            DEPENDS ${PROJECT_NAME}
    )

    # Windows zip
    add_custom_target(zip_windows
            COMMAND ${CMAKE_ZIP_EXECUTABLE} -r ${ZIP_NAME_WIN} assets bombkurdistan.exe
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            COMMENT "Creating Windows ZIP package"
            DEPENDS ${PROJECT_NAME}
    )

    add_custom_target(zip
            COMMENT "Creating all available ZIP packages"
            DEPENDS ${PROJECT_NAME}
    )

    if (EXISTS ${PROJECT_SOURCE_DIR}/bombkurdistan)
        add_dependencies(zip zip_linux)
    endif ()

    if (EXISTS ${PROJECT_SOURCE_DIR}/bombkurdistan.exe)
        add_dependencies(zip zip_windows)
    endif ()
endif ()
