target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    )

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
	        MQTTMessage.cpp
)
target_link_libraries(${CMAKE_PROJECT_NAME} pico_stdlib )
