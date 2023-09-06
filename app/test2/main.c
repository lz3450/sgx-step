#include <sys/mman.h>
#include <sgx_urts.h>
#include "enclave/enclave_u.h"
#include <signal.h>
#include <unistd.h>
#include "libsgxstep/apic.h"
#include "libsgxstep/pt.h"
#include "libsgxstep/sched.h"
#include "libsgxstep/enclave.h"
#include "libsgxstep/debug.h"
#include "libsgxstep/config.h"
#include "libsgxstep/idt.h"
#include "libsgxstep/config.h"

#ifndef NUM_RUNS
#define NUM_RUNS 100
#endif

sgx_enclave_id_t eid = 0;
int              strlen_nb_access = 0;
int              irq_cnt = 0, do_irq = 1, fault_cnt = 0;
uint64_t        *pte_enclave = NULL;
uint64_t        *pte_str_enclave = NULL;
uint64_t        *pmd_enclave = NULL;
uint64_t         cycles_cnt;

/* Called upon SIGSEGV caused by untrusted page tables. */
void fault_handler(int signal)
{
    info("Caught fault %d! Restoring enclave page permissions..", signal);
    *pte_enclave = MARK_NOT_EXECUTE_DISABLE(*pte_enclave);
    ASSERT(fault_cnt++ < 10);

    // NOTE: return eventually continues at aep_cb_func and initiates
    // single-stepping mode.
}

/* Called before resuming the enclave after an Asynchronous Enclave eXit. */
void aep_cb_func(void)
{
    uint64_t erip = edbgrd_erip() - (uint64_t)get_enclave_base();
    cycles_cnt = nemesis_tsc_aex - nemesis_tsc_eresume;
    info("^^ enclave RIP=%#llx; ACCESSED=%d, cycles=%d", erip, ACCESSED(*pte_enclave), cycles_cnt);
    irq_cnt++;

    if (do_irq && (irq_cnt > NUM_RUNS * 500))
    {
        info("excessive interrupt rate detected (try adjusting timer interval "
             "to avoid getting stuck in zero-stepping); aborting...");
        do_irq = 0;
    }

    /*
     * NOTE: We explicitly clear the "accessed" bit of the _unprotected_ PTE
     * referencing the enclave code page about to be executed, so as to be able
     * to filter out "zero-step" results that won't set the accessed bit.
     */
    *pte_enclave = MARK_NOT_ACCESSED(*pte_enclave);

    /*
     * Configure APIC timer interval for next interrupt.
     *
     * On our evaluation platforms, we explicitly clear the enclave's
     * _unprotected_ PMD "accessed" bit below, so as to slightly slow down
     * ERESUME such that the interrupt reliably arrives in the first subsequent
     * enclave instruction.
     *
     */
    if (do_irq)
    {
        *pmd_enclave = MARK_NOT_ACCESSED(*pmd_enclave);
        apic_timer_irq(SGX_STEP_TIMER_INTERVAL);
    }
}

/* Configure and check attacker untrusted runtime environment. */
void attacker_config_runtime(void)
{
    ASSERT(!claim_cpu(VICTIM_CPU));
    ASSERT(!prepare_system_for_benchmark(PSTATE_PCT));
    ASSERT(signal(SIGSEGV, fault_handler) != SIG_ERR);
    print_system_settings();

    if (isatty(fileno(stdout)))
    {
        info("WARNING: interactive terminal detected; known to cause ");
        info("unstable timer intervals! Use stdout file redirection for ");
        info("precise single-stepping results...");
    }

    register_enclave_info();
    print_enclave_info();
}

/* Provoke page fault on enclave entry to initiate single-stepping mode. */
void attacker_config_page_table(void)
{
    void *code_address;

    SGX_ASSERT(get_nop_address(eid, &code_address));

    // print_page_table(code_address);
    info("enclave trigger code adrs at %p\n", code_address);
    ASSERT(pte_enclave = remap_page_table_level(code_address, PTE));
#if SINGLE_STEP_ENABLE
    *pte_enclave = MARK_EXECUTE_DISABLE(*pte_enclave);
    print_pte(pte_enclave);
    ASSERT(PRESENT(*pte_enclave));
#endif

    // print_page_table(get_enclave_base());
    ASSERT(pmd_enclave = remap_page_table_level(get_enclave_base(), PMD));
    ASSERT(PRESENT(*pmd_enclave));
}

int main(int argc, char **argv)
{
    sgx_launch_token_t token = {0};
    int                apic_fd, encl_strlen = 0, updated = 0, vec = 0;
    idt_t              idt = {0};

    info_event("Creating enclave...");
    SGX_ASSERT(sgx_create_enclave("./enclave/enclave.so", 1,
                                  &token, &updated, &eid, NULL));

    /* 0. dry run */
    info("Dry run to allocate pages");
    SGX_ASSERT(nops(eid));

    /* 1. Setup attack execution environment. */
    attacker_config_runtime();
    attacker_config_page_table();
    register_aep_cb(aep_cb_func);

    info_event("Establishing user-space APIC/IDT mappings");
    map_idt(&idt);
    install_kernel_irq_handler(&idt, __ss_irq_handler, IRQ_VECTOR);
    apic_timer_oneshot(IRQ_VECTOR);

    /* 2. Single-step enclaved execution. */
    info_event("calling enclave: num_runs=%d; timer=%d", NUM_RUNS, SGX_STEP_TIMER_INTERVAL);
    SGX_ASSERT(nops(eid));

    /* 3. Restore normal execution environment. */
    SGX_ASSERT(sgx_destroy_enclave(eid));

    info_event("all done; counted %d/%d IRQs (AEP/IDT)", irq_cnt, __ss_irq_count);
    return 0;
}