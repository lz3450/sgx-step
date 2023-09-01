-include Makefile.config

CC          = gcc
AS          = gcc
AR          = ar
ARFLAGS     = -rcs

ifneq "$(WITH_PSTATE)" "0"
CPPFLAGS     += -DHAS_PSTATE
endif

ifneq "$(WITH_TURBO)" "0"
CPPFLAGS     += -DHAS_TURBO
endif

ifeq ($(M32), 1)
	ASFLAGS = -m32 -DM32=1
	CFLAGS += -m32 -DM32=1
endif

SOURCES     = $(shell ls *.c)
ASM         = $(shell ls *.S)
OBJECTS     = $(SOURCES:.c=.o) $(ASM:.S=.o)
CMDS		= $(SOURCES:.c=.o.json)
OUTPUT      = libsgx-step.a

ifeq ($(GRAMINE), 1)
	CFLAGS += -DSGX_SSAFRAMESIZE=16384
	SOURCES += ./../sdk/gramine/aep.c
endif

all: $(OUTPUT) compile_commands.json

$(OUTPUT): $(OBJECTS)
	$(info AR      $(OUTPUT))
	@$(AR) $(ARFLAGS) $(OUTPUT) $(OBJECTS)

compile_commands.json : $(CMDS)
	$(info GEN     $@)
	@sed -e '1s/^/[\n/' -e '$$ s/,$$/\n]/' $^ > compile_commands.json
	@sed -E 's|/usr/lib/llvm-[0-9]+/bin/clang|/usr/bin/gcc|' -i compile_commands.json

%.o.json : %.c
	@clang $(CPPFLAGS) $(CFLAGS) $(INCLUDE) -c $< -MJ $@

%.o : %.c
	$(info CC      $<)
	@$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDE) -c $< -o $@

%.o : %.S
	$(info AS      $<)
	@$(AS) $(ASFLAGS) $(INCLUDE) -c $< -o $@

clean:
	$(info RM      $(OBJECTS) $(OUTPUT))
	@rm -f $(OBJECTS) $(OUTPUT)
	$(info RM      $(wildcard *.o.json) compile_commands.json)
	@rm -f $(wildcard *.o.json) compile_commands.json
