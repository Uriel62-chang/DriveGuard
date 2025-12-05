cmake_minimum_required(VERSION 3.10)
project(DriveGuard VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++ 标准为 C++17 (符合现代C++要求)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 OpenCV 包
# 注意：在 Windows 下使用 MinGW 编译时，可能需要设置 OpenCV_DIR 环境变量
# 或者在 cmake 命令中指定 -DOpenCV_DIR="path/to/opencv/build"
# set(OpenCV_DIR "E:\\OpenCV4.10.0\\build_mingw")
find_package(OpenCV 4.10.0 REQUIRED)

# 收集源文件
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 指定可执行文件输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# 创建可执行文件
add_executable(DriveGuard ${SOURCES})

# 包含头文件目录
target_include_directories( DriveGuard PUBLIC 
    ${PROJECT_SOURCE_DIR}/include
)

# 链接 OpenCV 库
target_link_libraries(DriveGuard PRIVATE
    ${OpenCV_LIBS})