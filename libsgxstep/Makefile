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
OUTPUT      = libsgx-step.a

ifeq ($(GRAMINE), 1)
	CFLAGS += -DSGX_SSAFRAMESIZE=16384
	SOURCES += ./../sdk/gramine/aep.c
endif

all: $(OUTPUT) compile_commands.json

$(OUTPUT): $(OBJECTS)
	echo "$(INDENT)[AR] " $(OUTPUT)
	$(AR) $(ARFLAGS) $(OUTPUT) $(OBJECTS)

compile_commands.json : $(patsubst %.c, %.o.json, $(SOURCES))
	sed -e '1s/^/[\n/' -e '$$ s/,$$/\n]/' -e 's|/usr/lib/llvm-12/bin/clang|/usr/bin/gcc|' $^ > compile_commands.json

%.o.json : %.c
	clang $(CPPFLAGS) $(CFLAGS) $(INCLUDE) -c $< -MJ $@

%.o : %.c
	echo "$(INDENT)[CC] " $<
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDE) -c $< -o $@

%.o : %.S
	echo "$(INDENT)[AS] " $<
	$(AS) $(ASFLAGS) $(INCLUDE) -c $< -o $@

clean:
	echo "$(INDENT)[RM]" $(OBJECTS) $(OUTPUT)
	rm -f $(OBJECTS) $(OUTPUT)
	rm -f $(wildcard *.o.json) compile_commands.json
