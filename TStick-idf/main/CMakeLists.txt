idf_component_register(SRCS "TStick-idf.cpp" "Capsense.cpp" "data.cpp" "fs_json.cpp" "IMU.cpp" "libmapper.cpp" "OSC.cpp" "WiFi.cpp" "instrument.cpp"
                       INCLUDE_DIRS "./"
                       REQUIRES arduino-esp32 TStick-arduino-libraries libmapper-esp)
target_compile_options(${COMPONENT_LIB} PRIVATE -DESP32 -Wno-format)
