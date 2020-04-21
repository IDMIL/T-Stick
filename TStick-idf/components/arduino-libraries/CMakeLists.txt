set(LIB_LIST "./Adafruit_BusIO" "./Adafruit_LIS3MDL" "./Adafruit_LSM9DS1" "./Adafruit_Sensor" "./ArduinoJson/src" "./Eigen" "./MIMU" "./OSC" "./SparkFun_LSM9DS1_Arduino_Library/src" "./WiFi32Manager")
idf_component_register(SRC_DIRS ${LIB_LIST}
                       INCLUDE_DIRS ${LIB_LIST}
                       REQUIRES arduino-esp32)
target_compile_options(${COMPONENT_LIB} PUBLIC -DARDUINO=100 -DESP32 -Wno-implicit-fallthrough -Wno-uninitialized)
