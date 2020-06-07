#if defined(__clang__)
  #pragma clang diagnostic ignored "-Wparentheses"

  /* placing code in section(text) does not mark it executable with Clang. */
  #undef  LIBCO_MPROTECT
  #define LIBCO_MPROTECT
#endif

#include <stddef.h>
#if defined(__clang__) || defined(__GNUC__)
#include <stdint.h>
#endif
#include "libco.h"

#define CO_ALIGN(value,alignment) (((value) + ((alignment) - 1)) & ~((alignment) - 1))
static cothread_t co_derive_init(void* memory, unsigned int size, co_entrypoint entrypoint, void* args0);
static void co_halt();

#if defined(__clang__) || defined(__GNUC__)
  #if defined(__i386__)
    #define CO_STACK_ALIGNMENT 64
    #define CO_STACK_MINIMAL_SIZE 128 * 1024
    #include "x86.c"
  #elif defined(__amd64__)
    #define CO_STACK_ALIGNMENT 64
    #define CO_STACK_MINIMAL_SIZE 128 * 1024
    #include "amd64.c"
  #elif defined(__arm__)
    #define CO_STACK_ALIGNMENT 16
    #define CO_STACK_MINIMAL_SIZE 128
    #include "arm.c"
  #elif defined(__aarch64__)
    #define CO_STACK_ALIGNMENT 32
    #define CO_STACK_MINIMAL_SIZE 256
    #include "aarch64.c"
  #elif defined(__powerpc64__) && defined(_CALL_ELF) && _CALL_ELF == 2
    #define CO_STACK_ALIGNMENT 64
    #define CO_STACK_MINIMAL_SIZE 128 * 1024
    #include "ppc64v2.c"
  #elif defined(_ARCH_PPC) && !defined(__LITTLE_ENDIAN__)
    #define CO_STACK_ALIGNMENT 64
    #define CO_STACK_MINIMAL_SIZE 128 * 1024
    #include "ppc.c"
  #elif defined(_WIN32)
    #define CO_STACK_ALIGNMENT 64
    #define CO_STACK_MINIMAL_SIZE 128 * 1024
    #include "fiber.c"
  #else
    #define CO_STACK_ALIGNMENT 64
    #define CO_STACK_MINIMAL_SIZE 128 * 1024
    #include "sjlj.c"
  #endif
#elif defined(_MSC_VER)
  #if defined(_M_IX86)
    #define CO_STACK_ALIGNMENT 32
    #define CO_STACK_MINIMAL_SIZE 128 * 1024
    #include "x86.c"
  #elif defined(_M_AMD64)
    #define CO_STACK_ALIGNMENT 64
    #define CO_STACK_MINIMAL_SIZE 128 * 1024
    #include "amd64.c"
  #else
    #define CO_STACK_ALIGNMENT 64
    #define CO_STACK_MINIMAL_SIZE 128 * 1024
    #include "fiber.c"
  #endif
#else
  #error "libco: unsupported processor, compiler or operating system"
#endif

static cothread_t co_derive_init(void* memory, unsigned int size, co_entrypoint entrypoint, void* args0) {
  cothread_t co = NULL;
  if (memory != NULL) {
    const unsigned int size_stct = CO_ALIGN(sizeof(co[0]), CO_STACK_ALIGNMENT);
    uintptr_t raw_addr = (uintptr_t)memory;
    uintptr_t raw_end_addr = raw_addr + size;
    uintptr_t base_addr = CO_ALIGN(raw_addr, CO_STACK_ALIGNMENT);
    uintptr_t handle_addr = base_addr + size_stct;

    if (entrypoint == NULL || /* main coroutine do not need check the stack size */
        (handle_addr +  CO_STACK_MINIMAL_SIZE <= raw_end_addr)) {
      co = (void*)base_addr;
      co->handle = (void*)handle_addr;
      co->allocated = NULL;
      co->size = raw_end_addr - handle_addr;
      co->entrypoint = entrypoint;
      co->args0 = args0;
      co->halt = 0;
    }
  }
  return co;
}

static void co_halt() {
  co_active()->halt = 1;
  for (;;) {
    co_switch(co_main());
  }
}

#if !defined(co_main_defined)
cothread_t co_main()
{
  return co_main_thread;
}
#endif

#if !defined(co_active_defined)
cothread_t co_active() {
  if(!co_active_handle) {
    co_main_thread = co_derive_init(co_main_buffer, sizeof(co_main_buffer), NULL, NULL);
    co_active_handle = co_main_thread;
  }
  return co_active_handle;
}
#endif

cothread_t co_derive(void*memory, unsigned int size, co_entrypoint entrypoint) {
  return co_derive_arg(memory, size, entrypoint, NULL);
}

cothread_t co_create(unsigned int size, co_entrypoint entrypoint) {
  return co_create_arg(size, entrypoint, NULL);
}

#if !defined(co_create_defined)
cothread_t co_create_arg(unsigned int size, co_entrypoint entrypoint, void* args0)
{
  cothread_t co = NULL;
  void* memory = malloc(size);
  if(memory)
  {
    co = co_derive_arg(memory, size, entrypoint, args0);
    if (co == NULL)
    {
      co->allocated = memory;
    }
    else
    {
      free(memory);
    }
  }
  return co;
}
#endif


#if !defined(co_delete_defined)
void co_delete(cothread_t handle) {
  if (handle->allocated != NULL)
  {
    free(handle->allocated);
  }
}
#endif


#if !defined(co_switch_defined)
void co_switch(cothread_t handle) {
  if (handle->halt)
  {
    handle = co_main();
  }
  if (handle == co_active_handle)
  {
    return;
  }
  cothread_t co_previous_handle = co_active_handle;
  co_active_handle = handle;
  co_swap(co_active_handle->handle, co_previous_handle->handle);
}
#endif

void co_switch_main() {
  co_switch(co_main());
}

#if !defined(co_serializable_defined)
int co_serializable() {
  return 1;
}
#endif
