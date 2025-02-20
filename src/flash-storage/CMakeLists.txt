
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
	       FlashStorage.cpp ObjectCache.h CRC/CRC16.cpp
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/CRC
)

target_link_libraries(${CMAKE_PROJECT_NAME} pico_stdlib hardware_flash)
