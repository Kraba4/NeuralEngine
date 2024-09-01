# CMakeList.txt: файл проекта CMake верхнего уровня; проведите глобальную конфигурацию
# и включите сюда подпроекты.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# Включение горячей перезагрузки для компиляторов MSVC, если поддерживается.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project (NeuralEngine CXX)
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set(IMGUI_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui_draw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui_tables.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui_widgets.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui_demo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends/imgui_impl_dx12.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends/imgui_impl_glfw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/misc/cpp/imgui_stdlib.cpp)
set(IMGUIZMO_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imguizmo/Imguizmo.cpp)

add_compile_definitions(D3D12_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/d3d12")
add_compile_definitions(RESOURCES="${CMAKE_CURRENT_SOURCE_DIR}/resources")
add_compile_definitions(MODEL_DATA_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/ml_data")
# Включите подпроекты.
# add_subdirectory (${CMAKE_SOURCE_DIR}/external/assimp)
add_subdirectory ("src")
add_subdirectory ("external/assimp")
add_subdirectory ("external/directx_tool_kit")