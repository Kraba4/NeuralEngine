include_directories(${CMAKE_SOURCE_DIR}/external/glfw/include)
include_directories(${CMAKE_SOURCE_DIR}/external)
include_directories(${CMAKE_SOURCE_DIR}/external/assimp)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/external/imgui)
include_directories(${CMAKE_SOURCE_DIR}/external/imguizmo)
include_directories(${CMAKE_SOURCE_DIR}/external)

link_directories(${CMAKE_SOURCE_DIR}/external/glfw)

set(NEURAL_SRC
        ${CMAKE_SOURCE_DIR}/src/a_main/main.cpp
        ${CMAKE_SOURCE_DIR}/src/a_main/Application.cpp
        ${CMAKE_SOURCE_DIR}/src/a_main/Timer.cpp
        ${CMAKE_SOURCE_DIR}/src/a_main/Camera.cpp
        ${CMAKE_SOURCE_DIR}/src/a_main/AppCallbacks.cpp

        ${CMAKE_SOURCE_DIR}/src/game/ProcessInputs.cpp

        ${CMAKE_SOURCE_DIR}/src/utils/Utils.cpp

        ${CMAKE_SOURCE_DIR}/src/graphics/d3d12/DX12Initialize.cpp
        ${CMAKE_SOURCE_DIR}/src/graphics/d3d12/DX12RenderApplication.cpp
        ${CMAKE_SOURCE_DIR}/src/graphics/d3d12/DX12Gui.cpp
        ${CMAKE_SOURCE_DIR}/src/graphics/d3d12/classes/ComputePipeline.cpp
        ${CMAKE_SOURCE_DIR}/src/graphics/d3d12/classes/RootSignature.cpp
        ${CMAKE_SOURCE_DIR}/src/graphics/d3d12/classes/GraphicsPipeline.cpp
        ${CMAKE_SOURCE_DIR}/src/graphics/d3d12/classes/DescriptorHeap.cpp
        ${CMAKE_SOURCE_DIR}/src/graphics/d3d12/classes/SceneManager.cpp
        ${CMAKE_SOURCE_DIR}/src/graphics/d3d12/classes/ResourceManager.cpp
        ${CMAKE_SOURCE_DIR}/src/graphics/d3d12/classes/resource/BufferAndTexture.cpp
        ${CMAKE_SOURCE_DIR}/src/graphics/d3d12/classes/resource/ConstantBuffer.cpp
        ${CMAKE_SOURCE_DIR}/src/graphics/d3d12/classes/ml/ConvolutionLayer.cpp
        ${CMAKE_SOURCE_DIR}/src/graphics/d3d12/classes/ml/ConvolutionLayersContainer.cpp
        ${CMAKE_SOURCE_DIR}/src/graphics/d3d12/classes/ml/Model.cpp
        )

add_executable(neural ${IMGUI_SRC} ${IMGUIZMO_SRC}  ${NEURAL_SRC})
if(MSVC)
  add_compile_options($<$<CONFIG:Release>:/MT> # Runtime library: Multi-threaded
                      $<$<CONFIG:RelWithDebInfo>:/MT> # Runtime library: Multi-threaded                           
                      $<$<CONFIG:Debug>:/MTd> # Runtime library: Multi-threaded Debug
                      )
endif()
set_target_properties(neural PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

target_link_libraries(neural PRIVATE glfw3)
target_link_libraries(neural PRIVATE d3d12)
target_link_libraries(neural PRIVATE dxgi)
target_link_libraries(neural PRIVATE assimp)
target_link_libraries(neural PRIVATE DirectXTK12)
target_link_libraries(neural PRIVATE DirectML)
# add_custom_command(
#     TARGET neural POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:neural> $<TARGET_FILE_DIR:neural>
#     COMMAND_EXPAND_LISTS
# )
