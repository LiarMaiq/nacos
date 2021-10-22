# Nacos
# Use vcpkg and Visual Studio 2019
CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(Nacos)

find_package(cpprestsdk CONFIG REQUIRED)

# Nacos
add_library(Nacos SHARED Nacos.cpp MicroService.cpp)
if(WIN32)
	target_link_libraries(Nacos PRIVATE cpprestsdk::cpprest cpprestsdk::cpprestsdk_zlib_internal cpprestsdk::cpprestsdk_brotli_internal)
else()
	target_link_libraries(Nacos PRIVATE cpprestsdk::cpprest cpprestsdk::cpprestsdk_zlib_internal cpprestsdk::cpprestsdk_boost_internal cpprestsdk::cpprestsdk_openssl_internal)
endif()

