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

# Toggles engine mode for game
remove_definitions(-DENGINE_MODE)

# Toggles discord mode for game
# REPLACE 'remove' WITH 'add' TO USE DISCORD GAME SDK
remove_definitions(-DDISCORD_MODE)

project("Game")

# Set parent path
get_filename_component(PARENT_DIR "${CMAKE_SOURCE_DIR}" DIRECTORY)
set(PARENT_DIR_PATH "${PARENT_DIR}")

# Specify resource file for Windows
if (WIN32)
    set(RESOURCE_FILE "${CMAKE_SOURCE_DIR}/files/icon.rc")
endif()

# Set src and include file paths
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

set(ENGINE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../Engine/include/engine")
set(GAME_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../Engine/include/game")

# External files
set(EXT_ASSIMP_DIR "${CMAKE_SOURCE_DIR}/../_external_shared/Assimp")
set(EXT_GLAD_DIR "${CMAKE_SOURCE_DIR}/../_external_shared/Glad")
set(EXT_GLFW_DIR "${CMAKE_SOURCE_DIR}/../_external_shared/GLFW")
set(EXT_GLM_DIR "${CMAKE_SOURCE_DIR}/../_external_shared/GLM")
set(EXT_IMGUI_DIR "${CMAKE_SOURCE_DIR}/../_external_shared/ImGui")
set(EXT_MAGIC_ENUM_DIR "${CMAKE_SOURCE_DIR}/../_external_shared/magic_enum")
set(EXT_STB_IMAGE_DIR "${CMAKE_SOURCE_DIR}/../_external_shared/Stb_image")
set(EXT_MINIAUDIO_DIR "${CMAKE_SOURCE_DIR}/../_external_shared/miniaudio")
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

# Add x11 (linux only)
if (UNIX)
	find_package(X11 REQUIRED)
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
file(GLOB_RECURSE SOURCE_FILES
	${SRC_DIR}/*.cpp
	${SRC_DIR}/*/*.cpp
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

# Build as game executable
add_executable(Game ${SOURCE_FILES})

if (WIN32)
	set_source_files_properties(${RESOURCE_FILE} PROPERTIES LANGUAGE RC)
	target_sources(Game PRIVATE ${RESOURCE_FILE})
endif()

set_target_properties(Game PROPERTIES OUTPUT_NAME "Game")
target_compile_features(Game PRIVATE cxx_std_20)

target_include_directories(Game PRIVATE
	# Code added by user
	${INCLUDE_DIR}
	
	# Engine code for game
	${GAME_INCLUDE_DIR}/core
	
	# Engine code
	${ENGINE_INCLUDE_DIR}/core
	${ENGINE_INCLUDE_DIR}/file
	${ENGINE_INCLUDE_DIR}/graphics
	${ENGINE_INCLUDE_DIR}/graphics/components
	${ENGINE_INCLUDE_DIR}/graphics/gui
	${ENGINE_INCLUDE_DIR}/graphics/shapes
	${ENGINE_INCLUDE_DIR}/physics
	${ENGINE_INCLUDE_DIR}/utils
	${ENGINE_INCLUDE_DIR}/game
	
	# External libraries
	${EXT_ASSIMP_DIR}
	${EXT_GLAD_DIR}
	${EXT_GLFW_DIR}
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
	# UNCOMMENT TO USE DISCORD GAME SDK
	# ${EXT_DISCORD_DIR}/include
)

if (WIN32)
	if(CMAKE_BUILD_TYPE STREQUAL "Release")
		target_link_libraries(Game PRIVATE external_libs ${GLFW_LIBRARY_PATH} ${ASSIMP_LIBRARY_PATH} ${ASSIMP_LIBRARY_PATH2} "${CMAKE_SOURCE_DIR}/Elypso engine.lib")
	elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_link_libraries(Game PRIVATE external_libs ${GLFW_LIBRARY_PATH} ${ASSIMP_LIBRARY_PATH} ${ASSIMP_LIBRARY_PATH2} "${CMAKE_SOURCE_DIR}/Elypso engineD.lib")
	endif()
else()
	if(CMAKE_BUILD_TYPE STREQUAL "Release")
		target_link_libraries(Game PRIVATE external_libs glfw assimp ${X11_LIBRARIES} "${CMAKE_SOURCE_DIR}/libElypso engine.a")
	elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_link_libraries(Game PRIVATE external_libs glfw assimp ${X11_LIBRARIES} "${CMAKE_SOURCE_DIR}/libElypso engineD.a")
	endif()
endif()

# UNCOMMENT TO USE DISCORD GAME SDK
# target_link_libraries(Game PUBLIC ${DISCORD_LIBRARY_PATH})

# Define GLFW_INCLUDE_NONE
target_compile_definitions(Game PRIVATE GLFW_INCLUDE_NONE)

# Set the folder inside the install folder where the exe will be placed for this project
set(CMAKE_INSTALL_BINDIR bin)
install(TARGETS Game DESTINATION ${CMAKE_INSTALL_BINDIR})

# Copy "files" directory to the install directory after build
add_custom_command(TARGET Game POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E remove_directory
	$<TARGET_FILE_DIR:Game>/files
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/files 
	$<TARGET_FILE_DIR:Game>/files
)

# Copy Assimp DLL to the install directory after build
if (WIN32)
    set(ASSIMP_DLL_ORIGIN_BASE "${CMAKE_SOURCE_DIR}/files/external dlls")
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(ASSIMP_DLL_ORIGIN "${ASSIMP_DLL_ORIGIN_BASE}/release/assimp-vc143-mt.dll")
        set(ASSIMP_DLL_TARGET "$<TARGET_FILE_DIR:Game>/assimp-vc143-mt.dll")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(ASSIMP_DLL_ORIGIN "${ASSIMP_DLL_ORIGIN_BASE}/debug/assimp-vc143-mtd.dll")
        set(ASSIMP_DLL_TARGET "$<TARGET_FILE_DIR:Game>/assimp-vc143-mtd.dll")
    endif()

    add_custom_command(TARGET Game POST_BUILD
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
	set(GLFW_DLL_TARGET "$<TARGET_FILE_DIR:Game>/glfw3.dll")
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(GLFW_DLL_ORIGIN "${GLFW_DLL_ORIGIN_BASE}/release/glfw3.dll")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(GLFW_DLL_ORIGIN "${GLFW_DLL_ORIGIN_BASE}/debug/glfw3.dll")
    endif()

    add_custom_command(TARGET Game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove 
		${GLFW_DLL_TARGET}
        COMMAND ${CMAKE_COMMAND} -E copy 
		${GLFW_DLL_ORIGIN}
		${GLFW_DLL_TARGET}
    )
endif()

# Copy discord dll to the install directory after build
# UNCOMMENT TO USE DISCORD GAME SDK
# add_custom_command(TARGET Game POST_BUILD
	# COMMAND ${CMAKE_COMMAND} -E remove
	# $<TARGET_FILE_DIR:Game>/discord_game_sdk.dll
	# COMMAND ${CMAKE_COMMAND} -E copy
	# ${CMAKE_SOURCE_DIR}/files/external\ dlls/discord_game_sdk.dll
	# $<TARGET_FILE_DIR:Game>/discord_game_sdk.dll
# )

set(COPY_TARGETS README.md LIBRARIES.md)
foreach(FILE ${COPY_TARGETS})
  add_custom_command(TARGET Game POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E remove
	$<TARGET_FILE_DIR:Game>/${FILE}
	COMMAND ${CMAKE_COMMAND} -E copy
	${PARENT_DIR_PATH}/${FILE}
	$<TARGET_FILE_DIR:Game>/${FILE}
  )
endforeach()

# Copy the MIT license from this folder, not parent folder
add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove
    $<TARGET_FILE_DIR:Game>/LICENSE.md
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/LICENSE.md
    $<TARGET_FILE_DIR:Game>/LICENSE.md
)

# Include CPack to enable packaging
include(CPack)