project(fridaynight)

cmake_minimum_required(VERSION 3.1)

# Include local settings.
include(cmake_user.txt)

# Include the engine.
include(${ENGINE_PATH}/CMakeLists.txt)

# Build the engine.
add_game_library(fridaynight main.c tilemap.c tilemap.h tilemap_render.c tilemap_render.h room.c room.h)
