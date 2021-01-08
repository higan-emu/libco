#include <stdio.h>
#include <unistd.h>

#include "libco.h"
#include "libco.c"

void cothr_a_job();
void main_cothr_job();

cothread_t main_cothr;
cothread_t program_pseudo;

int main()
{
    program_pseudo = co_active();

    if ((main_cothr = co_create(1024 * sizeof(void *), main_cothr_job))) {
        printf("main cothread started\n");
    } else {
        fprintf(stderr, "failed to create main cothread\n");
    }

    co_switch(main_cothr); // when switched back, the control flow resumes from below this line
    printf("co_active3");

    return 0;
}

void main_cothr_job()
{
    printf("entering main cothread!\n");
    
    cothread_t a;
    if ((a = co_create(1024 * sizeof(void *), cothr_a_job))) {
        printf("thread A started\n");
    } else {
        fprintf(stderr, "failed to create thread A\n");
    }

    co_switch(a);
    printf("back to main cothread\n");
    co_delete(a);
    printf("thread A deleted\n");
    co_switch(program_pseudo);
}

void cothr_a_job()
{
    printf("Hi from cothread A\n");
    co_switch(main_cothr);
    sleep(200);
}
