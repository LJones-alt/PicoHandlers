
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}    
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
	SensorHandler.cpp 
)

#Uhhhhhh... @Laura todo, put libraries used in here
target_link_libraries(${CMAKE_PROJECT_NAME} pico_stdlib hardware_adc pico_multicore hardware_gpio)
