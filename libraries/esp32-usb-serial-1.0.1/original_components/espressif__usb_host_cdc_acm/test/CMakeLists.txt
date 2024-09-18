include($ENV{IDF_PATH}/tools/cmake/version.cmake)
set (TINYUSB_LIB)
if("${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR}" VERSION_GREATER_EQUAL "5.0")
    set(TINYUSB_LIB "esp_tinyusb")
else()
    set(TINYUSB_LIB "tinyusb")
endif()
idf_component_register(SRCS "test_cdc_acm_host.c" "usb_device.c"
                       INCLUDE_DIRS "."
                       REQUIRES usb_host_cdc_acm unity ${TINYUSB_LIB})
