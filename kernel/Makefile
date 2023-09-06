LKM             = sgx-step
# KERNEL_BUILD := $(realpath ../../kernel/linux/build/)
KERNEL_BUILD := /lib/modules/$(shell uname -r)/build

obj-m          += $(LKM).o
$(LKM)-objs     = sgxstep.o

all:
	make -C $(KERNEL_BUILD) M=$(PWD) modules
	$(KERNEL_BUILD)/../scripts/clang-tools/gen_compile_commands.py -d $(KERNEL_BUILD) $(KERNEL_BUILD) .

clean:
	make -C $(KERNEL_BUILD) M=$(PWD) clean
	rm -f compile_commands.json

unload:
	sudo rmmod $(LKM).ko || true

load: unload all
	sudo modprobe -a isgx msr || true
	sudo insmod $(LKM).ko
	sudo dmesg | tail
