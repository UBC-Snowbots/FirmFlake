cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(esp_cbs LANGUAGES C CXX)
# set(CMAKE_C_COMPILER g++)
# set(CMAKE_CXX_COMPILER g++)
# set(CMAKE_ASM_COMPILER g++)
# include($ENV{IDF_PATH}/tools/cmake/project.cmake)
#telling it what targets to aim for
target_sources(app PRIVATE src/main.c)
