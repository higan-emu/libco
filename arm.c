#define LIBCO_C
#include "settings.h"

#include <assert.h>
#include <stdlib.h>
#ifdef LIBCO_MPROTECT
  #include <unistd.h>
  #include <sys/mman.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

static thread_local unsigned long co_main_buffer[64];
static thread_local cothread_t co_main_thread;
static thread_local cothread_t co_active_handle = 0;

__attribute__((naked))
void co_swap(cothread_t new_co, cothread_t old_co)
{
  __asm__ (
    "mov r2, sp\n"               // store old sp
    "str r2, [r1]\n"             // here because thumb1 can't store sp directly
    "add r1, 4\n"
    "stmia r1!, {r4-r11,lr}\n"   // store old registers

    "ldr r3, [r0]\n"             // here because thumb1 can't load sp directly
    "mov sp, r3\n"               // recover new sp
    "add r0, 4\n"
    "ldmia r0!, {r4-r11,pc}\n"   // recover new registers
    "bx lr \n"
  );
}

static void co_entrypoint_wrapper(void)
{
  co_active()->entrypoint();
  co_halt();
}

cothread_t co_derive_arg(void* memory, unsigned int size, co_entrypoint entrypoint, void* args0) {
  cothread_t co = NULL;
  if(!co_active_handle) co_active_handle = co_active();

  co = co_derive_init(memory, size, entrypoint, args0);
  {
    unsigned long* handle = co->handle;
    unsigned int offset = (co->size & ~15);
    unsigned long* p = (unsigned long*)((unsigned char*)handle + offset);
    handle[0] = (unsigned long)p;                       // sp stack pointer
    handle[9] = (unsigned long)co_entrypoint_wrapper;   // pc program counter
  }

  return co;
}

#ifdef __cplusplus
}
#endif
