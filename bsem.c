
#include "types.h"
#include "param.h"
#include "user.h"
#include "uthread.h"
#include "bsem.h"

int bsem_alloc(){
	int* sem = malloc(sizeof(int));
	*sem = 1;
	return (int)sem;
}

void bsem_free(int des){
	free((void*)des);
}

void bsem_down(int des){
	alarm(0);
	//printf(1, "thread %d trying to lock sem\n", uthread_self());
	int* sem = (int*)des;
	if (*sem){
		*sem = 0;
	}
	else{
		uthread_semaphore(0);
	}
	alarm(UTHREAD_QUANTA);
}

void bsem_up(int des){
	alarm(0);
	int* sem = (int*)des;
	*sem = 1;
	uthread_semaphore(-1);
}