/*
  libco v20 (2019-10-16)
  author: byuu
  license: ISC
*/

#ifndef LIBCO_H
#define LIBCO_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*co_entrypoint)(void);

struct cothread_stct
{
  void* handle;
  void *allocated;
  unsigned int size;
  co_entrypoint entrypoint;
  void *args0;
  int halt;
};
typedef struct cothread_stct cothread_t;

enum cothread_err_t
{
  cothread_ok = 0,
  cothread_malloc_failed = -1,
  cothread_derive_failed = -2
};
typedef enum cothread_err_t cothread_err_t;

cothread_t* co_active();
cothread_t* co_main();
cothread_err_t co_derive(cothread_t *co, void* memory, unsigned int size, co_entrypoint entrypoint, void* args0);
cothread_err_t co_create(cothread_t *co, unsigned int size, co_entrypoint entrypoint, void* args0);
void co_delete(cothread_t *co);
void co_switch(cothread_t *co);
void co_yield();
int co_serializable();

#ifdef __cplusplus
}
#endif

/* ifndef LIBCO_H */
#endif
