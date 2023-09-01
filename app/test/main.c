#include "libsgxstep/idt.h"
#include "libsgxstep/gdt.h"
#include "libsgxstep/apic.h"
#include "libsgxstep/cpu.h"
#include "libsgxstep/sched.h"
#include "libsgxstep/config.h"

#define DO_APIC_SW_IRQ  1
#define DO_APIC_TMR_IRQ 0
#define DO_EXEC_PRIV    0
#define NUM             100
#define INFINITE_LOOP   1
#define NEMESIS_HIGH    1

int             cpl = -1;
uint64_t        flags = 0;
extern uint64_t nemesis_tsc_aex, nemesis_tsc_eresume;

void pre_irq(void)
{
    cpl = get_cpl();
    flags = read_flags();
    __ss_irq_fired = 0;
    nemesis_tsc_eresume = rdtsc_begin();
}

void do_irq_sw(void)
{
    pre_irq();
    asm("int %0\n\t" ::"i"(IRQ_VECTOR)
        :);
}

void do_irq_tmr(void)
{
    pre_irq();
    apic_timer_irq(10);

    /*
     * Ring-0 `exec_priv` handler executes with interrupts disabled FLAGS.IF=0
     * (as the kernel may freeze when interrupting a ring-0 trap gate), so the
     * APIC timer IRQ should be handled after returning.
     */
    if (!(flags & (0x1 << 9)))
        return;

    while (!__ss_irq_fired)
    {
#if NEMESIS_HIGH
        asm("rdrand %rax\n\t");
#else
        asm("nop\n\t");
#endif
    }
}

void post_irq(char *s)
{
    ASSERT(__ss_irq_fired);
    info("returned from %s IRQ: cpl=%d; irq_cpl=%d; flags=%p; count=%02d; nemesis=%d", s,
         cpl, __ss_irq_cpl, flags, __ss_irq_count, nemesis_tsc_aex - nemesis_tsc_eresume);
}

void do_irq_test(int do_exec_priv)
{
#if DO_APIC_SW_IRQ
    printf("\n");
    info_event("Triggering ring-3 software interrupts..");
    for (int i = 0; i < NUM; i++)
    {
        do_irq_sw();
        post_irq("software");
    }

    if (do_exec_priv)
    {
        printf("\n");
        info_event("Triggering ring-0 software interrupts..");
        for (int i = 0; i < NUM; i++)
        {
            cpl = -1;
            exec_priv(do_irq_sw);
            post_irq("software");
        }
    }
#endif

#if DO_APIC_TMR_IRQ
    printf("\n");
    info_event("Triggering ring-3 APIC timer interrupts..");
    apic_timer_oneshot(IRQ_VECTOR);

    for (int i = 0; i < NUM; i++)
    {
        do_irq_tmr();
        post_irq("APIC timer");
    }

    if (do_exec_priv)
    {
        printf("\n");
        info_event("Triggering ring-0 APIC timer interrupts..");
        for (int i = 0; i < NUM; i++)
        {
            cpl = -1;
            exec_priv(do_irq_tmr);
            while (!__ss_irq_fired)
                ;
            post_irq("APIC timer");
        }
    }
    apic_timer_deadline();
#endif
}

int main(int argc, char **argv)
{
    idt_t idt = {0};
    ASSERT(!claim_cpu(VICTIM_CPU));
    map_idt(&idt);

    info_event("Installing and testing ring-0 IDT handler");
    install_kernel_irq_handler(&idt, __ss_irq_handler, IRQ_VECTOR);

#if DO_EXEC_PRIV
    exec_priv(pre_irq);
    info("back from exec_priv(pre_irq) with CPL=%d", cpl);
#endif

#if INFINITE_LOOP
    while (1)
#endif
        do_irq_test(/*do_exec_priv=*/DO_EXEC_PRIV);

    info("all is well; irq_count=%d; exiting..", __ss_irq_count);
    return 0;
}
