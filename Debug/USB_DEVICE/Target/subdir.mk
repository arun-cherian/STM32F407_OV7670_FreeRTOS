################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USB_DEVICE/Target/usbd_conf.c 

C_DEPS += \
./USB_DEVICE/Target/usbd_conf.d 

OBJS += \
./USB_DEVICE/Target/usbd_conf.o 


# Each subdirectory must supply rules for building sources it contributes
USB_DEVICE/Target/%.o USB_DEVICE/Target/%.su USB_DEVICE/Target/%.cyclo: ../USB_DEVICE/Target/%.c USB_DEVICE/Target/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Drivers/STM32F4xx_HAL_Driver/Inc -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Drivers/CMSIS/Device/ST/STM32F4xx/Include -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FreeRTOS/Source/include -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/ST/STM32_USB_Device_Library/Core/Inc -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../FATFS/Target -I../FATFS/App -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-USB_DEVICE-2f-Target

clean-USB_DEVICE-2f-Target:
	-$(RM) ./USB_DEVICE/Target/usbd_conf.cyclo ./USB_DEVICE/Target/usbd_conf.d ./USB_DEVICE/Target/usbd_conf.o ./USB_DEVICE/Target/usbd_conf.su

.PHONY: clean-USB_DEVICE-2f-Target

