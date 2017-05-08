#include "types.h"
#include "user.h"
#include "stat.h"
#include "uthread.h"
#include "bsem.h"
#include "csem.h"

int size = 1000;
int nums[1000];
int j = 0;
struct counting_semaphore* sem;

int getNum(){
	return j++;
}

void producer(void *arg) {
  int i;
  for(i = 0; i < size; i++) {
    nums[i] = i + 1;
  }
}


void customer(void *arg) {
	while (j < size){
		down(sem);
		int x = getNum();
		up(sem);
		x = nums[x];
		uthread_sleep(x);
		printf(1, "Thread %d slept for %d ticks\n", uthread_self(), x);
		if (j > size){
			uthread_exit();
		}
		if (j == size){
			uthread_sleep(size + 100);
			csem_free(sem);
			exit();
		}
	}
}

int main(int argc, char *argv[]) {
	int i;
	for (i = 0; i< size; i++)
		nums[i] = -1;
		
	csem_init(sem, 1);
	
	printf(1,"sem s1: %d\n", *((int*)sem->s1));
	printf(1,"sem s2: %d\n", *((int*)sem->s2));
	printf(1,"sem value: %d\n", sem->value);
	
	uthread_init();
	uthread_create(producer, (void*)555);
	uthread_create(customer, (void*)555);
	uthread_create(customer, (void*)555);
	uthread_create(customer, (void*)555);
	
	while(1);

	
	csem_free(sem);
	exit();
}