#include "enclave_t.h"

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
