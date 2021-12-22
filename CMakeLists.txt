# Nacos
# Use vcpkg and Visual Studio 2019
CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(nacos)

# Nacos
add_library(nacos SHARED Nacos.cpp "Service.cpp" "NacosImpl.cpp" "NacosImpl.h")

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(nacos PRIVATE nlohmann_json nlohmann_json::nlohmann_json)

find_package(CURL CONFIG REQUIRED)
target_link_libraries(nacos PRIVATE CURL::libcurl)

