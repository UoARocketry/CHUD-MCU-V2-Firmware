################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/IMU/imu.c 

OBJS += \
./Drivers/BSP/IMU/imu.o 

C_DEPS += \
./Drivers/BSP/IMU/imu.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/IMU/%.o Drivers/BSP/IMU/%.su Drivers/BSP/IMU/%.cyclo: ../Drivers/BSP/IMU/%.c Drivers/BSP/IMU/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/BSP/GNSS -I../Drivers/BSP/IMU -I../Drivers/BSP/ADXL314 -I../Drivers/BSP/Ra02 -I../Drivers/BSP/BMP585 -I../FATFS/Target -I../FATFS/App -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FatFs/src -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-IMU

clean-Drivers-2f-BSP-2f-IMU:
	-$(RM) ./Drivers/BSP/IMU/imu.cyclo ./Drivers/BSP/IMU/imu.d ./Drivers/BSP/IMU/imu.o ./Drivers/BSP/IMU/imu.su

.PHONY: clean-Drivers-2f-BSP-2f-IMU

