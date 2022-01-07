# Nacos
# Use vcpkg and Visual Studio 2019
CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(nacos)

# Nacos
add_library(nacos SHARED "src/Nacos.cpp" "src/NacosService.cpp" "src/NacosImpl.cpp" "src/NacosImpl.h" "src/NacosInstance.cpp")

target_include_directories(nacos PRIVATE ${CMAKE_SOURCE_DIR})

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(nacos PRIVATE nlohmann_json nlohmann_json::nlohmann_json)

find_package(CURL CONFIG REQUIRED)
target_link_libraries(nacos PRIVATE CURL::libcurl)

