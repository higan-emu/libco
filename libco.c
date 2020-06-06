#if defined(__clang__)
  #pragma clang diagnostic ignored "-Wparentheses"

  /* placing code in section(text) does not mark it executable with Clang. */
  #undef  LIBCO_MPROTECT
  #define LIBCO_MPROTECT
#endif

#include <stddef.h>
#include "libco.h"

static void co_derive_init(cothread_t *co, void* memory, unsigned int size, co_entrypoint entrypoint, void* args0) {
  co->handle = memory;
  co->allocated = NULL;
  co->size = size;
  co->entrypoint = entrypoint;
  co->args0 = args0;
  co->halt = 0;
}

static void co_halt() {
  co_active()->halt = 1;
  for (;;) {
    co_switch(co_main());
  }
}

#if defined(__clang__) || defined(__GNUC__)
  #if defined(__i386__)
    #include "x86.c"
  #elif defined(__amd64__)
    #include "amd64.c"
  #elif defined(__arm__)
    #include "arm.c"
  #elif defined(__aarch64__)
    #include "aarch64.c"
  #elif defined(__powerpc64__) && defined(_CALL_ELF) && _CALL_ELF == 2
    #include "ppc64v2.c"
  #elif defined(_ARCH_PPC) && !defined(__LITTLE_ENDIAN__)
    #include "ppc.c"
  #elif defined(_WIN32)
    #include "fiber.c"
  #else
    #include "sjlj.c"
  #endif
#elif defined(_MSC_VER)
  #if defined(_M_IX86)
    #include "x86.c"
  #elif defined(_M_AMD64)
    #include "amd64.c"
  #else
    #include "fiber.c"
  #endif
#else
  #error "libco: unsupported processor, compiler or operating system"
#endif

#if !defined(co_active_defined)
cothread_t* co_active() {
  if(!co_active_handle) {
    co_derive_init(&co_main_thread, co_main_buffer, sizeof(co_main_buffer), NULL, NULL);
    co_active_handle = &co_main_thread;
  }
  return co_active_handle;
}
#endif

#if !defined(co_main_defined)
cothread_t* co_main()
{
  return &co_main_thread;
}
#endif

#if !defined(co_create_defined)
cothread_err_t co_create(cothread_t *co, unsigned int size, co_entrypoint entrypoint, void* args0)
{
  cothread_err_t err = cothread_malloc_failed;
  void* memory = malloc(size);
  if(memory)
  {
    err = co_derive(co, memory, size, entrypoint, args0);
    if (err == cothread_ok)
    {
      co->allocated = memory;
    }
    else
    {
      free(memory);
    }
  }
  return err;
}
#endif

#if !defined(co_delete_defined)
void co_delete(cothread_t* handle) {
  if (handle->allocated != NULL)
  {
    free(handle->allocated);
  }
}
#endif


#if !defined(co_switch_defined)
void co_switch(cothread_t* handle) {
  cothread_t *co_previous_handle = co_active_handle;
  co_active_handle = handle;
  co_swap(co_active_handle->handle, co_previous_handle->handle);
}
#endif

void co_yield() {
  co_switch(co_main());
}

#if !defined(co_serializable_defined)
int co_serializable() {
  return 1;
}
#endif
