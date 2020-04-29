idf_component_register(SRCS "Capsense.cpp" "data.cpp" "fs_json.cpp" "IMU.cpp" "libmapper.cpp" "main.cpp" "OSC.cpp" "WiFi.cpp"
                       INCLUDE_DIRS "./"
                       REQUIRES arduino-esp32 arduino-libraries libmapper)
target_compile_options(${COMPONENT_LIB} PRIVATE -DESP32)
