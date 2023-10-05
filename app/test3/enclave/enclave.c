#include "enclave_t.h"

extern void nop_benchmark(void);

void nops(void)
{
    nop_benchmark();
}

void *get_nop_address(void)
{
    return nop_benchmark;
}

void ecall_profile_aex(void)
{
    for (int i = 0; i < 100000; i++)
    {
        asm volatile("nop\n\t");
    }
}

void *get_ecall_profile_aex(void)
{
    return ecall_profile_aex;
}
