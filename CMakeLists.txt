cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0091 NEW)
project(HeliosEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
  # Runtime estático: el .exe no exige instalar el redistribuible de Visual C++.
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  add_compile_options(/utf-8 /W3)
endif()

option(HELIOS_CONSTRUIR_JUEGO "Compilar el ejecutable gráfico (descarga SDL2)" ON)

# Simulación pura (sin gráficos): se prueba de forma automática.
add_library(helios_simulacion STATIC juego/jugador.cpp juego/escena.cpp)
target_include_directories(helios_simulacion PUBLIC ${CMAKE_SOURCE_DIR})

add_executable(helios_pruebas tests/unit/pruebas_simulacion.cpp)
target_link_libraries(helios_pruebas PRIVATE helios_simulacion)
enable_testing()
add_test(NAME simulacion COMMAND helios_pruebas)

if(HELIOS_CONSTRUIR_JUEGO)
  include(FetchContent)
  set(SDL_SHARED OFF CACHE BOOL "" FORCE)
  set(SDL_STATIC ON CACHE BOOL "" FORCE)
  set(SDL_TEST OFF CACHE BOOL "" FORCE)
  set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.30.8
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(SDL2)
  find_package(OpenGL REQUIRED)

  add_executable(helios_juego juego/principal.cpp)
  target_link_libraries(helios_juego PRIVATE helios_simulacion SDL2::SDL2main SDL2::SDL2-static OpenGL::GL)
endif()
