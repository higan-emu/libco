#define LIBCO_C
#include "libco.h"
#include "settings.h"

#include <assert.h>
#include <stdlib.h>
#ifdef LIBCO_MPROTECT
  #include <unistd.h>
  #include <sys/mman.h>
#endif

static thread_local unsigned long co_main_buffer[64];
static thread_local cothread_t co_main_thread;
static thread_local cothread_t* co_active_handle = 0;
static void (*co_swap)(cothread_t*, cothread_t*) = 0;

#ifdef LIBCO_MPROTECT
  alignas(4096)
#else
  section(text)
#endif
static const unsigned long co_swap_function[1024] = {
  0xe8a16ff0,  /* stmia r1!, {r4-r11,sp,lr} */
  0xe8b0aff0,  /* ldmia r0!, {r4-r11,sp,pc} */
  0xe12fff1e,  /* bx lr                     */
};

static void co_init() {
  #ifdef LIBCO_MPROTECT
  unsigned long addr = (unsigned long)co_swap_function;
  unsigned long base = addr - (addr % sysconf(_SC_PAGESIZE));
  unsigned long size = (addr - base) + sizeof co_swap_function;
  mprotect((void*)base, size, PROT_READ | PROT_EXEC);
  #endif
}

static void co_entrypoint_wrapper(void)
{
  co_active()->entrypoint();
  co_halt();
}

cothread_err_t co_derive(cothread_t *co, void* memory, unsigned int size, co_entrypoint entrypoint, void* args0) {
  if(!co_swap) {
    co_init();
    co_swap = (void (*)(cothread_t*, cothread_t*))co_swap_function;
  }
  if(!co_active_handle) co_active_handle = co_active();
  co_derive_init(co, memory, size, entrypoint, args0);

  if((unsigned long*)memory) {
    unsigned long* handle = (unsigned long*)memory;
    unsigned int offset = (size & ~15);
    unsigned long* p = (unsigned long*)((unsigned char*)handle + offset);
    handle[8] = (unsigned long)p;
    handle[9] = (unsigned long)co_entrypoint_wrapper;
  }

  return cothread_ok;
}
