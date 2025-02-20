target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    SimpleDS3231.cpp
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}    
)

target_link_libraries(${CMAKE_PROJECT_NAME} pico_stdlib hardware_i2c)
