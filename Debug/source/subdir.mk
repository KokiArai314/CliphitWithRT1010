################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/audio_player.c \
../source/composite.c \
../source/edit.c \
../source/midi_device.c \
../source/midi_if.c \
../source/midi_message.c \
../source/midi_player.c \
../source/param.c \
../source/semihost_hardfault.c \
../source/usb_device_descriptor.c 

C_DEPS += \
./source/audio_player.d \
./source/composite.d \
./source/edit.d \
./source/midi_device.d \
./source/midi_if.d \
./source/midi_message.d \
./source/midi_player.d \
./source/param.d \
./source/semihost_hardfault.d \
./source/usb_device_descriptor.d 

OBJS += \
./source/audio_player.o \
./source/composite.o \
./source/edit.o \
./source/midi_device.o \
./source/midi_if.o \
./source/midi_message.o \
./source/midi_player.o \
./source/param.o \
./source/semihost_hardfault.o \
./source/usb_device_descriptor.o 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c source/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DCPU_MIMXRT1011DAE5A -DCPU_MIMXRT1011DAE5A_cm7 -DDATA_SECTION_IS_CACHEABLE=1 -D_DEBUG=1 -DSDK_DEBUGCONSOLE=1 -DXIP_EXTERNAL_FLASH=1 -DXIP_BOOT_HEADER_ENABLE=1 -DUSB_STACK_BM -DUSING_SAI -DSDK_I2C_BASED_COMPONENT_USED=1 -DBOARD_USE_CODEC=1 -DCODEC_WM8960_ENABLE -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\board" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\source" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\usb\device\source\ehci" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\usb\include" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\osa" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\usb\device\include" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\usb\device\source" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\usb\phy" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\drivers" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\device" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\CMSIS" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\codec" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\component\i2c" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\component\lists" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\component\serial_manager" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\utilities" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\xip" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\component\uart" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\usb\device\class\audio" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\usb\device\class\midi" -I"C:\Users\koki_arai\Documents\MCUXpressoIDE_11.10.0_3148\workspace\evkmimxrt1010_dev_composite_audio_midi_bm\usb\device\class" -O0 -fno-common -g3 -gdwarf-4 -Wall -c  -ffunction-sections  -fdata-sections  -ffreestanding  -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-source

clean-source:
	-$(RM) ./source/audio_player.d ./source/audio_player.o ./source/composite.d ./source/composite.o ./source/edit.d ./source/edit.o ./source/midi_device.d ./source/midi_device.o ./source/midi_if.d ./source/midi_if.o ./source/midi_message.d ./source/midi_message.o ./source/midi_player.d ./source/midi_player.o ./source/param.d ./source/param.o ./source/semihost_hardfault.d ./source/semihost_hardfault.o ./source/usb_device_descriptor.d ./source/usb_device_descriptor.o

.PHONY: clean-source

