
cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

set(TESTIMG_FAT_FILENAME testimg.fat.img)

project(elix)

include_directories(libs/mfat/src)
include_directories(libs/mbmp/src)
include_directories(libs/mnet/src)

add_subdirectory(libs/mbmp)
add_subdirectory(libs/mfat)
add_subdirectory(libs/mnet)

set(ELIX_DRIVERS)
set(ELIX_LINK_EXTERNAL mfat mnet)

set(ELIX_TESTIMG_CONTENTS
   init.elx
   ${CMAKE_CURRENT_SOURCE_DIR}/README.md
   random.bmp)

if(UNIX)
    set(ELIX_DRIVER_PATH src/colinux)
    set(ELIX_DEFINES
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
    
    set(ELIX_BOOT_MODULE src/colinux/boot.c)

    list(APPEND ELIX_LINK_EXTERNAL PkgConfig::PCAP mbmp)
    list(APPEND ELIX_DRIVERS
        ${ELIX_DRIVER_PATH}/net.c
        ${ELIX_DRIVER_PATH}/display.c
        ${ELIX_DRIVER_PATH}/keyboard.c)
elseif(MSP430_MCU)
    set(ELIX_DRIVER_PATH src/msp430)
    set(ELIX_DEFINES
        USE_DISK
	     USE_DISK_RO
        QD_UART_HW
        QD_CPU_MHZ=16
        QD_SPI
        QD_UART
	     USE_ERROR_CODES
	     CONSOLE_IN=uart
	     CONSOLE_OUT=uart
	     MEM_HEAP_SIZE=274
	     UART_TX_BUFFER_LENGTH=0
        UART_RX_BUFFER_LENGTH=20
        QD_WDT_DISABLED
        QD_PLATFORM=QD_PLATFORM_MSP430)
    set(ELIX_BOOT_MODULE src/msp430/boot.c)
    list(APPEND ELIX_DRIVERS
        ${ELIX_DRIVER_PATH}/irqal.c
        ${ELIX_DRIVER_PATH}/i2c.c
        ${ELIX_DRIVER_PATH}/uart.c)
endif(UNIX)

set(ELIX_SOURCES
    src/console.c
    src/adhd.c
    src/vm.c
    src/mem.c
    src/kernel.c)

list(APPEND ELIX_DRIVERS
    ${ELIX_DRIVER_PATH}/disk.c)

# Build test data.

add_custom_command(
   OUTPUT ${TESTIMG_FAT_FILENAME}
   COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/mktestimg.sh
      ${TESTIMG_FAT_FILENAME} ${ELIX_TESTIMG_CONTENTS}
   DEPENDS ${ELIX_TESTIMG_CONTENTS})

add_custom_command(
   OUTPUT random.bmp
   COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/mktestbmp.sh random.bmp)

add_custom_command(
   OUTPUT init.elx
   COMMAND assm ${CMAKE_CURRENT_SOURCE_DIR}/sysapps/init.asm init.elx
   DEPENDS assm ${CMAKE_CURRENT_SOURCE_DIR}/sysapps/init.asm)

if(MSP430_MCU)
   msp430_add_executable_compilation(elix ${ELIX_SOURCES} ${ELIX_DRIVERS} ${ELIX_BOOT_MODULE})
   target_compile_definitions(elix.elf PUBLIC ${ELIX_DEFINES})
   target_compile_definitions(mfat PUBLIC ${ELIX_DEFINES})
   target_link_libraries(elix.elf PUBLIC ${ELIX_LINK_EXTERNAL})
else()
   add_executable(elix ${ELIX_SOURCES} ${ELIX_DRIVERS} ${ELIX_BOOT_MODULE})
   target_compile_definitions(elix PUBLIC ${ELIX_DEFINES})
   target_link_libraries(elix PUBLIC ${ELIX_LINK_EXTERNAL})

   add_executable(assm src/assm.c)

   add_custom_target(testfat ALL DEPENDS ${TESTIMG_FAT_FILENAME})
endif()

