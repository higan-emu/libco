#include <libco.h>
#include <stdio.h>
#include <assert.h>

void simple_for_and_finished(void)
{
  int i = 0;
  int* args = co_active()->args0;
  for (i = 0; i < 10; ++i)
  {
    printf("Hello,  acrop_main %d\n", i);
    *args =  i;
    co_yield();
  }
}

static void* acrop_stack[1024];

int main()
{
  cothread_t acrop_work_thread;
  int val = 0;
  int k = 0;
  co_derive(&acrop_work_thread, acrop_stack, sizeof(acrop_stack), simple_for_and_finished, &val);
  for (;;) {
    co_switch(&acrop_work_thread);
    fflush(stdout);
    if (acrop_work_thread.halt == 1){
      break;
    }
    assert(k == val);
    k += 1;
  }

  assert(9 == val);
  return 0;
}
