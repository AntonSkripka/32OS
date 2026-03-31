CC = /home/anton32/opt/cross/bin/x86_64-elf-gcc
LD = /home/anton32/opt/cross/bin/x86_64-elf-ld
OBJCOPY = /home/anton32/opt/cross/bin/x86_64-elf-objcopy
AS = nasm
QEMU = qemu-system-x86_64

BUILD_DIR = build

CFLAGS_BASE = -ffreestanding -fno-stack-protector -fno-pie -O2 -Wall -Iinclude
CFLAGS_32   = $(CFLAGS_BASE) -m32 -fno-pic -fcf-protection=none -mno-sse -mno-mmx
ASFLAGS_32 = -f elf32
CFLAGS_64   = $(CFLAGS_BASE) -m64 -mno-red-zone -mcmodel=small -fno-pic -mno-sse -mno-mmx -mno-80387 -mgeneral-regs-only
ASFLAGS_64 = -f elf64

DRIVERS_SOURCES = src/drivers/vga.c \
				src/drivers/serial.c
DRIVERS_OBJS_32 = $(patsubst src/%.c, $(BUILD_DIR)/32/src/%.o, $(DRIVERS_SOURCES))
DRIVERS_OBJS_64 = $(patsubst src/%.c, $(BUILD_DIR)/64/src/%.o, $(DRIVERS_SOURCES))

BUNDLE_OBJS = $(BUILD_DIR)/32/bundles/supervisor64_bundle.o \
           $(BUILD_DIR)/32/bundles/kernel64_bundle.o

OBJS_BOOT_32 = $(BUILD_DIR)/32/src/boot/boot.o \
		  $(BUILD_DIR)/32/src/boot/supervisor32.o \
          $(BUILD_DIR)/32/src/boot/gdt_init.o \
		  $(BUILD_DIR)/32/src/boot/jump.o \
          $(DRIVERS_OBJS_32)

OBJS_SUP_64 = $(BUILD_DIR)/64/src/supervisor/arch/entry.o \
		  $(BUILD_DIR)/64/src/supervisor/supervisor.o \
		  $(BUILD_DIR)/64/src/supervisor/region_array.o \
          $(BUILD_DIR)/64/src/supervisor/arch/gdt.o \
		  $(BUILD_DIR)/64/src/supervisor/arch/gdt_asm.o \
		  $(BUILD_DIR)/64/src/supervisor/arch/idt.o \
          $(BUILD_DIR)/64/src/supervisor/arch/idt_asm.o \
		  $(BUILD_DIR)/64/src/supervisor/arch/paging.o \
          $(BUILD_DIR)/64/src/supervisor/arch/paging_asm.o \
          $(BUILD_DIR)/64/src/supervisor/arch/apic.o \
          $(BUILD_DIR)/64/src/supervisor/arch/timer.o \
          $(DRIVERS_OBJS_64)

OBJS_KERN_64 = $(BUILD_DIR)/64/src/kernel/kernel_entry.o \
			$(BUILD_DIR)/64/src/kernel/kernel.o \
			$(DRIVERS_OBJS_64)

all: $(BUILD_DIR)/os_image.elf

$(BUILD_DIR)/supervisor64.bin: $(OBJS_SUP_64)
	@mkdir -p $(dir $@)
	$(LD) -m elf_x86_64 -T src/supervisor/supervisor64.ld -o $(BUILD_DIR)/supervisor64.elf $(OBJS_SUP_64)
	$(OBJCOPY) -O binary $(BUILD_DIR)/supervisor64.elf $@

$(BUILD_DIR)/kernel64.bin: $(OBJS_KERN_64)
	@mkdir -p $(dir $@)
	$(LD) -m elf_x86_64 -T src/kernel/kernel64.ld -o $(BUILD_DIR)/kernel64.elf $(OBJS_KERN_64)
	$(OBJCOPY) -O binary $(BUILD_DIR)/kernel64.elf $@

$(BUILD_DIR)/32/bundles/%_bundle.o: src/boot/bundles/%_bundle.asm $(BUILD_DIR)/%.bin
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS_32) -I$(BUILD_DIR)/ $< -o $@

$(BUILD_DIR)/os_image.elf: $(OBJS_BOOT_32) $(BUNDLE_OBJS)
	$(LD) -m elf_i386 -T linker.ld -o $@ $(OBJS_BOOT_32) $(BUNDLE_OBJS)


$(BUILD_DIR)/32/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_32) -c $< -o $@

$(BUILD_DIR)/32/%.o: %.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS_32) $< -o $@

$(BUILD_DIR)/64/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_64) -c $< -o $@

$(BUILD_DIR)/64/%.o: %.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS_64) $< -o $@

clean:
	rm -rf $(BUILD_DIR)

run: all
	$(QEMU) -kernel $(BUILD_DIR)/os_image.elf -serial stdio -d int,cpu -D $(BUILD_DIR)/qemu-exec.log -no-reboot