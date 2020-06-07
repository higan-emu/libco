#define LIBCO_C
#include "settings.h"

#include <assert.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__clang__) || defined(__GNUC__)
  #define fastcall __attribute__((fastcall))
#elif defined(_MSC_VER)
  #define fastcall __fastcall
#else
  #error "libco: please define fastcall macro"
#endif

static thread_local long co_main_buffer[64];
static thread_local cothread_t co_main_thread = 0;
static thread_local cothread_t co_active_handle = 0;
static void (fastcall *co_swap)(cothread_t, cothread_t) = 0;

#ifdef LIBCO_MPROTECT
  alignas(4096)
#else
#pragma code_seg(".text")
__declspec(allocate(".text"))
#endif
/* ABI: fastcall */
static const unsigned char co_swap_function[4096] = {
  0x89, 0x22,        /* mov [edx],esp    */
  0x8b, 0x21,        /* mov esp,[ecx]    */
  0x58,              /* pop eax          */
  0x89, 0x6a, 0x04,  /* mov [edx+ 4],ebp */
  0x89, 0x72, 0x08,  /* mov [edx+ 8],esi */
  0x89, 0x7a, 0x0c,  /* mov [edx+12],edi */
  0x89, 0x5a, 0x10,  /* mov [edx+16],ebx */
  0x8b, 0x69, 0x04,  /* mov ebp,[ecx+ 4] */
  0x8b, 0x71, 0x08,  /* mov esi,[ecx+ 8] */
  0x8b, 0x79, 0x0c,  /* mov edi,[ecx+12] */
  0x8b, 0x59, 0x10,  /* mov ebx,[ecx+16] */
  0xff, 0xe0,        /* jmp eax          */
};

#ifdef _WIN32
  #include <windows.h>

  static void co_init() {
    #ifdef LIBCO_MPROTECT
    DWORD old_privileges;
    VirtualProtect((void*)co_swap_function, sizeof co_swap_function, PAGE_EXECUTE_READ, &old_privileges);
    #endif
  }
#else
  #ifdef LIBCO_MPROTECT
    #include <unistd.h>
    #include <sys/mman.h>
  #endif

  static void co_init() {
    #ifdef LIBCO_MPROTECT
    unsigned long addr = (unsigned long)co_swap_function;
    unsigned long base = addr - (addr % sysconf(_SC_PAGESIZE));
    unsigned long size = (addr - base) + sizeof co_swap_function;
    mprotect((void*)base, size, PROT_READ | PROT_EXEC);
    #endif
  }
#endif

cothread_t co_derive_arg(void* memory, unsigned int size, co_entrypoint entrypoint, void* args0) {
  cothread_t co;
  if(!co_swap) {
    co_init();
    co_swap = (void (fastcall*)(cothread_t, cothread_t))co_swap_function;
  }
  if(!co_active_handle) co_active_handle = co_active();

  co = co_derive_init(memory, size, entrypoint, args0);
  if(co) {
    void* handle= co->handle;
    unsigned int offset = (co->size & ~15) - 32;
    long *p = (long*)((char*)handle + offset);  /* seek to top of stack */
    *--p = (long)co_halt;                       /* halt if entrypoint returns */
    *--p = (long)entrypoint;                    /* start of function */
    *(long*)handle = (long)p;                   /* stack pointer */
  }

  return co;
}

#ifdef __cplusplus
}
#endif
