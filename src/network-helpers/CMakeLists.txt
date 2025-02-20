
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}    
)


#DNS LOOKUP

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
	DNSLookup.cpp
)

target_link_libraries(${CMAKE_PROJECT_NAME}
        pico_stdlib
        pico_cyw43_arch_lwip_threadsafe_background
        pico_mbedtls
       	)


#Wifi Handler
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        WifiHandler.cpp
)

target_link_libraries(${CMAKE_PROJECT_NAME}
        pico_stdlib
        pico_cyw43_arch_lwip_threadsafe_background
)
 

#NTP
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        NTP.cpp
)

target_link_libraries(${CMAKE_PROJECT_NAME}
        pico_stdlib
        pico_cyw43_arch_lwip_threadsafe_background
)

#MQTT Connection
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        MQTTHandler.cpp
)

target_link_libraries(${CMAKE_PROJECT_NAME}
        pico_stdlib
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_mqtt
        pico_lwip_mbedtls
)

#MQTT Connection Helper
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        MQTTConnectionHelper.cpp
)

target_link_libraries(${CMAKE_PROJECT_NAME}
        pico_stdlib
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_mqtt
        pico_lwip_mbedtls
)