include_directories(${CMAKE_SOURCE_DIR}/external/glfw/include)
include_directories(${CMAKE_SOURCE_DIR}/external)
include_directories(${CMAKE_SOURCE_DIR}/external/assimp)
include_directories(${CMAKE_SOURCE_DIR}/src)
link_directories(${CMAKE_SOURCE_DIR}/external/glfw)

add_executable(neural "a_main/main.cpp"  "graphics/d3d12/DX12Initialize.cpp" "graphics/IRenderEngine.h" "graphics/d3d12/DX12RenderEngine.h" "graphics/d3d12/DX12RenderApplication.cpp" "a_main/Application.h" "a_main/Application.cpp" "game/Game.h" "game/ProcessInputs.cpp" "a_main/Timer.h" "a_main/Timer.cpp" "utils/Macros.h" "graphics/d3d12/GraphicsPipeline.h"  "utils/Utils.h" "utils/Utils.cpp" "graphics/d3d12/RootSignature.h"  "graphics/d3d12/RootSignature.cpp" "graphics/d3d12/GraphicsPipeline.cpp" "graphics/d3d12/CommonGraphicsHeaders.h" "graphics/d3d12/DescriptorHeap.cpp"    "graphics/d3d12/Resource.cpp" "graphics/d3d12/Resource.h" "a_main/Camera.h" "a_main/Camera.cpp" "a_main/AppCallbacks.cpp" "a_main/AppInput.h" "graphics/RenderSettings.h" "graphics/d3d12/SceneManager.h" "graphics/d3d12/SceneManager.cpp")

set_target_properties(neural PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

target_link_libraries(neural PRIVATE glfw3)
target_link_libraries(neural PRIVATE d3d12)
target_link_libraries(neural PRIVATE dxgi)
target_link_libraries(neural PRIVATE assimp)