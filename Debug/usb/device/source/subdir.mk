################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../usb/device/source/usb_device_ch9.c \
../usb/device/source/usb_device_dci.c 

C_DEPS += \
./usb/device/source/usb_device_ch9.d \
./usb/device/source/usb_device_dci.d 

OBJS += \
./usb/device/source/usb_device_ch9.o \
./usb/device/source/usb_device_dci.o 


# Each subdirectory must supply rules for building sources it contributes
usb/device/source/%.o: ../usb/device/source/%.c usb/device/source/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DCPU_MIMXRT1011DAE5A -DCPU_MIMXRT1011DAE5A_cm7 -DDATA_SECTION_IS_CACHEABLE=1 -D_DEBUG=1 -DSDK_DEBUGCONSOLE=1 -DXIP_EXTERNAL_FLASH=1 -DXIP_BOOT_HEADER_ENABLE=1 -DUSB_STACK_BM -DUSING_SAI -DSDK_I2C_BASED_COMPONENT_USED=1 -DBOARD_USE_CODEC=1 -DCODEC_WM8960_ENABLE -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\board" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\source" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\usb\device\source\ehci" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\usb\include" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\osa" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\usb\device\include" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\usb\device\source" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\usb\phy" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\drivers" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\device" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\CMSIS" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\codec" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\component\i2c" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\component\lists" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\component\serial_manager" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\utilities" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\xip" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\component\uart" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\usb\device\class\audio" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\usb\device\class\midi" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\usb\device\class" -O3 -fno-common -g3 -gdwarf-4 -Wall -c  -ffunction-sections  -fdata-sections  -ffreestanding  -fno-builtin -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-usb-2f-device-2f-source

clean-usb-2f-device-2f-source:
	-$(RM) ./usb/device/source/usb_device_ch9.d ./usb/device/source/usb_device_ch9.o ./usb/device/source/usb_device_dci.d ./usb/device/source/usb_device_dci.o

.PHONY: clean-usb-2f-device-2f-source

