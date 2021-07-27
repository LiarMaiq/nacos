# Nacos
CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(Nacos)

# Nacos
add_library(Nacos SHARED Nacos.cpp MicroService.cpp)
target_link_libraries(Nacos cpprest ssl crypto)
