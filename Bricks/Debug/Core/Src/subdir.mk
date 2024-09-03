################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/app_timers.c \
../Core/Src/audio_play.c \
../Core/Src/dma.c \
../Core/Src/freertos.c \
../Core/Src/lcd.c \
../Core/Src/main.c \
../Core/Src/mmc.c \
../Core/Src/retarget.c \
../Core/Src/sdram.c \
../Core/Src/stm32_lcd.c \
../Core/Src/stm32h7xx_hal_msp.c \
../Core/Src/stm32h7xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32h7xx.c \
../Core/Src/touchscreen.c \
../Core/Src/uart.c 

OBJS += \
./Core/Src/app_timers.o \
./Core/Src/audio_play.o \
./Core/Src/dma.o \
./Core/Src/freertos.o \
./Core/Src/lcd.o \
./Core/Src/main.o \
./Core/Src/mmc.o \
./Core/Src/retarget.o \
./Core/Src/sdram.o \
./Core/Src/stm32_lcd.o \
./Core/Src/stm32h7xx_hal_msp.o \
./Core/Src/stm32h7xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32h7xx.o \
./Core/Src/touchscreen.o \
./Core/Src/uart.o 

C_DEPS += \
./Core/Src/app_timers.d \
./Core/Src/audio_play.d \
./Core/Src/dma.d \
./Core/Src/freertos.d \
./Core/Src/lcd.d \
./Core/Src/main.d \
./Core/Src/mmc.d \
./Core/Src/retarget.d \
./Core/Src/sdram.d \
./Core/Src/stm32_lcd.d \
./Core/Src/stm32h7xx_hal_msp.d \
./Core/Src/stm32h7xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32h7xx.d \
./Core/Src/touchscreen.d \
./Core/Src/uart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H750xx -c -I"C:/faks/OR/ORLab-STM32H7-main/ORLab-STM32H7-main/STM32H750B-DK_BSP_C_Basic/Drivers/BSP" -I"C:/faks/OR/ORLab-STM32H7-main/ORLab-STM32H7-main/STM32H750B-DK_BSP_C_Basic/Utilities/Fonts" -I"C:/faks/OR/ORLab-STM32H7-main/ORLab-STM32H7-main/STM32H750B-DK_BSP_C_Basic/Drivers/BSP/Components/ft5336" -I"C:/faks/OR/ORLab-STM32H7-main/ORLab-STM32H7-main/STM32H750B-DK_BSP_C_Basic/Drivers/BSP/Components/mt25tl01g" -I"C:/faks/OR/ORLab-STM32H7-main/ORLab-STM32H7-main/STM32H750B-DK_BSP_C_Basic/Drivers/BSP/Components/mt48lc4m32b2" -I"C:/faks/OR/ORLab-STM32H7-main/ORLab-STM32H7-main/STM32H750B-DK_BSP_C_Basic/Drivers/BSP/Components/rk043fn48h" -I"C:/faks/OR/ORLab-STM32H7-main/ORLab-STM32H7-main/STM32H750B-DK_BSP_C_Basic/Drivers/BSP/Components/wm8994" -I"C:/faks/OR/ORLab-STM32H7-main/ORLab-STM32H7-main/STM32H750B-DK_BSP_C_Basic/Drivers/BSP/Components/Common" -I"C:/faks/OR/ORLab-STM32H7-main/ORLab-STM32H7-main/STM32H750B-DK_BSP_C_Basic/Middlewares/ST/STM32_Audio/Addons/PDM/Inc" -I"C:/faks/OR/ORLab-STM32H7-main/ORLab-STM32H7-main/STM32H750B-DK_BSP_C_Basic/Drivers/BSP/Components" -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/app_timers.cyclo ./Core/Src/app_timers.d ./Core/Src/app_timers.o ./Core/Src/app_timers.su ./Core/Src/audio_play.cyclo ./Core/Src/audio_play.d ./Core/Src/audio_play.o ./Core/Src/audio_play.su ./Core/Src/dma.cyclo ./Core/Src/dma.d ./Core/Src/dma.o ./Core/Src/dma.su ./Core/Src/freertos.cyclo ./Core/Src/freertos.d ./Core/Src/freertos.o ./Core/Src/freertos.su ./Core/Src/lcd.cyclo ./Core/Src/lcd.d ./Core/Src/lcd.o ./Core/Src/lcd.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/mmc.cyclo ./Core/Src/mmc.d ./Core/Src/mmc.o ./Core/Src/mmc.su ./Core/Src/retarget.cyclo ./Core/Src/retarget.d ./Core/Src/retarget.o ./Core/Src/retarget.su ./Core/Src/sdram.cyclo ./Core/Src/sdram.d ./Core/Src/sdram.o ./Core/Src/sdram.su ./Core/Src/stm32_lcd.cyclo ./Core/Src/stm32_lcd.d ./Core/Src/stm32_lcd.o ./Core/Src/stm32_lcd.su ./Core/Src/stm32h7xx_hal_msp.cyclo ./Core/Src/stm32h7xx_hal_msp.d ./Core/Src/stm32h7xx_hal_msp.o ./Core/Src/stm32h7xx_hal_msp.su ./Core/Src/stm32h7xx_it.cyclo ./Core/Src/stm32h7xx_it.d ./Core/Src/stm32h7xx_it.o ./Core/Src/stm32h7xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32h7xx.cyclo ./Core/Src/system_stm32h7xx.d ./Core/Src/system_stm32h7xx.o ./Core/Src/system_stm32h7xx.su ./Core/Src/touchscreen.cyclo ./Core/Src/touchscreen.d ./Core/Src/touchscreen.o ./Core/Src/touchscreen.su ./Core/Src/uart.cyclo ./Core/Src/uart.d ./Core/Src/uart.o ./Core/Src/uart.su

.PHONY: clean-Core-2f-Src

