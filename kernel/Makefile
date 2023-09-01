LKM             = sgx-step
KERNEL_BUILD := $(realpath ../../kernel/linux/build/)

obj-m          += $(LKM).o
$(LKM)-objs     = sgxstep.o

all:
	make -C $(KERNEL_BUILD) M=$(PWD) modules
	$(KERNEL_BUILD)/../scripts/clang-tools/gen_compile_commands.py -d $(KERNEL_BUILD) $(KERNEL_BUILD) .

clean:
	make -C $(KERNEL_BUILD) M=$(PWD) clean
	rm -f compile_commands.json
