cmake_minimum_required(VERSION 3.19)

# MSVC runtime: DLL in both Debug and Release
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

project(MyVulkanEngine VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# — third-party directories —
set(ENGINE_EXTERNAL_DIR		"${PROJECT_SOURCE_DIR}/engine/external")
set(GLFW_DIR				"${ENGINE_EXTERNAL_DIR}/glfw-3.4.bin.WIN64")
set(GLFW_INCLUDE			"${GLFW_DIR}/include")
set(GLFW_LIB_DIR			"${GLFW_DIR}/lib-vc2022")
set(VULKAN_SDK_ROOT			"${ENGINE_EXTERNAL_DIR}/VulkanSDK/1.4.309.0")
set(VULKAN_INCLUDE			"${VULKAN_SDK_ROOT}/Include")
set(VULKAN_LIB_DIR			"${VULKAN_SDK_ROOT}/Lib")
set(TASKFLOW_INCLUDE        "${ENGINE_EXTERNAL_DIR}/taskflow-3.10.0")
set(KTX_DIR					"${ENGINE_EXTERNAL_DIR}/KTX-Software")
set(KTX_INCLUDE_DIR			"${KTX_DIR}/include")
set(KTX_LIB_DIR				"${KTX_DIR}/lib")
set(VMA_INCLUDE_DIR			"${ENGINE_EXTERNAL_DIR}/VMA")

# — gather files —
file(GLOB_RECURSE SRC_FILES
	"${PROJECT_SOURCE_DIR}/engine/src/*.cpp"
	"${PROJECT_SOURCE_DIR}/game/src/*.cpp"
)
file(GLOB_RECURSE HEADER_FILES
	"${PROJECT_SOURCE_DIR}/engine/include/*.h"
	"${PROJECT_SOURCE_DIR}/game/include/*.h"
	"${PROJECT_SOURCE_DIR}/engine/include/*.hpp"
	"${PROJECT_SOURCE_DIR}/game/include/*.hpp"
)

# — build target —
add_executable(${PROJECT_NAME}
	${SRC_FILES}
	${HEADER_FILES}
)

# - global definitions -
target_compile_definitions(${PROJECT_NAME} PRIVATE
    GLFW_DLL
    PROJECT_ROOT_DIR=\"${CMAKE_SOURCE_DIR}/\"
)

# — includes —
target_include_directories(${PROJECT_NAME} PRIVATE
	"${PROJECT_SOURCE_DIR}/engine/include"
	"${PROJECT_SOURCE_DIR}/game/include"
	${GLFW_INCLUDE}
	${VULKAN_INCLUDE}
	${TASKFLOW_INCLUDE}
	${KTX_INCLUDE_DIR}
	"${ENGINE_EXTERNAL_DIR}/stb_image"
	${VMA_INCLUDE_DIR}
)

# — link libs —
target_link_libraries(${PROJECT_NAME} PRIVATE
	"${GLFW_LIB_DIR}/glfw3dll.lib"
	opengl32
	"${VULKAN_LIB_DIR}/vulkan-1.lib"
	"${KTX_LIB_DIR}/ktx.lib"
)

# - execute command when build -
if (WIN32)
  add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
	COMMENT     "Auto-compile GLSL to SPIR-V shaders"
    COMMAND "${PROJECT_SOURCE_DIR}/shaders/build_spirv.bat"
  )
endif()


# Command List in PROJECT_SOURCE_DIR/build

# Generate (configure + VS solution)
# cmake -G "Visual Studio 17 2022" -A x64 ..

# Re-generate (only if you change CMakeLists.txt)
# cmake ..

# Build (compile+link; uses Debug by default)
# cmake --build . --config Debug

# Run Debug binary
# .\Debug\MyVulkanEngine.exe