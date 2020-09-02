cmake_minimum_required(VERSION 3.16)
project(raspberry_bmp280)

set(CMAKE_CXX_STANDARD 14)

add_library(bosch_bmp280 STATIC lib/BMP280_driver/bmp280.c)
target_include_directories(bosch_bmp280 PUBLIC lib/BMP280_driver)

add_library(raspberry_bmp280 STATIC src/raspberry_bmp280.cpp)
target_include_directories(raspberry_bmp280 PUBLIC include)
target_link_libraries(raspberry_bmp280 PUBLIC bosch_bmp280)

add_executable(example_basic examples/basic.cpp)
target_link_libraries(example_basic PUBLIC raspberry_bmp280)