cmake_minimum_required (VERSION 3.29.5)

if (WIN32)
	# Enable hot reload for MSVC
	if (POLICY CMP0141)
		cmake_policy(SET CMP0141 NEW)
		set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
	endif()
	# Explicitly set the MSVC toolset for Visual Studio
	if (MSVC)
		set(CMAKE_GENERATOR_TOOLSET "v143")
	endif()
endif()

# Specify the compiler for Linux builds
if (UNIX)
    set(CMAKE_C_COMPILER "/usr/bin/gcc")
    set(CMAKE_CXX_COMPILER "/usr/bin/g++")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Toggles engine mode for engine
add_definitions(-DENGINE_MODE)

# Toggles discord sdk mode for engine
# REPLACE 'remove' WITH 'add' TO USE DISCORD GAME SDK
remove_definitions(-DDISCORD_MODE)

set(PROJECT_MAJOR_VERSION 1)
set(PROJECT_MINOR_VERSION 0)
set(PROJECT_PATCH_VERSION 0)
set(PROJECT_VERSION "${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_PATCH_VERSION}")

project("Elypso engine" VERSION ${PROJECT_VERSION})

if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Paths to frequently used directories
get_filename_component(PARENT_DIR "${CMAKE_SOURCE_DIR}" DIRECTORY)
set(PARENT_DIR_PATH "${PARENT_DIR}")

# Specify resource file for Windows
if (WIN32)
    set(RESOURCE_FILE "${CMAKE_SOURCE_DIR}/files/icon.rc")
endif()

# Engine files
set(ENGINE_SRC_DIR "${CMAKE_SOURCE_DIR}/src/engine")
set(ENGINE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include/engine")

# Game files
set(GAME_SRC_DIR "${CMAKE_SOURCE_DIR}/src/game")
set(GAME_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include/game")

# External files
set(EXT_DIR "${CMAKE_SOURCE_DIR}/../_external_shared")
set(EXT_GLFW_DIR "${EXT_DIR}/GLFW")
set(EXT_ASSIMP_DIR "${EXT_DIR}/Assimp")
set(EXT_GLAD_DIR "${EXT_DIR}/Glad")
set(EXT_GLM_DIR "${EXT_DIR}/GLM")
set(EXT_IMGUI_DIR "${EXT_DIR}/ImGui")
set(EXT_MAGIC_ENUM_DIR "${EXT_DIR}/magic_enum")
set(EXT_STB_IMAGE_DIR "${EXT_DIR}/Stb_image")
set(EXT_MINIAUDIO_DIR "${EXT_DIR}/miniaudio")
set(EXT_JOLT_DIR "${EXT_DIR}/Jolt")
# UNCOMMENT TO USE DISCORD GAME SDK
# set(EXT_DISCORD_DIR "${CMAKE_SOURCE_DIR}/../_external_shared/Discord")

# Add GLAD
add_library(glad STATIC ${EXT_GLAD_DIR}/glad.c)
target_include_directories(glad PUBLIC ${EXT_GLAD_DIR}/..)

# Add GLFW
if (WIN32)
	if(CMAKE_BUILD_TYPE STREQUAL "Release")
		set(GLFW_LIBRARY_PATH "${EXT_GLFW_DIR}/release/glfw3dll.lib")
	elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(GLFW_LIBRARY_PATH "${EXT_GLFW_DIR}/debug/glfw3dll.lib")
	endif()
else()
	find_package(glfw3 REQUIRED)
endif()

# Add Assimp
if (WIN32)
	if(CMAKE_BUILD_TYPE STREQUAL "Release")
		set(ASSIMP_LIBRARY_PATH "${EXT_ASSIMP_DIR}/release/assimp-vc143-mt.lib")
		set(ASSIMP_LIBRARY_PATH2 "${EXT_ASSIMP_DIR}/release/zlibstatic.lib")
	elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(ASSIMP_LIBRARY_PATH "${EXT_ASSIMP_DIR}/debug/assimp-vc143-mtd.lib")
		set(ASSIMP_LIBRARY_PATH2 "${EXT_ASSIMP_DIR}/debug/zlibstaticd.lib")
	endif()
else()
	find_package(assimp REQUIRED)
endif()

# Add Jolt
if (WIN32)
	if(CMAKE_BUILD_TYPE STREQUAL "Release")
		set(JOLT_LIBRARY_PATH "${EXT_JOLT_DIR}/release/Jolt.lib")
	elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(JOLT_LIBRARY_PATH "${EXT_JOLT_DIR}/debug/Jolt.lib")
	endif()
else()
	if(CMAKE_BUILD_TYPE STREQUAL "Release")
		set(JOLT_LIBRARY_PATH "${EXT_JOLT_DIR}/release/libJolt.so")
	elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(JOLT_LIBRARY_PATH "${EXT_JOLT_DIR}/debug/libJolt.so")
	endif()
endif()

# Add magic_enum
add_library(magic_enum INTERFACE)
target_include_directories(magic_enum INTERFACE ${EXT_MAGIC_ENUM_DIR})

# Add discord
# UNCOMMENT TO USE DISCORD GAME SDK
# set(DISCORD_LIBRARY_PATH "${EXT_DISCORD_DIR}/discord_game_sdk.dll.lib")

# Group external libraries
add_library(external_libs INTERFACE)
target_link_libraries(external_libs INTERFACE glad magic_enum)

# Add your source files
file(GLOB_RECURSE ENGINE_SOURCE_FILES CONFIGURE_DEPENDS
	${ENGINE_SRC_DIR}/*.cpp
	${ENGINE_SRC_DIR}/*/*.cpp
)
file(GLOB_RECURSE GAME_SOURCE_FILES CONFIGURE_DEPENDS
	${GAME_SRC_DIR}/*.cpp
	${GAME_SRC_DIR}/*/*.cpp
)
set(SOURCE_FILES
	${ENGINE_SOURCE_FILES}
	${GAME_SOURCE_FILES}
)

# Add discord source files
# UNCOMMENT TO USE DISCORD GAME SDK
# file(GLOB DISCORD_SRC_FILES ${EXT_DISCORD_DIR}/src/*.cpp)

# Add external source files
list(APPEND SOURCE_FILES
	${EXT_GLAD_DIR}/glad.c
	${EXT_IMGUI_DIR}/imgui.cpp
	${EXT_IMGUI_DIR}/imgui_draw.cpp
	${EXT_IMGUI_DIR}/imgui_impl_glfw.cpp
	${EXT_IMGUI_DIR}/imgui_impl_opengl3.cpp
	${EXT_IMGUI_DIR}/imgui_tables.cpp
	${EXT_IMGUI_DIR}/imgui_widgets.cpp
	${EXT_STB_IMAGE_DIR}/stb_image.cpp
	# UNCOMMENT TO USE DISCORD GAME SDK
	# ${DISCORD_SRC_FILES}
)

# Build as engine executable
add_executable(Elypso_engine ${SOURCE_FILES})

if (WIN32)
	set_source_files_properties(${RESOURCE_FILE} PROPERTIES LANGUAGE RC)
	target_sources(Elypso_engine PRIVATE ${RESOURCE_FILE})
endif()
	
set_target_properties(Elypso_engine PROPERTIES OUTPUT_NAME "Elypso engine")

target_compile_features(Elypso_engine PRIVATE cxx_std_20)

target_include_directories(Elypso_engine PRIVATE 
    ${GAME_INCLUDE_DIR}/core
    ${ENGINE_INCLUDE_DIR}/core
    ${ENGINE_INCLUDE_DIR}/file
    ${ENGINE_INCLUDE_DIR}/graphics
	${ENGINE_INCLUDE_DIR}/graphics/components
    ${ENGINE_INCLUDE_DIR}/graphics/gui
    ${ENGINE_INCLUDE_DIR}/graphics/shapes
    ${ENGINE_INCLUDE_DIR}/physics
    ${ENGINE_INCLUDE_DIR}/utils
    ${EXT_DIR}
	${EXT_ASSIMP_DIR}
    ${EXT_GLFW_DIR}
    ${EXT_GLAD_DIR}
    ${EXT_GLM_DIR}
	${EXT_GLM_DIR}/detail
	${EXT_GLM_DIR}/ext
	${EXT_GLM_DIR}/gtc
	${EXT_GLM_DIR}/gtx
	${EXT_GLM_DIR}/simd
    ${EXT_MAGIC_ENUM_DIR}
    ${EXT_IMGUI_DIR}
    ${EXT_STB_IMAGE_DIR}
	${EXT_MINIAUDIO_DIR}
	${EXT_JOLT_DIR}
    # UNCOMMENT TO USE DISCORD GAME SDK
    # ${EXT_DISCORD_DIR}/include
)

# Apply all directories to target
target_include_directories(Elypso_engine PRIVATE ${ENGINE_INCLUDE_DIRS})

if (WIN32)
	target_link_libraries(Elypso_engine PRIVATE external_libs ${GLFW_LIBRARY_PATH} ${ASSIMP_LIBRARY_PATH} ${ASSIMP_LIBRARY_PATH2} ${JOLT_LIBRARY_PATH})
else()
	find_package(Wayland QUIET)
	find_package(X11 QUIET)
	if (Wayland_FOUND)
		target_link_libraries(Elypso_engine PRIVATE external_libs glfw assimp Wayland::Client ${JOLT_LIBRARY_PATH})
	elseif (X11_FOUND)
		target_link_libraries(Elypso_engine PRIVATE external_libs glfw assimp ${X11_LIBRARIES} ${JOLT_LIBRARY_PATH})
	endif()
endif()

# UNCOMMENT TO USE DISCORD GAME SDK
# target_link_libraries(Elypso_engine PUBLIC ${DISCORD_LIBRARY_PATH})

# Define GLFW_INCLUDE_NONE
target_compile_definitions(Elypso_engine PRIVATE GLFW_INCLUDE_NONE)

# Set the subsystem to WINDOWS
#set_target_properties(Elypso_engine PROPERTIES
#    WIN32_EXECUTABLE TRUE
#)

# Set the entry point (MSVC-specific)
#if (MSVC)
#    set_target_properties(Elypso_engine PROPERTIES
#       LINK_FLAGS "/ENTRY:mainCRTStartup"
#    )
#endif()

# Set the folder inside the install folder where the exe will be placed for this project
set(CMAKE_INSTALL_BINDIR bin)
install(TARGETS Elypso_engine DESTINATION ${CMAKE_INSTALL_BINDIR})

# Copy "files" directory to the install directory after build
add_custom_command(TARGET Elypso_engine POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E remove_directory
	$<TARGET_FILE_DIR:Elypso_engine>/files
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/files 
	$<TARGET_FILE_DIR:Elypso_engine>/files
)

# Copy Assimp DLL to the install directory after build
if (WIN32)
    set(ASSIMP_DLL_ORIGIN_BASE "${CMAKE_SOURCE_DIR}/files/external dlls")
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(ASSIMP_DLL_ORIGIN "${ASSIMP_DLL_ORIGIN_BASE}/release/assimp-vc143-mt.dll")
        set(ASSIMP_DLL_TARGET "$<TARGET_FILE_DIR:Elypso_engine>/assimp-vc143-mt.dll")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(ASSIMP_DLL_ORIGIN "${ASSIMP_DLL_ORIGIN_BASE}/debug/assimp-vc143-mtd.dll")
        set(ASSIMP_DLL_TARGET "$<TARGET_FILE_DIR:Elypso_engine>/assimp-vc143-mtd.dll")
    endif()

    add_custom_command(TARGET Elypso_engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove 
		${ASSIMP_DLL_TARGET}
        COMMAND ${CMAKE_COMMAND} -E copy 
		${ASSIMP_DLL_ORIGIN}
		${ASSIMP_DLL_TARGET}
    )
endif()

# Copy GLFW DLL to the install directory after build
if (WIN32)
    set(GLFW_DLL_ORIGIN_BASE "${CMAKE_SOURCE_DIR}/files/external dlls")
	set(GLFW_DLL_TARGET "$<TARGET_FILE_DIR:Elypso_engine>/glfw3.dll")
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(GLFW_DLL_ORIGIN "${GLFW_DLL_ORIGIN_BASE}/release/glfw3.dll")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(GLFW_DLL_ORIGIN "${GLFW_DLL_ORIGIN_BASE}/debug/glfw3.dll")
    endif()

    add_custom_command(TARGET Elypso_engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove 
		${GLFW_DLL_TARGET}
        COMMAND ${CMAKE_COMMAND} -E copy 
		${GLFW_DLL_ORIGIN}
		${GLFW_DLL_TARGET}
    )
endif()

# Copy jolt DLL to the install directory after build
set(JOLT_DLL_ORIGIN_BASE "${CMAKE_SOURCE_DIR}/files/external dlls")
if (WIN32)
	set(JOLT_DLL_TARGET "$<TARGET_FILE_DIR:Elypso_engine>/Jolt.dll")
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(JOLT_DLL_ORIGIN "${JOLT_DLL_ORIGIN_BASE}/release/Jolt.dll")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(JOLT_DLL_ORIGIN "${JOLT_DLL_ORIGIN_BASE}/debug/Jolt.dll")
    endif()
else()
	set(JOLT_DLL_TARGET "$<TARGET_FILE_DIR:Elypso_engine>/libJolt.so")
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(JOLT_DLL_ORIGIN "${JOLT_DLL_ORIGIN_BASE}/release/libJolt.so")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(JOLT_DLL_ORIGIN "${JOLT_DLL_ORIGIN_BASE}/debug/libJolt.so")
    endif()
endif()

add_custom_command(TARGET Elypso_engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove 
	${JOLT_DLL_TARGET}
    COMMAND ${CMAKE_COMMAND} -E copy 
	${JOLT_DLL_ORIGIN}
	${JOLT_DLL_TARGET}
)

# Copy discord dll to the install directory after build
# UNCOMMENT TO USE DISCORD GAME SDK
# add_custom_command(TARGET Elypso_engine POST_BUILD
	# COMMAND ${CMAKE_COMMAND} -E remove
	# $<TARGET_FILE_DIR:Elypso_engine>/discord_game_sdk.dll
	# COMMAND ${CMAKE_COMMAND} -E copy
	# ${CMAKE_SOURCE_DIR}/files/external\ dlls/discord_game_sdk.dll
	# $<TARGET_FILE_DIR:Elypso_engine>/discord_game_sdk.dll
# )

set(COPY_TARGETS LICENSE.md README.md SECURITY.md LIBRARIES.md)
foreach(FILE ${COPY_TARGETS})
  add_custom_command(TARGET Elypso_engine POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E remove
	$<TARGET_FILE_DIR:Elypso_engine>/${FILE}
	COMMAND ${CMAKE_COMMAND} -E copy
	${PARENT_DIR_PATH}/${FILE}
	$<TARGET_FILE_DIR:Elypso_engine>/${FILE}
  )
endforeach()

# Include CPack to enable packaging
include(CPack)
