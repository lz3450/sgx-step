LIBSGXSTEP_DIR		= ../..
LIBSGXSTEP			= $(LIBSGXSTEP_DIR)/libsgxstep
-include $(LIBSGXSTEP)/Makefile.config

SUBDIRS				= $(LIBSGXSTEP)

CC					= gcc
AS					= gcc
LD					= gcc

CFLAGS				+= -fPIC -fno-stack-protector -fno-builtin -fno-jump-tables \
                       -fno-common -Wno-attributes -g -D_GNU_SOURCE -O0
INCLUDE				 = -I$(SGX_SDK)/include/ -I$(LIBSGXSTEP_DIR) 
LDFLAGS				+= -lsgx-step -lsgx_urts \
                       -lsgx_uae_service -pthread $(SUBDIRS:%=-L%) -L$(SGX_SDK)/lib64/

SOURCES				= $(wildcard *.c)
OBJECTS				= $(SOURCES:.c=.o)
CMDS				= $(SOURCES:.c=.o.json)
SUBDIR_CMDS			= $(patsubst %.c,%.o.json,$(wildcard $(addsuffix /*.c,$(SUBDIRS))))
OUTPUT				= app

BUILDDIRS			= $(SUBDIRS:%=build-%)
CLEANDIRS			= $(SUBDIRS:%=clean-%)

all : $(OUTPUT) compile_commands.json

$(OUTPUT) : $(BUILDDIRS) $(OBJECTS)
	$(info LD      $(OUTPUT))
	@$(LD) $(OBJECTS) $(LDFLAGS) -o $(OUTPUT)

compile_commands.json : $(BUILDDIRS) $(CMDS)
	$(info GEN     $@)
	@sed -e '1s/^/[\n/' -e '$$ s/,$$/\n]/' $(CMDS) $(SUBDIR_CMDS) > $@
	@sed -E 's|/usr/lib/llvm-[0-9]+/bin/clang|/usr/bin/gcc|' -i $@

%.o : %.c
	$(info CC      $<)
	@$(CC) $(CFLAGS) $(INCLUDE) -c $<

%.o.json : %.c
	@clang $(CFLAGS) $(INCLUDE) -c $< -MJ $@

clean : $(CLEANDIRS)
	$(info RM      $(OBJECTS) $(OUTPUT))
	@rm -f $(OBJECTS) $(OUTPUT)
	$(info RM      $(CMDS) compile_commands.json)
	@rm -f $(CMDS) compile_commands.json

$(BUILDDIRS) :
	$(MAKE) -C $(@:build-%=%)

$(CLEANDIRS) :
	$(MAKE) clean -C $(@:clean-%=%)

run: clean all
	sudo ./app
