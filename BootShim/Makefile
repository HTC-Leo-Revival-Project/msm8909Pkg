

ARCH              = arm
TARGET            = arm-none-eabi
CROSS_COMPILE     = $(TARGET)-
CC                = $(CROSS_COMPILE)gcc
OBJCOPY           = $(CROSS_COMPILE)objcopy

all: BootShim.bin

clean:
	@echo "Removing build artifacts"
	@rm -f *.elf
	@rm -f *.bin

BootShim.bin: BootShim.elf
	@echo "[OC] $@"
	@$(OBJCOPY) -O binary $< $@

BootShim.elf: BootShim.S
	@echo "[CC] $@"
	@$(CC) -c $< -o $@ -mfpu=vfpv3 -mfloat-abi=hard -DUEFI_BASE=$(UEFI_BASE) -DUEFI_SIZE=$(UEFI_SIZE)

BootShim.S: