cmake_minimum_required(VERSION 3.14)
project(Voxel65cpp)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "Minimum policy version" FORCE)

if (EMSCRIPTEN)
    set(PLATFORM "Web" CACHE STRING "Target platform" FORCE)
    set(GRAPHICS "GRAPHICS_API_OPENGL_ES3" CACHE STRING "Graphics API" FORCE)
endif()

find_package(raylib QUIET)
if (NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        raylib 
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
        GIT_SHALLOW 1
    )
    FetchContent_MakeAvailable(raylib)
endif()

find_package(raylib_cpp QUIET)
if (NOT raylib_cpp_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        raylib_cpp
        GIT_REPOSITORY https://github.com/RobLoach/raylib-cpp.git
        GIT_TAG v5.5.1
        GIT_SHALLOW 1
    )
    FetchContent_MakeAvailable(raylib_cpp)
endif()

set(SOURCES
    src/Chunk.cpp
    src/Voxel65.cpp
    src/World.cpp
)

set(HEADERS
    src/Block.hpp
    src/Chunk.hpp
    src/OpenSimplexNoise.hpp
    src/Settings.hpp
    src/Terrain.hpp
    src/Utils.hpp
    src/Voxel65.hpp
    src/World.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

if (EMSCRIPTEN)
    target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_WEB)
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
    target_link_options(${PROJECT_NAME} PRIVATE
        "-sUSE_GLFW=3"
        "-sASYNCIFY"
        "-sALLOW_MEMORY_GROWTH=1"
        "--preload-file" "${CMAKE_SOURCE_DIR}/resources@resources"
    )
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PUBLIC raylib raylib_cpp)


add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
    COMMENT "Syncing game resources to output directory"
)
