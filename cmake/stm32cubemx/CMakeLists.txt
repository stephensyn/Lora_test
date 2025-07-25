cmake_minimum_required(VERSION 3.22)

project(stm32cubemx)
add_library(stm32cubemx INTERFACE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

target_compile_definitions(stm32cubemx INTERFACE
    USE_HAL_DRIVER
    STM32F030x8
    $<$<CONFIG:Debug>:DEBUG>
)

target_include_directories(stm32cubemx INTERFACE
    ../../Core/Inc
    ../../Radio/Inc
    ../../Drivers/STM32F0xx_HAL_Driver/Inc
    ../../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy
    ../../Drivers/CMSIS/Device/ST/STM32F0xx/Include
    ../../Drivers/CMSIS/Include
)

target_sources(stm32cubemx INTERFACE
    ../../Core/Src/main.c
    ../../Core/Src/delay.c
    ../../Radio/Src/crc.c
    ../../Radio/Src/sx1276.c
    ../../Radio/Src/sx1276-Hal.c
    ../../Radio/Src/sx1276-LoRa.c
    ../../Radio/Src/sx1276-LoRaMisc.c
    ../../Radio/Src/sx1276-FskMisc.c
    ../../Radio/Src/sx1276-FSK.c
    ../../Radio/Src/radio.c
    ../../Core/Src/gpio.c
    ../../Core/Src/spi.c
    ../../Core/Src/usart.c
    ../../Core/Src/dma.c
    ../../Core/Src/uart_dma_queue.c
    ../../Core/Src/stm32f0xx_it.c
    ../../Core/Src/stm32f0xx_hal_msp.c
    ../../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_spi.c
    ../../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_spi_ex.c
    ../../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rcc.c
    ../../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rcc_ex.c
    ../../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal.c
    ../../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_i2c.c
    ../../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_i2c_ex.c
    ../../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_gpio.c
    ../../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_dma.c
    ../../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_cortex.c
    ../../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pwr.c
    ../../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pwr_ex.c
    ../../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_flash.c
    ../../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_flash_ex.c
    ../../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_exti.c
    ../../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_tim.c
    ../../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_tim_ex.c
    ../../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_uart.c
    ../../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_uart_ex.c
    ../../Core/Src/system_stm32f0xx.c
    ../../Core/Src/sysmem.c
    ../../Core/Src/syscalls.c
    ../../startup_stm32f030x8.s
)

target_link_directories(stm32cubemx INTERFACE
)

target_link_libraries(stm32cubemx INTERFACE
)

# Validate that STM32CubeMX code is compatible with C standard
if(CMAKE_C_STANDARD LESS 11)
    message(ERROR "Generated code requires C11 or higher")
endif()
