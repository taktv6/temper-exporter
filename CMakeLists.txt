cmake_minimum_required(VERSION 3.14)

project(Temper++)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Microsoft.GSL CONFIG)
find_package(prometheus-cpp CONFIG REQUIRED)
find_package(Boost REQUIRED)

add_executable(temper++ main.cpp temper.cpp)
target_link_libraries(temper++ PRIVATE usb Microsoft.GSL::GSL prometheus-cpp::core prometheus-cpp::pull)
