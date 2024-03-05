# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)                # 指定运行此配置文件所需的 CMake 最低版本为3.16

include($ENV{IDF_PATH}/tools/cmake/project.cmake)   # 指定ESP-IDF应用的CMake配置文件
project(magicLight)       # 指定当前工程的名称为template
