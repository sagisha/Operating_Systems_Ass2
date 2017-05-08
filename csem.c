
#include "types.h"
#include "param.h"
#include "user.h"
#include "uthread.h"
#include "bsem.h"
#include "csem.h"

void down(struct counting_semaphore *sem){
	bsem_down(sem->s2);
	bsem_down(sem->s1);
	sem->value--;
	if(sem->value > 0)
		bsem_up(sem->s2);
	bsem_up(sem->s1);
	
}

void up(struct counting_semaphore *sem){
	bsem_down(sem->s1);
	sem->value++;
	if(sem->value == 1)
		bsem_up(sem->s2);
	bsem_up(sem->s1);
}

void csem_init(struct counting_semaphore *sem, int count){
	sem->s1 = bsem_alloc();
	*((int*)sem->s1) = 1;
	sem->s2 = bsem_alloc();
	if (count){
		*((int*)sem->s2) = 1;
	}
	else{
		*((int*)sem->s2) = 0;
	}
	sem->value = 1;
}

void csem_free(struct counting_semaphore *sem){
	bsem_free(sem->s1);
	bsem_free(sem->s2);
}