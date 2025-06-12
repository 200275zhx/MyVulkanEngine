cmake_minimum_required(VERSION 3.19)
project(MyVulkanEngine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# — third-party roots —
set(ENGINE_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/engine/external")
set(GLFW_DIR        "${ENGINE_EXTERNAL_DIR}/glfw-3.4.bin.WIN64")
set(GLFW_INCLUDE    "${GLFW_DIR}/include")
set(GLFW_LIB_DIR    "${GLFW_DIR}/lib-vc2022")
set(VULKAN_SDK_ROOT "${ENGINE_EXTERNAL_DIR}/VulkanSDK/1.4.309.0")
set(VULKAN_INCLUDE  "${VULKAN_SDK_ROOT}/Include")
set(VULKAN_LIB_DIR  "${VULKAN_SDK_ROOT}/Lib")

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

# — includes —
target_include_directories(${PROJECT_NAME} PRIVATE
	"${PROJECT_SOURCE_DIR}/engine/include"
	"${PROJECT_SOURCE_DIR}/game/include"
	${GLFW_INCLUDE}
	${VULKAN_INCLUDE}
)

# — link libs —
target_link_libraries(${PROJECT_NAME} PRIVATE
	"${GLFW_LIB_DIR}/glfw3.lib"
	opengl32
	"${VULKAN_LIB_DIR}/vulkan-1.lib"
)


# Command List in PROJECT_SOURCE_DIR/build

# Generate (configure + VS solution)
# cmake -G "Visual Studio 17 2022" -A x64 ..

# Re-generate (only if you change CMakeLists.txt)
# cmake ..

# Build (compile+link; uses Debug by default)
# cmake --build . --config Debug

# Run Debug binary
# .\Debug\MyVulkanEngine.exe