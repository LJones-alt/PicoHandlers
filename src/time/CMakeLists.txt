add_subdirectory(ds3231)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    Time.cpp
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}    
)

#Uhhhhhh... @Laura todo, put libraries used in here
target_link_libraries(${CMAKE_PROJECT_NAME} pico_stdlib hardware_rtc)
