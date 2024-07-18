

ARCH              = arm
TARGET            = arm-none-eabi
CROSS_COMPILE     = $(TARGET)-
CC                = $(CROSS_COMPILE)gcc
OBJCOPY           = $(CROSS_COMPILE)objcopy

all: BooterShim.elf BooterShim.bin

BooterShim.bin: BooterShim.elf
	$(OBJCOPY) -O binary $< $@

BooterShim.elf: BooterShim.S
	$(CC) -c $< -o $@ -mfpu=vfpv3 -mfloat-abi=hard

BooterShim.S: