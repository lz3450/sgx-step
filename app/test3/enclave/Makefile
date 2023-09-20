CC		= gcc
AS		= gcc
LD		= gcc
EDGER	= sgx_edger8r
SIGNER	= sgx_sign

INCLUDE				= 	-I$(SGX_SDK)/include -I$(SGX_SDK)/include/tlibc
T_CFLAGS			= 	$(CFLAGS) -nostdinc -fvisibility=hidden -fpie -fstack-protector -g -Os
U_CFLAGS			= 	$(CFLAGS) -nostdinc -fvisibility=hidden -fpie -fstack-protector -g
AR_FLAGS			= 	rcs
ENCLAVE_LIBS		=	-lsgx_trts
ENCLAVE_LIB_PARTS	=	-lsgx_tstdc -lsgx_tcrypto -lsgx_tservice
LIB_DIRS			=	-L$(SGX_SDK)/lib64/
LD_FLAGS			+=	-Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles \
						-Wl,--whole-archive -Wl,--start-group $(ENCLAVE_LIBS) -Wl,--end-group \
						-Wl,--no-whole-archive -Wl,--start-group $(ENCLAVE_LIB_PARTS) -Wl,--end-group \
						-Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
						-Wl,-pie,-eenclave_entry -Wl,--export-dynamic  \
						-Wl,--defsym,__ImageBase=0

OBJECTS				= $(ENCLAVE).o
ENCLAVE				= enclave
ENCLAVE_EDL			= $(ENCLAVE).edl
ENCLAVE_CONFIG		= $(ENCLAVE).config.xml
TRUSTED_CODE		= $(ENCLAVE)_t.h $(ENCLAVE)_t.c
UNTRUSTED_CODE		= $(ENCLAVE)_u.h $(ENCLAVE)_u.c
TRUSTED_OBJECTS		= $(ENCLAVE)_t.o
UNTRUSTED_OBJECTS	= $(ENCLAVE)_u.o
CMDS				= $(ENCLAVE).o.json $(ENCLAVE)_t.o.json $(ENCLAVE)_u.o.json
OUTPUT_T			= $(ENCLAVE).so
OUTPUT_T_UNSIG		= $(ENCLAVE).unsigned.so
OUTPUT_U			= lib$(ENCLAVE)_proxy.a

KEY_SIZE		= 3072
PRIVATE_KEY		= private_key.pem
PUBLIC_KEY		= public_key.pem

all: $(OUTPUT_T) $(OUTPUT_U) $(CMDS)

$(CMDS): $(OUTPUT_T) $(OUTPUT_U)

$(OUTPUT_T) : $(TRUSTED_OBJECTS) $(OBJECTS) $(PRIVATE_KEY)
	$(info LD     $@ $(TRUSTED_OBJECTS) $(ENCLAVE_LIBS) $(ENCLAVE_LIBS_PARTS) $(OUTPUT_T_UNSIG))
	@$(LD) $(OBJECTS) $(TRUSTED_OBJECTS) $(LD_FLAGS) $(LIB_DIRS) -o $(OUTPUT_T_UNSIG) 
	$(info SGN    $(OUTPUT_T_UNSIG))
	@$(SIGNER) sign -key $(PRIVATE_KEY) -enclave $(OUTPUT_T_UNSIG) -out $(OUTPUT_T) -config $(ENCLAVE_CONFIG) > /dev/null 2> /dev/null

$(OUTPUT_U) : $(UNTRUSTED_OBJECTS) 
	$(info AR     $@)
	$(AR) $(AR_FLAGS) $(OUTPUT_U) $(UNTRUSTED_OBJECTS)

%_t.o : $(subst .o,.c,$@) edger
	@echo "$(INDENT)[CC] " $(subst .o,.c,$@) "(trusted edge)"
	@touch $(subst .o,.c,$@)
	$(CC) -c $(INCLUDE) $(T_CFLAGS) $(subst .o,.c,$@)
	@clang $(INCLUDE) $(T_CFLAGS) -c $(subst .o,.c,$@) -MJ $(subst .o,.o.json,$@)

%_u.o : $(subst .o,.c,$@) edger
	@echo "$(INDENT)[CC] " $(subst .o,.c,$@) "(untrusted edge)"
	@touch $(subst .o,.c,$@)
	@$(CC) -c $(INCLUDE) $(U_CFLAGS) $(subst .o,.c,$@)
	@clang $(INCLUDE) $(U_CFLAGS) -c $(subst .o,.c,$@) -MJ $(subst .o,.o.json,$@)

%.o : %.c edger
	@echo "$(INDENT)[CC] " $< "(core)"
	@$(CC) $(INCLUDE) $(T_CFLAGS) -c $<
	@clang $(INCLUDE) $(T_CFLAGS) -c $< -MJ $(subst .o,.o.json,$@)

%.o : %.S
	@echo "$(INDENT)[AS] " $< "(core)"
	@$(CC) $(INCLUDE) $(T_CFLAGS) -c $< -o $@

edger: $(ENCLAVE).edl
	$(info GEN	$(EDGER) $(ENCLAVE_EDL))
	@$(EDGER) $(ENCLAVE_EDL)

.PHONY: scrub
scrub:
	$(info RM	$(PRIVATE_KEY) $(PUBLIC_KEY))
	@$(RM) $(PRIVATE_KEY) $(PUBLIC_KEY)

# generate 3072 bit private RSA key
$(PRIVATE_KEY):
	$(info GEN	$(PRIVATE_KEY) ($(KEY_SIZE) bits))
	@openssl genrsa -out $(PRIVATE_KEY) -3 $(KEY_SIZE)
	$(info EXT	$(PUBLIC_KEY))
	@openssl rsa -in $(PRIVATE_KEY) -pubout -out $(PUBLIC_KEY) 

.PHONY: clean
clean:
	$(info RM	$(OBJECTS) $(OUTPUT_T_UNSIG) $(OUTPUT_T) $(OUTPUT_U))
	@$(RM) $(OBJECTS) $(OUTPUT_T_UNSIG) $(OUTPUT_T) $(OUTPUT_U)
	$(info RM	$(TRUSTED_OBJECTS) $(UNTRUSTED_OBJECTS) $(TRUSTED_CODE) $(UNTRUSTED_CODE))
	@$(RM) $(TRUSTED_OBJECTS) $(UNTRUSTED_OBJECTS) $(TRUSTED_CODE) $(UNTRUSTED_CODE)
	$(info RM	$(CMDS))
	@$(RM) $(CMDS)