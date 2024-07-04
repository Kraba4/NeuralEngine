include_directories(${CMAKE_SOURCE_DIR}/external/glfw/include)
include_directories(${CMAKE_SOURCE_DIR}/src)
link_directories(${CMAKE_SOURCE_DIR}/external/glfw)

add_executable(neural "a_main/main.cpp"  "graphics/IRenderEngine.h" "graphics/d3d12/DX12RenderEngine.h" "graphics/d3d12/DX12RenderEngine.cpp" "a_main/Application.h" "a_main/Application.cpp" "game/Game.h" "game/Game.cpp" "a_main/Timer.h" "a_main/Timer.cpp" "utils/Macros.h")

set_target_properties(neural PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

target_link_libraries(neural PRIVATE glfw3)
target_link_libraries(neural PRIVATE d3d12)
target_link_libraries(neural PRIVATE dxgi)