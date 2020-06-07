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
typedef struct cothread_stct cothread_stct;
typedef cothread_stct* cothread_t;

cothread_t co_main();
cothread_t co_active();
cothread_t co_derive(void*, unsigned int, void (*)(void));
cothread_t co_create(unsigned int, void (*)(void));
cothread_t co_derive_arg(void*, unsigned int, co_entrypoint, void* args0);
cothread_t co_create_arg(unsigned int, co_entrypoint, void* args0);
void co_delete(cothread_t);
void co_switch(cothread_t);
void co_switch_main();
int co_serializable();

#ifdef __cplusplus
}
#endif

/* ifndef LIBCO_H */
#endif
