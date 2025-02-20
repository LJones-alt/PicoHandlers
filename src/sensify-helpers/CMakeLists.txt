target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}    
)

##### Self-Test ######
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
	        SensifySelfTest.cpp
)
target_link_libraries(${CMAKE_PROJECT_NAME} pico_stdlib hardware_adc)

##### Power Handler ######
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
	        PowerHandler.cpp
)
target_link_libraries(${CMAKE_PROJECT_NAME} pico_stdlib hardware_adc)


##### Watchdog ######
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
	        Watchdog.cpp
)
target_link_libraries(${CMAKE_PROJECT_NAME} pico_stdlib hardware_watchdog)

##### HealthAlert ######
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
	        HealthAlert.cpp
)
target_link_libraries(${CMAKE_PROJECT_NAME} pico_stdlib)

##### BootSel Reboot ######
target_link_libraries(${CMAKE_PROJECT_NAME} pico_bootrom)