################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FatFs/src/diskio.c \
C:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FatFs/src/ff.c \
C:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FatFs/src/ff_gen_drv.c \
C:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FatFs/src/option/syscall.c 

C_DEPS += \
./Middlewares/FatFs/diskio.d \
./Middlewares/FatFs/ff.d \
./Middlewares/FatFs/ff_gen_drv.d \
./Middlewares/FatFs/syscall.d 

OBJS += \
./Middlewares/FatFs/diskio.o \
./Middlewares/FatFs/ff.o \
./Middlewares/FatFs/ff_gen_drv.o \
./Middlewares/FatFs/syscall.o 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/FatFs/diskio.o: C:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FatFs/src/diskio.c Middlewares/FatFs/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Drivers/STM32F4xx_HAL_Driver/Inc -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Drivers/CMSIS/Device/ST/STM32F4xx/Include -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FreeRTOS/Source/include -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/ST/STM32_USB_Device_Library/Core/Inc -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../FATFS/Target -I../FATFS/App -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Middlewares/FatFs/ff.o: C:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FatFs/src/ff.c Middlewares/FatFs/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Drivers/STM32F4xx_HAL_Driver/Inc -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Drivers/CMSIS/Device/ST/STM32F4xx/Include -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FreeRTOS/Source/include -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/ST/STM32_USB_Device_Library/Core/Inc -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../FATFS/Target -I../FATFS/App -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Middlewares/FatFs/ff_gen_drv.o: C:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FatFs/src/ff_gen_drv.c Middlewares/FatFs/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Drivers/STM32F4xx_HAL_Driver/Inc -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Drivers/CMSIS/Device/ST/STM32F4xx/Include -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FreeRTOS/Source/include -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/ST/STM32_USB_Device_Library/Core/Inc -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../FATFS/Target -I../FATFS/App -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Middlewares/FatFs/syscall.o: C:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FatFs/src/option/syscall.c Middlewares/FatFs/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Drivers/STM32F4xx_HAL_Driver/Inc -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Drivers/CMSIS/Device/ST/STM32F4xx/Include -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FreeRTOS/Source/include -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/ST/STM32_USB_Device_Library/Core/Inc -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../FATFS/Target -I../FATFS/App -IC:/Users/Arun/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2/Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-FatFs

clean-Middlewares-2f-FatFs:
	-$(RM) ./Middlewares/FatFs/diskio.cyclo ./Middlewares/FatFs/diskio.d ./Middlewares/FatFs/diskio.o ./Middlewares/FatFs/diskio.su ./Middlewares/FatFs/ff.cyclo ./Middlewares/FatFs/ff.d ./Middlewares/FatFs/ff.o ./Middlewares/FatFs/ff.su ./Middlewares/FatFs/ff_gen_drv.cyclo ./Middlewares/FatFs/ff_gen_drv.d ./Middlewares/FatFs/ff_gen_drv.o ./Middlewares/FatFs/ff_gen_drv.su ./Middlewares/FatFs/syscall.cyclo ./Middlewares/FatFs/syscall.d ./Middlewares/FatFs/syscall.o ./Middlewares/FatFs/syscall.su

.PHONY: clean-Middlewares-2f-FatFs

