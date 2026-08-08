################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/Ra02/ra02.c 

OBJS += \
./Drivers/BSP/Ra02/ra02.o 

C_DEPS += \
./Drivers/BSP/Ra02/ra02.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/Ra02/%.o Drivers/BSP/Ra02/%.su Drivers/BSP/Ra02/%.cyclo: ../Drivers/BSP/Ra02/%.c Drivers/BSP/Ra02/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/BSP/Ra02 -I../Drivers/BSP/BMP585 -I../FATFS/Target -I../FATFS/App -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FatFs/src -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-Ra02

clean-Drivers-2f-BSP-2f-Ra02:
	-$(RM) ./Drivers/BSP/Ra02/ra02.cyclo ./Drivers/BSP/Ra02/ra02.d ./Drivers/BSP/Ra02/ra02.o ./Drivers/BSP/Ra02/ra02.su

.PHONY: clean-Drivers-2f-BSP-2f-Ra02

