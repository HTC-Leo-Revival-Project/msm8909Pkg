

ARCH              = arm
TARGET            = arm-none-eabi
CROSS_COMPILE     = $(TARGET)-
CC                = $(CROSS_COMPILE)gcc
LD                = $(CROSS_COMPILE)ld
OBJCOPY           = $(CROSS_COMPILE)objcopy

all: ../ImageResources/$(PLATFORM)/bootpayload.bin

.SECONDARY:

clean:
	@echo "Removing build artifacts"
	@rm -f ../ImageResources/$(PLATFORM)/bootpayload.bin
	@rm -f ../ImageResources/$(PLATFORM)/BootShim.bin
	@rm -f ../ImageResources/$(PLATFORM)/BootShim.elf
	@rm -f ../ImageResources/$(PLATFORM)/BootShim.o
	@rm -f ../ImageResources/$(PLATFORM)/UEFI.fd

%/bootpayload.bin: %/BootShim.bin %/UEFI.fd
	@echo "[CAT] $(notdir $@)"
	@mkdir -p $(dir $@)
	@cat $^ > $@

%/BootShim.bin: %/BootShim.elf
	@echo "[OC] $(notdir $@)"
	@mkdir -p $(dir $@)
	@$(OBJCOPY) -O binary $< $@

%/BootShim.o: BootShim.S
	@echo "[CC] $(notdir $@)"
	@mkdir -p $(dir $@)
	@$(CC) -c $< -o $@ -mfpu=vfpv3 -mfloat-abi=hard -DUEFI_BASE=$(UEFI_BASE) -DUEFI_SIZE=$(UEFI_SIZE)

%/BootShim.elf: %/BootShim.o
	@echo "[CC] $(notdir $@)"
	@mkdir -p $(dir $@)
	@$(CC) $^ -nostartfiles -o $@

../ImageResources/%/UEFI.fd:
	@echo "[CP] $(notdir $@)"
	@mkdir -p $(dir $@)
	@cp $< $@

# Add FD paths for specific targets here
../ImageResources/Gold/UEFI.fd: $(WORKSPACE)/Build/HtcGold/DEBUG_GCC/FV/QSD8250_UEFI.fd
../ImageResources/Leo/UEFI.fd: $(WORKSPACE)/Build/HtcLeo/DEBUG_GCC/FV/QSD8250_UEFI.fd
../ImageResources/Schubert/UEFI.fd: $(WORKSPACE)/Build/HtcSchubert/DEBUG_GCC/FV/QSD8250_UEFI.fd
