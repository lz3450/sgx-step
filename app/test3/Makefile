LIBSGXSTEP_DIR		= $(realpath ../..)
LIBSGXSTEP			= $(LIBSGXSTEP_DIR)/libsgxstep
-include $(LIBSGXSTEP)/Makefile.config
ifeq ($(SGX_SDK),)
SGX_SDK				= /opt/intel/sgxsdk
endif

ENCLAVE              = enclave
SUBDIRS				= $(ENCLAVE) $(LIBSGXSTEP)

CC					= gcc
AS					= gcc
LD					= gcc

NUM_RUNS			?=	1
CFLAGS				+=	-fPIC -fno-stack-protector -fno-builtin -fno-jump-tables \
                      	-fno-common -Wno-attributes -g -D_GNU_SOURCE -O0 \
						-DNUM_RUNS=$(NUM_RUNS)
INCLUDE				 =	-I$(SGX_SDK)/include/ -I$(LIBSGXSTEP_DIR)
LDFLAGS				+=	-lsgx-step -lenclave_proxy -lsgx_urts \
						-lsgx_uae_service -pthread $(SUBDIRS:%=-L %) -L$(SGX_SDK)/lib$(LIB_SUFX)/ \
						-L$(LIBSGXSTEP_DIR)/linux-sgx/psw/urts/linux

SOURCES				= $(wildcard *.c)
OBJECTS				= $(SOURCES:.c=.o)
CMDS				= $(SOURCES:.c=.o.json)
SUBDIR_CMDS			= $(patsubst %.c,%.o.json,$(wildcard $(addsuffix /*.c,$(SUBDIRS))))
OUTPUT				= app

BUILDDIRS			= $(SUBDIRS:%=build-%)
CLEANDIRS			= $(SUBDIRS:%=clean-%)

all : $(OUTPUT) compile_commands.json

$(OUTPUT) : $(BUILDDIRS) $(OBJECTS)
	$(info LD	$(OUTPUT))
	@$(LD) $(OBJECTS) $(LDFLAGS) -o $(OUTPUT)

compile_commands.json : $(BUILDDIRS)
	$(info GEN	$@)
	@sed -e '1s/^/[\n/' -e '$$ s/,$$/\n]/' $(CMDS) $(SUBDIR_CMDS) > $@
	@sed -E 's|/usr/lib/llvm-[0-9]+/bin/clang|/usr/bin/gcc|' -i $@

%.o : %.c
	$(info CC	$<)
	$(CC) $(CFLAGS) $(INCLUDE) -c $<
	@clang $(CFLAGS) $(INCLUDE) -c $< -MJ $(addsuffix .json,$@)

clean : $(CLEANDIRS)
	$(info RM	$(OBJECTS) $(OUTPUT))
	@rm -f $(OBJECTS) $(OUTPUT)
	$(info RM	$(CMDS) compile_commands.json)
	@rm -f $(CMDS) compile_commands.json

$(BUILDDIRS) :
	$(MAKE) -C $(@:build-%=%)

$(CLEANDIRS) :
	$(MAKE) clean -C $(@:clean-%=%)

run: all
	sudo ./app
