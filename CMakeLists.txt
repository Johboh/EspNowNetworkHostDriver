FILE(GLOB_RECURSE sources "./src/impl/*.*")

if(IDF_VERSION_MAJOR GREATER_EQUAL 6 OR
    (IDF_VERSION_MAJOR EQUAL 5 AND IDF_VERSION_MINOR GREATER_EQUAL 3))
set(required_components esp_wifi mbedtls esp_http_client esp_http_server driver esp_driver_gpio)
else()
set(required_components esp_wifi mbedtls esp_http_client esp_http_server driver)
endif()

idf_component_register(COMPONENT_NAME "EspNowNetworkHostDriver"
                        SRCS ${sources}
                        INCLUDE_DIRS "./src/"
                        REQUIRES ${required_components})


if(IDF_VERSION_MAJOR LESS 5) # 5+ compiles with c++23.
target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++17)
endif()
