cmake_minimum_required(VERSION 3.5)
set(EXTRA_COMPONENT_DIRS "external/libmapper_idf/components" "external/GestureLooper/components" "external/GestureLooper/external/esp32-idf-blemidi/components")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(TStick)
