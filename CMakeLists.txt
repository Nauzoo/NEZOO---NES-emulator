cmake_minimum_required(VERSION 3.16...3.28 FATAL_ERROR) # Compatible with GLFW

project(NESoo_Emulator LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# --- GLFW ---
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

# --- GLAD ---
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
)
FetchContent_MakeAvailable(glad)

# --- ImGui ---
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.0   # or latest stable version
)
FetchContent_MakeAvailable(imgui)

# Create ImGui source group manually since it’s not a CMake project
add_library(imgui_lib
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui_lib PUBLIC glfw OpenGL::GL)

# --- INCLUDE DIRECTORIES ----
add_library(project_headers INTERFACE)
target_include_directories(project_headers INTERFACE ${CMAKE_SOURCE_DIR}/includes)

add_subdirectory(includes/nz6502)
add_subdirectory(includes/nzBUS)

# --- Executable ---
add_executable(${PROJECT_NAME} src/main.c)

find_package(OpenGL REQUIRED)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        glfw
        OpenGL::GL
        imgui_lib
        nz6502
        nzBUS
        project_headers
)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE winmm)
endif()
