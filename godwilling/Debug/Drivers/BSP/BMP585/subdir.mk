################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/BMP585/bmp585.c 

OBJS += \
./Drivers/BSP/BMP585/bmp585.o 

C_DEPS += \
./Drivers/BSP/BMP585/bmp585.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/BMP585/%.o Drivers/BSP/BMP585/%.su Drivers/BSP/BMP585/%.cyclo: ../Drivers/BSP/BMP585/%.c Drivers/BSP/BMP585/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/BSP/SD -I../Drivers/BSP/GNSS -I../Drivers/BSP/IMU -I../Drivers/BSP/ADXL314 -I../Drivers/BSP/Ra02 -I../Drivers/BSP/BMP585 -I../FATFS/Target -I../FATFS/App -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FatFs/src -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-BMP585

clean-Drivers-2f-BSP-2f-BMP585:
	-$(RM) ./Drivers/BSP/BMP585/bmp585.cyclo ./Drivers/BSP/BMP585/bmp585.d ./Drivers/BSP/BMP585/bmp585.o ./Drivers/BSP/BMP585/bmp585.su

.PHONY: clean-Drivers-2f-BSP-2f-BMP585

