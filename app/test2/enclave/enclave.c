extern void nop_benchmark(void);

void nops(void)
{
    nop_benchmark();
}

void *get_nop_address(void)
{
    return nop_benchmark;
}
