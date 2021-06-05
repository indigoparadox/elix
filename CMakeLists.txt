
cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

project(elix)

include_directories(libs/mfat/src)
include_directories(libs/mbmp/src)
include_directories(libs/mnet/src)

add_subdirectory(libs/mbmp)
add_subdirectory(libs/mfat)
add_subdirectory(libs/mnet)

if(UNIX)
    set(ELIX_DRIVER_PATH src/colinux)
    set(ELIX_DEFINES
        USE_EXT_CLI
        USE_NET
        USE_BMP
        USE_DISK
        QD_PLATFORM=QD_PLATFORM_COLINUX
        DEBUG
        MBMP_AGGRESSIVE_VALIDATE
        COLINUX_TERMIOS)
    find_package(PkgConfig REQUIRED)

    # Use PCAP for networking.
    pkg_check_modules(PCAP REQUIRED IMPORTED_TARGET libpcap)
    set(ELIX_LINK_EXTERNAL PkgConfig::PCAP mfat mbmp mnet)
    
    set(ELIX_BOOT_MODULE src/colinux/boot.c)
endif(UNIX)

set(ELIX_SOURCES
    src/console.c
    src/adhd.c
    src/alpha.c
    src/vm.c
    src/mem.c
    src/kernel.c)
    #src/net/net.c)

set(ELIX_DRIVERS
    ${ELIX_DRIVER_PATH}/net.c
    ${ELIX_DRIVER_PATH}/display.c
    ${ELIX_DRIVER_PATH}/keyboard.c
    ${ELIX_DRIVER_PATH}/disk.c)

add_executable(elix ${ELIX_SOURCES} ${ELIX_DRIVERS} ${ELIX_BOOT_MODULE})

target_compile_definitions(elix PUBLIC ${ELIX_DEFINES})

target_link_libraries(elix PUBLIC ${ELIX_LINK_EXTERNAL})
