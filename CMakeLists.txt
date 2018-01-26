cmake_minimum_required(VERSION 2.8.4)
project(ProjectZ)

option(RN_BUILD_BULLET_MODULE "Build the Bullet physics module" OFF)
option(RN_BUILD_PHYSX_MODULE "Build the PhysX physics module" ON)
option(RN_BUILD_ASSIMP_MODULE "Build the Assimp module" OFF)
option(RN_BUILD_STEAMAUDIO_MODULE "Build the Steam Audio module" ON)
option(RN_BUILD_OPENAL_MODULE "Build the OpenAL module" OFF)
option(RN_BUILD_ENET_MODULE "Build the ENet module" OFF)
option(RN_BUILD_RECAST_MODULE "Build the Recast pathfinding module" ON)
option(RN_BUILD_SAMPLES "Enable building the sample projects" OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/Build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/Build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/Build)

set(CMAKE_OSX_DEPLOYMENT_TARGET 10.12)

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH @executable_path;@executable_path/Resources/Modules/RaynePhysX;@executable_path/Resources/Modules/RayneMetal;@executable_path/Resources/Modules/RayneOgg;@executable_path/Resources/Modules/RayneSteamAudio;@executable_path/Resources/Modules/RayneRecast;@executable_path/Resources/Modules/RayneOpenVR;@executable_path/Resources/Modules/RayneVR)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../../Rayne" "${CMAKE_CURRENT_BINARY_DIR}/Rayne")

include(${CMAKE_CURRENT_SOURCE_DIR}/../../Rayne/CMake/CopyResources.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../Rayne/CMake/Rayne.cmake)

set(SOURCES
	Sources/PZApplication.cpp
	Sources/PZWorld.cpp
	Sources/PZPlayer.cpp
	Sources/main.cpp)

set(HEADERS
	Sources/PZApplication.h
	Sources/PZWorld.h
	Sources/PZPlayer.h)

set(RESOURCES
	manifest.json
	Resources)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

add_executable(ProjectZ "${SOURCES}" "${HEADERS}" "${PROTOSOURCES}")

rayne_link_with(ProjectZ)
rayne_use_modules(ProjectZ RaynePhysX)
rayne_use_modules(ProjectZ RayneVR)
rayne_use_modules(ProjectZ RayneSteamAudio)
rayne_use_modules(ProjectZ RayneOgg)
rayne_use_modules(ProjectZ RayneRecast)

rayne_use_modules(ProjectZ RayneOpenVR)
add_custom_command(TARGET ProjectZ POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Other/RunOculus.bat $<TARGET_FILE_DIR:ProjectZ>/RunOculus.bat)
add_custom_command(TARGET ProjectZ POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Other/RunOpenVR.bat $<TARGET_FILE_DIR:ProjectZ>/RunOpenVR.bat)

if(NOT APPLE)
    rayne_use_modules(ProjectZ RayneOculus)
    rayne_use_modules(ProjectZ RayneD3D12)
else()
    rayne_use_modules(ProjectZ RayneMetal)
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ProjectZ)

rayne_copy_resources(ProjectZ "${RESOURCES}")
if(NOT APPLE)
add_custom_command(TARGET ProjectZ POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE_DIR:RayneSteamAudio>/phonon.dll" "$<TARGET_FILE_DIR:ProjectZ>/phonon.dll")
endif()
