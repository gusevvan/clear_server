cmake_minimum_required(VERSION 3.26)
project(boost_tests VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME} clear_server.cpp)

target_link_libraries(${PROJECT_NAME} Boost::boost OpenSSL::SSL OpenSSL::Crypto)
