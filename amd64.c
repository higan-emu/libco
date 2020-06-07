#define LIBCO_C
#include "settings.h"

#include <assert.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

static thread_local long long co_main_buffer[64];
static thread_local cothread_t co_main_thread;
static thread_local cothread_t co_active_handle = 0;
static void (*co_swap)(cothread_t, cothread_t) = 0;

#ifdef LIBCO_MPROTECT
  alignas(4096)
#else
#pragma code_seg(".text")
__declspec(allocate(".text"))
#endif
#ifdef _WIN32
  /* ABI: Win64 */
  static const unsigned char co_swap_function[4096] = {
    0x48, 0x89, 0x22,              /* mov [rdx],rsp          */
    0x48, 0x8b, 0x21,              /* mov rsp,[rcx]          */
    0x58,                          /* pop rax                */
    0x48, 0x89, 0x6a, 0x08,        /* mov [rdx+ 8],rbp       */
    0x48, 0x89, 0x72, 0x10,        /* mov [rdx+16],rsi       */
    0x48, 0x89, 0x7a, 0x18,        /* mov [rdx+24],rdi       */
    0x48, 0x89, 0x5a, 0x20,        /* mov [rdx+32],rbx       */
    0x4c, 0x89, 0x62, 0x28,        /* mov [rdx+40],r12       */
    0x4c, 0x89, 0x6a, 0x30,        /* mov [rdx+48],r13       */
    0x4c, 0x89, 0x72, 0x38,        /* mov [rdx+56],r14       */
    0x4c, 0x89, 0x7a, 0x40,        /* mov [rdx+64],r15       */
  #if !defined(LIBCO_NO_SSE)
    0x0f, 0x29, 0x72, 0x50,        /* movaps [rdx+ 80],xmm6  */
    0x0f, 0x29, 0x7a, 0x60,        /* movaps [rdx+ 96],xmm7  */
    0x44, 0x0f, 0x29, 0x42, 0x70,  /* movaps [rdx+112],xmm8  */
    0x48, 0x83, 0xc2, 0x70,        /* add rdx,112            */
    0x44, 0x0f, 0x29, 0x4a, 0x10,  /* movaps [rdx+ 16],xmm9  */
    0x44, 0x0f, 0x29, 0x52, 0x20,  /* movaps [rdx+ 32],xmm10 */
    0x44, 0x0f, 0x29, 0x5a, 0x30,  /* movaps [rdx+ 48],xmm11 */
    0x44, 0x0f, 0x29, 0x62, 0x40,  /* movaps [rdx+ 64],xmm12 */
    0x44, 0x0f, 0x29, 0x6a, 0x50,  /* movaps [rdx+ 80],xmm13 */
    0x44, 0x0f, 0x29, 0x72, 0x60,  /* movaps [rdx+ 96],xmm14 */
    0x44, 0x0f, 0x29, 0x7a, 0x70,  /* movaps [rdx+112],xmm15 */
  #endif
    0x48, 0x8b, 0x69, 0x08,        /* mov rbp,[rcx+ 8]       */
    0x48, 0x8b, 0x71, 0x10,        /* mov rsi,[rcx+16]       */
    0x48, 0x8b, 0x79, 0x18,        /* mov rdi,[rcx+24]       */
    0x48, 0x8b, 0x59, 0x20,        /* mov rbx,[rcx+32]       */
    0x4c, 0x8b, 0x61, 0x28,        /* mov r12,[rcx+40]       */
    0x4c, 0x8b, 0x69, 0x30,        /* mov r13,[rcx+48]       */
    0x4c, 0x8b, 0x71, 0x38,        /* mov r14,[rcx+56]       */
    0x4c, 0x8b, 0x79, 0x40,        /* mov r15,[rcx+64]       */
  #if !defined(LIBCO_NO_SSE)
    0x0f, 0x28, 0x71, 0x50,        /* movaps xmm6, [rcx+ 80] */
    0x0f, 0x28, 0x79, 0x60,        /* movaps xmm7, [rcx+ 96] */
    0x44, 0x0f, 0x28, 0x41, 0x70,  /* movaps xmm8, [rcx+112] */
    0x48, 0x83, 0xc1, 0x70,        /* add rcx,112            */
    0x44, 0x0f, 0x28, 0x49, 0x10,  /* movaps xmm9, [rcx+ 16] */
    0x44, 0x0f, 0x28, 0x51, 0x20,  /* movaps xmm10,[rcx+ 32] */
    0x44, 0x0f, 0x28, 0x59, 0x30,  /* movaps xmm11,[rcx+ 48] */
    0x44, 0x0f, 0x28, 0x61, 0x40,  /* movaps xmm12,[rcx+ 64] */
    0x44, 0x0f, 0x28, 0x69, 0x50,  /* movaps xmm13,[rcx+ 80] */
    0x44, 0x0f, 0x28, 0x71, 0x60,  /* movaps xmm14,[rcx+ 96] */
    0x44, 0x0f, 0x28, 0x79, 0x70,  /* movaps xmm15,[rcx+112] */
  #endif
    0xff, 0xe0,                    /* jmp rax                */
  };

  #include <windows.h>

  static void co_init() {
    #ifdef LIBCO_MPROTECT
    DWORD old_privileges;
    VirtualProtect((void*)co_swap_function, sizeof co_swap_function, PAGE_EXECUTE_READ, &old_privileges);
    #endif
  }
#else
  /* ABI: SystemV */
  static const unsigned char co_swap_function[4096] = {
    0x48, 0x89, 0x26,        /* mov [rsi],rsp    */
    0x48, 0x8b, 0x27,        /* mov rsp,[rdi]    */
    0x58,                    /* pop rax          */
    0x48, 0x89, 0x6e, 0x08,  /* mov [rsi+ 8],rbp */
    0x48, 0x89, 0x5e, 0x10,  /* mov [rsi+16],rbx */
    0x4c, 0x89, 0x66, 0x18,  /* mov [rsi+24],r12 */
    0x4c, 0x89, 0x6e, 0x20,  /* mov [rsi+32],r13 */
    0x4c, 0x89, 0x76, 0x28,  /* mov [rsi+40],r14 */
    0x4c, 0x89, 0x7e, 0x30,  /* mov [rsi+48],r15 */
    0x48, 0x8b, 0x6f, 0x08,  /* mov rbp,[rdi+ 8] */
    0x48, 0x8b, 0x5f, 0x10,  /* mov rbx,[rdi+16] */
    0x4c, 0x8b, 0x67, 0x18,  /* mov r12,[rdi+24] */
    0x4c, 0x8b, 0x6f, 0x20,  /* mov r13,[rdi+32] */
    0x4c, 0x8b, 0x77, 0x28,  /* mov r14,[rdi+40] */
    0x4c, 0x8b, 0x7f, 0x30,  /* mov r15,[rdi+48] */
    0xff, 0xe0,              /* jmp rax          */
  };

  #ifdef LIBCO_MPROTECT
    #include <unistd.h>
    #include <sys/mman.h>
  #endif

  static void co_init() {
    #ifdef LIBCO_MPROTECT
    unsigned long long addr = (unsigned long long)co_swap_function;
    unsigned long long base = addr - (addr % sysconf(_SC_PAGESIZE));
    unsigned long long size = (addr - base) + sizeof co_swap_function;
    mprotect((void*)base, size, PROT_READ | PROT_EXEC);
    #endif
  }
#endif

cothread_t co_derive_arg(void* memory, unsigned int size, co_entrypoint entrypoint, void* args0) {
  cothread_t co = NULL;
  if(!co_swap) {
    co_init();
    co_swap = (void (*)(cothread_t, cothread_t))co_swap_function;
  }
  if(!co_active_handle) co_active_handle = co_active();

  co = co_derive_init(memory, size, entrypoint, args0);
  if(co) {
    void* handle = co->handle;
    unsigned int offset = (co->size & ~15) - 32;
    long long *p = (long long*)((char*)handle + offset);  /* seek to top of stack */
    *--p = (long long)co_halt;                            /* halt if entrypoint returns */
    *--p = (long long)entrypoint;                         /* start of function */
    *(long long*)handle = (long long)p;                   /* stack pointer */
  }

  return co;
}

#ifdef __cplusplus
}
#endif
