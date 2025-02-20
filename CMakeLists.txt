cmake_minimum_required(VERSION 3.13)
set(PICO_SDK_FETCH_FROM_GIT off)

set(SENSIFY_BOARD_REVISION Sensify-1v4)
set(PICO_SDK_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/pico-sdk)
set(PICO_MBEDTLS_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/pico-sdk/lib/mbedtls)

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD pico_w)
set(PICO_CYW43_ARCH_THREADSAFE_BACKGROUND 1)

pico_sdk_init()

#################################
####### Main Application ########
#################################

project(Main C CXX ASM)
    

if(SENSIFY_TEST_MODE)    
    add_executable(${CMAKE_PROJECT_NAME}
        Main.cpp Test.cpp    
    )
else(SENSIFY_TEST_MODE)    
    add_executable(${CMAKE_PROJECT_NAME}
        Main.cpp Test.cpp    
    )
endif(SENSIFY_TEST_MODE) 



#Add DM libraries and submodules
add_subdirectory(src)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        #MBEDTLS_SSL_VERIFY_REQUIRED
        ALTCP_MBEDTLS_AUTHMODE=MBEDTLS_SSL_VERIFY_NONE
) 

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include/sensify-boards/${SENSIFY_BOARD_REVISION}
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/src
    )


# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(${CMAKE_PROJECT_NAME}
pico_stdlib
pico_multicore
)


pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 1)
pico_enable_stdio_uart(${CMAKE_PROJECT_NAME} 1)
pico_add_extra_outputs(${CMAKE_PROJECT_NAME})