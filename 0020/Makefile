# Default make target
.PHONY: all
all: yaos.img 

X64 = 1

BITS = 64
XFLAGS = -DDEBUG -std=gnu11 -m64 -DX64 -mcmodel=kernel -mtls-direct-seg-refs \
	-mno-red-zone  -fno-delete-null-pointer-checks  -fno-omit-frame-pointer \
	-fno-optimize-sibling-calls -fno-var-tracking-assignments -fconserve-stack \
	-fno-strict-overflow 	-Wno-pointer-sign  -falign-jumps=1 -falign-loops=1 \
	--param=allow-store-data-races=0   -fno-common  -mno-80387 \
	-mno-fp-ret-in-387 -mskip-rax-setup -mtune=generic \
	-fno-asynchronous-unwind-tables  -mno-sse -mno-mmx -mno-sse2 -mno-3dnow \
	-mno-avx
LDFLAGS = -m elf_x86_64 -nodefaultlibs  

FSGSBASE=$(shell cat /proc/cpuinfo|grep fsgsbase)
ifneq ($(strip $(FSGSBASE)),)
   XFLAGS+= -D__FSGSBASE__
endif
OUT = out

HOST_CC ?= gcc

OPT ?= -O2
ARCHOBJ_DIR =.archobj
DOBJ_DIR =.dobj
KOBJ_DIR = .kobj
OBJS := $(addprefix $(KOBJ_DIR)/,$(OBJS))
AOBJS := entry64.o main.o vgaoutput.o uart.o cpu.o traps.o vectors.o trapasm64.o \
  pm64.o string64.o multiboot.o pgtable.o phymem.o acpi.o apic.o alternative.o \
  ioapic.o hpet.o time.o irq.o apic_timer.o tsc.o msi.o \
  lib/memcpy_64.o lib/memset_64.o lib/memmove_64.o \
  lib/iomap_copy_64.o lib/copy_page_64.o 
KOBJS := printk.o kheap.o yaos.o main.o smp.o kthread.o timer.o tasklet.o softirq.o \
  test.o vm.o module.o yaos_page.o yaoscall.o
DOBJS := pci.o 
ifneq ($(MAKECMDGOALS),clean)
include $(shell test -d $(ARCHOBJ_DIR) && find $(ARCHOBJ_DIR) -name '*.d')
include $(shell test -d $(DOBJ_DIR) && find $(DOBJ_DIR) -name '*.d')
include $(shell test -d $(KOBJ_DIR) && find $(KOBJ_DIR) -name '*.d')

endif
MODULEC_SOURCES = $(shell find module -name "*.c")
MODULEC_OBJECTS = $(patsubst %.c, %.o, $(MODULEC_SOURCES))

LIBS_SOURCES = $(shell find libs -name "*.c")
LIBS_OBJECTS = $(patsubst %.c, %.o, $(LIBS_SOURCES))
LIBS_ASMSOURCES = $(shell find libs -name "*.S")
LIBS_ASMOBJECTS = $(patsubst %.S, %.o, $(LIBS_ASMSOURCES))
OBJS := $(addprefix $(ARCHOBJ_DIR)/,$(AOBJS)) \
          $(addprefix $(KOBJ_DIR)/,$(KOBJS)) $(addprefix $(DOBJ_DIR)/,$(DOBJS)) 
OBJS += $(MODULEC_OBJECTS) $(LIBS_OBJECTS) $(LIBS_ASMOBJECTS)

# Cross-compiling (e.g., on Mac OS X)
CROSS_COMPILE ?=

# If the makefile can't find QEMU, specify its path here
QEMU ?= qemu-system-x86_64

CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)gas
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
cc-option = $(shell if $(CC) $(1) -S -o /dev/null -xc /dev/null \
	> /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi ;)

CFLAGS = -fno-pic -static -fno-builtin -fno-strict-aliasing -Wall
CFLAGS += -g -Wall -MD -D__KERNEL__ 
CFLAGS += -ffreestanding -fno-common -nostdlib -I arch/x86_64/include -Iinclude -gdwarf-2 $(XFLAGS) $(OPT) 
CFLAGS += $(call cc-option, -fno-stack-protector, "")
CFLAGS += $(call cc-option, -fno-stack-protector-all, "")
ASFLAGS = -gdwarf-2 -fno-pie -Wa,-divide -D__ASSEMBLY__ -Iinclude -I arch/x86_64/include $(XFLAGS)

MODULEC_FLAGS=$(CFLAGS) -D_MODULE
$(KOBJ_DIR)/%.o: kernel/%.c
	@mkdir -p $(KOBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(KOBJ_DIR)/%.o: kernel/%.S
	@mkdir -p $(KOBJ_DIR)
	$(CC) $(ASFLAGS) -c -o $@ $<

$(ARCHOBJ_DIR)/%.o: arch/x86_64/%.c
	 @mkdir -p $(ARCHOBJ_DIR)
	 $(CC) $(CFLAGS) -c -o $@ $<

$(ARCHOBJ_DIR)/%.o: arch/x86_64/%.S
	@mkdir -p $(ARCHOBJ_DIR)
	@mkdir -p $(ARCHOBJ_DIR)/lib
	 $(CC) $(ASFLAGS) -c -o $@ $<

$(DOBJ_DIR)/%.o: drivers/%.c
	@mkdir -p $(DOBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(DOBJ_DIR)/%.o: drivers/%.S
	@mkdir -p $(DOBJ_DIR)
	$(CC) $(ASFLAGS) -c -o $@ $<

.c.o:
	$(CC) $(MODULEC_FLAGS) -c $< -o $@
.S.o:
	$(CC) $(ASFLAGS)  -c -o $@ $<

$(OUT)/bootblock: bootloader/bootasm.S bootloader/bootmain.c
	@mkdir -p $(OUT)
	$(CC) -fno-builtin -fno-pic -m32 -nostdinc -fno-stack-protector -Iinclude \
  -Os -o $(OUT)/bootmain.o -c bootloader/bootmain.c
	$(CC) -fno-builtin -fno-pic -m32 -nostdinc -Iarch/x86_64/include -Iinclude \
   -o $(OUT)/bootasm.o -c bootloader/bootasm.S
	$(LD) -m elf_i386 -nodefaultlibs --omagic -e start -Ttext 0x7C00 \
                -o $(OUT)/bootblock.o $(OUT)/bootasm.o $(OUT)/bootmain.o
	$(OBJDUMP) -S $(OUT)/bootblock.o > $(OUT)/bootblock.asm
	$(OBJCOPY) -S -O binary -j .text $(OUT)/bootblock.o $(OUT)/bootblock
	../tools/sign.pl $(OUT)/bootblock
clean: 
	rm -rf $(OUT) $(FS_DIR) $(UOBJ_DIR) $(KOBJ_DIR) $(ARCHOBJ_DIR) $(DOBJ_DIR)
	rm -f kernel/vectors.S yaos.img usr.img yaosmemfs.img  .gdbinit
	rm -rf $(MODULEC_OBJECTS)
	rm -rf $(LIBS_OBJECTS)
	rm -rf `find libs module |grep \.d$`
ifndef CPUS
CPUS := $(shell grep -c ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu)
endif
$(OUT)/entryother: arch/x86_64/entryother.S
	@mkdir -p $(OUT)
	$(CC) $(CFLAGS) -fno-pic -nostdinc -I. -o $(OUT)/entryother.o -c arch/x86_64/entryother.S
	$(LD) $(LDFLAGS) --omagic -e start -Ttext 0x7000 -o $(OUT)/bootblockother.o $(OUT)/entryother.o
	$(OBJCOPY) -S -O binary -j .text $(OUT)/bootblockother.o $(OUT)/entryother
	$(OBJDUMP) -S $(OUT)/bootblockother.o > $(OUT)/entryother.asm

ENTRYCODE = $(ARCHOBJ_DIR)/entry64.o
LINKSCRIPT = arch/x86_64/kernel64.ld
LIBS=
$(OUT)/kernel.elf: $(OBJS) $(OUT)/entryother  $(LINKSCRIPT) 
	@mkdir -p $(OUT)
	$(LD) $(LDFLAGS) -T $(LINKSCRIPT) -o $(OUT)/kernel.elf \
		$(OBJS)  -b  binary $(OUT)/entryother $(LIBS) 
	$(OBJDUMP) -S $(OUT)/kernel.elf > $(OUT)/kernel.asm
	$(OBJDUMP) -t $(OUT)/kernel.elf | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $(OUT)/kernel.sym

MKVECTORS = ../tools/vectors64.pl
kernel/vectors.S: $(MKVECTORS)
	perl $(MKVECTORS) > kernel/vectors.S

QEMUOPTS =-enable-kvm -cpu host,+x2apic  \
  -device virtio-blk-pci,id=blk0,drive=hd0,scsi=off \
   -drive file=./usr.img,if=none,id=hd0,cache=none,aio=native\
   -netdev type=tap,script=qemu-ifup.sh,id=net0 -device virtio-net-pci,netdev=net0 \
    -vga vmware -display vnc=192.168.10.2:10\
  -smp 2 -m 512 $(QEMUEXTRA)

yaos.img: $(OUT)/bootblock $(OUT)/kernel.elf
	dd if=/dev/zero of=yaos.img count=10000
	dd if=$(OUT)/bootblock of=yaos.img conv=notrunc
	dd if=$(OUT)/kernel.elf of=yaos.img seek=1 conv=notrunc
usr.img:  $(OUT)/kernel.elf
	dd if=/dev/zero of=usr.img count=100

qemuimg: yaos.img
	@echo Ctrl+a h for help
	$(QEMU) -serial mon:stdio  -nographic $(QEMUOPTS) -hda yaos.img
qemu:   out/kernel.elf usr.img
	@echo Ctrl+a h for help
	$(QEMU) -serial mon:stdio  -nographic $(QEMUOPTS) -kernel out/kernel.elf


