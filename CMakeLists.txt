cmake_minimum_required(VERSION 3.20)

# ---------------------------------------------------------------------------
# ARCADE RESCUE - build principal
#
# El proyecto se puede compilar de tres formas:
#   1. Con raylib instalada en el sistema   -> -DCMAKE_PREFIX_PATH=/ruta/raylib
#   2. Con raylib descargada por FetchContent (por defecto, requiere red)
#   3. Solo consola / headless              -> -DARCADE_RESCUE_WITH_RAYLIB=OFF
# ---------------------------------------------------------------------------

project(ArcadeRescue VERSION 1.0.0 LANGUAGES CXX)

option(ARCADE_RESCUE_WITH_RAYLIB "Compilar el frontend grafico con raylib" ON)
option(ARCADE_RESCUE_FETCH_RAYLIB "Descargar raylib con FetchContent si no esta instalada" ON)
option(ARCADE_RESCUE_WERROR "Tratar los warnings del compilador como errores" OFF)
option(ARCADE_RESCUE_SANITIZERS "Activar AddressSanitizer/UBSan (GCC/Clang)" OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Tipo de build" FORCE)
endif()

# --- Version en un unico lugar: el fichero VERSION -------------------------
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" ARCADE_RESCUE_VERSION_RAW)
string(STRIP "${ARCADE_RESCUE_VERSION_RAW}" ARCADE_RESCUE_VERSION)

set(ARCADE_RESCUE_GIT_SHA "unknown")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
  find_package(Git QUIET)
  if(Git_FOUND)
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      OUTPUT_VARIABLE ARCADE_RESCUE_GIT_SHA
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET)
  endif()
endif()

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.hpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/generated/arcade/version.hpp"
  @ONLY)

# --- Warnings ---------------------------------------------------------------
if(MSVC)
  set(ARCADE_RESCUE_WARNINGS /W4 /permissive- /utf-8 /EHsc)
  if(ARCADE_RESCUE_WERROR)
    list(APPEND ARCADE_RESCUE_WARNINGS /WX)
  endif()
else()
  set(ARCADE_RESCUE_WARNINGS -Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor)
  if(ARCADE_RESCUE_WERROR)
    list(APPEND ARCADE_RESCUE_WARNINGS -Werror)
  endif()
endif()

if(ARCADE_RESCUE_SANITIZERS AND NOT MSVC)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

# --- Nucleo: logica de juego sin dependencias graficas ----------------------
add_library(arcade_core STATIC
  src/game.cpp
  src/rng.cpp
  src/app.cpp
  src/assets.cpp
  src/pilot.cpp)

target_include_directories(arcade_core
  PUBLIC include
  PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/generated")

target_compile_options(arcade_core PRIVATE ${ARCADE_RESCUE_WARNINGS})

# --- Frontend grafico opcional ---------------------------------------------
set(ARCADE_RESCUE_RAYLIB_AVAILABLE OFF)

if(ARCADE_RESCUE_WITH_RAYLIB)
  find_package(raylib 4.5 QUIET)
  if(NOT raylib_FOUND AND ARCADE_RESCUE_FETCH_RAYLIB)
    message(STATUS "raylib no encontrada: se descargara con FetchContent")
    include(FetchContent)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
      raylib
      GIT_REPOSITORY https://github.com/raysan5/raylib.git
      GIT_TAG 6.2
      GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(raylib)
  endif()

  if(TARGET raylib)
    set(ARCADE_RESCUE_RAYLIB_AVAILABLE ON)
    message(STATUS "raylib disponible: frontend grafico activado")
  else()
    message(WARNING "raylib no disponible: se compilara solo el frontend de consola")
  endif()
endif()

# --- Ejecutable -------------------------------------------------------------
add_executable(arcade_rescue
  src/main.cpp
  src/frontend_console.cpp
  src/frontend_raylib.cpp)

target_link_libraries(arcade_rescue PRIVATE arcade_core)
target_compile_options(arcade_rescue PRIVATE ${ARCADE_RESCUE_WARNINGS})
target_compile_definitions(arcade_rescue PRIVATE
  $<$<BOOL:${ARCADE_RESCUE_RAYLIB_AVAILABLE}>:ARCADE_RESCUE_WITH_RAYLIB=1>)

if(ARCADE_RESCUE_RAYLIB_AVAILABLE)
  target_link_libraries(arcade_rescue PRIVATE raylib)
endif()

# --- Instalacion / empaquetado ---------------------------------------------
include(GNUInstallDirs)

install(TARGETS arcade_rescue RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY assets/ DESTINATION ${CMAKE_INSTALL_DATADIR}/arcade-rescue/assets)

set(CPACK_PACKAGE_NAME "arcade-rescue")
set(CPACK_PACKAGE_VENDOR "Arcade Rescue Studio")
set(CPACK_PACKAGE_VERSION "${ARCADE_RESCUE_VERSION}")
set(CPACK_PACKAGE_FILE_NAME
  "arcade-rescue-${ARCADE_RESCUE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Snake retro con pipeline DevOps")
include(CPack)
