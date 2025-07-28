cmake_minimum_required(VERSION 3.16.0)
target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++11)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(nekos)
