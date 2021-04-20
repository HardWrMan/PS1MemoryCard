################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal.c \
../Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_cortex.c \
../Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_flash.c \
../Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_flash_ex.c \
../Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_gpio.c \
../Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pwr.c \
../Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pwr_ex.c \
../Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rcc.c \
../Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rcc_ex.c 

OBJS += \
./Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal.o \
./Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_cortex.o \
./Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_flash.o \
./Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_flash_ex.o \
./Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_gpio.o \
./Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pwr.o \
./Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pwr_ex.o \
./Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rcc.o \
./Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rcc_ex.o 

C_DEPS += \
./Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal.d \
./Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_cortex.d \
./Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_flash.d \
./Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_flash_ex.d \
./Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_gpio.d \
./Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pwr.d \
./Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pwr_ex.d \
./Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rcc.d \
./Lib/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rcc_ex.d 


# Each subdirectory must supply rules for building sources it contributes
Lib/STM32F0xx_HAL_Driver/Src/%.o: ../Lib/STM32F0xx_HAL_Driver/Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -pedantic  -g3 -DSTM32F042x6 -DHSE_VALUE=8000000 -I"F:\WORK\!ARM\PS1MemoryCard\Lib\CMSIS\Core" -I"F:\WORK\!ARM\PS1MemoryCard\Lib\CMSIS\Device" -I"F:\WORK\!ARM\PS1MemoryCard\Lib\STM32F0xx_HAL_Driver\Inc" -I"F:\WORK\!ARM\PS1MemoryCard\Src" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


