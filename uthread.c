///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////
////////////////THREADS////////////////
///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////

#include "uthread.h"
#include "user.h"
#include "param.h"

struct thread threads[MAX_UTHREADS];
uint nexttid = 1;
int curr_thread_index = 0;
struct thread *curr_thread;

void
uthread_schedule_wrapper(int signum){
  uint tf;
  asm("movl %%esp, %0;" :"=r"(tf) : :);
  tf+=12;
  uthread_schedule((struct trapframe*)tf);
}

int uthread_init(void){
	struct thread *t;
	
	for(t = threads; t < &threads[MAX_UTHREADS]; t++)
		t->state = T_UNUSED;
	
	t = threads;
	t->tid = nexttid++;
	t->state = T_RUNNING;
	t->tstack = 0;
	t->sleep = 0;
	t->joined = 0;
	
	curr_thread = t;
	signal(SIGALRM, (sighandler_t)uthread_schedule_wrapper);
	sigsend(getpid(), SIGALRM);
	
	return t->tid;
}

int uthread_create(void (*start_func)(void*), void* arg){
	struct thread * t;
	uint sp;
	alarm(0);
	
	for(t = threads; t < &threads[MAX_UTHREADS]; t++)
		if(t->state == T_UNUSED)
		  goto found;

	alarm(UTHREAD_QUANTA);
	return -1;

found:
	t->state = T_EMBRYO;
	t->tid = nexttid++;
	t->tf = curr_thread->tf;
	
	// Allocate thread stack
	if((t->tstack = (uint)malloc(4096)) == 0){
		t->state = T_UNUSED;
		alarm(UTHREAD_QUANTA);
		return -1;
	}
	
	sp = t->tstack + TSTACKSIZE;
	
	// push args
    sp -= 4;
    *(void**)sp = arg;
	
    // return address : thread_exit
    sp -= 4;
    *(void**)sp = uthread_exit;
	
    // initialize thread stack pointers
    t->tf.esp = sp;
    // set threads eip to start_func
    t->tf.eip = (uint)start_func;
	
	t->state = T_RUNNABLE;
	alarm(UTHREAD_QUANTA);
	
	return t->tid;
	
}

void uthread_schedule(struct trapframe* tf){
	alarm(0);
	struct thread * t = curr_thread;
	struct thread * t_chk;
	struct thread * t_jnd;
	struct thread * t_sem;
	int j_found = 0;
	
	if (curr_thread->state == T_SEM_FREE){
		//printf(1, "sem freed by %d\n", curr_thread->tid);
		for(t_sem = threads; t_sem < &threads[MAX_UTHREADS]; t_sem++){
			if (t_sem->state == T_SEM_WAIT){
				//printf(1, "sem freed by %d and mov to %d\n", curr_thread->tid, t_sem->tid);
				t_sem->state = T_RUNNABLE;
				break;
			}
		}
		curr_thread->state = T_RUNNING;
	}
	
	// Checking sleep and join threads
	for(t_chk = threads; t_chk < &threads[MAX_UTHREADS]; t_chk++){
		j_found = 0;
		if (t_chk->state == T_SLEEPING){
			if(t_chk->sleep <= uptime()){
				if(t_chk->joined > 0){
					t_chk->state = T_JOINED;
				}
				else{
					t_chk->state = T_RUNNABLE;
				}
			}
		}
		if (t_chk->state == T_JOINED){
			for(t_jnd = threads; t_jnd < &threads[MAX_UTHREADS]; t_jnd++){
				if(t_chk->joined == t_jnd->tid && t_jnd->tid != T_TERMINATED)
					j_found = 1;
			}
			if (!j_found)
				t_chk->state = T_RUNNABLE;
		}
	}
	
	// If the current thread is running, joined, waiting for semaphore or just went to sleep
	if(t->state == T_RUNNING || t->state == T_SLEEPING || 
		t->state == T_JOINED || t->state == T_SEM_WAIT){
		memmove((void*)&t->tf, tf, sizeof(struct trapframe));
		if(t->state == T_RUNNING)
			t->state = T_RUNNABLE;
	}
	
	// Giving the next thread control in round robin
	t++;
	while(t->state != T_RUNNABLE && t != curr_thread){
		t++;
		if(t >= &threads[MAX_UTHREADS])
			t = threads;
	}	  
	memmove(tf, (void*)&t->tf, sizeof(struct trapframe));

	if(curr_thread->state == T_UNUSED && curr_thread->tstack)
		free((void*)curr_thread->tstack);
	
	curr_thread = t;
	
	if(t->state != T_RUNNABLE){
		if(t->tstack)
		 free((void*)t->tstack);
		exit();
	}
	
	curr_thread->state = T_RUNNING;
	//printf(1, "Thread %d is now running\n", curr_thread->tid);
	//for(t_chk = threads; t_chk < &threads[MAX_UTHREADS]; t_chk++)
	//	printf(1, "%d|", t_chk->state);
	//printf(1, "\n");
	alarm(UTHREAD_QUANTA);
	return;
}

void uthread_exit(void){
	alarm(0);
	struct thread* t;
	
	if (curr_thread->tid == 1){
		printf(1, "Main thread exit\n");
		exit();
	}
	
	curr_thread->state = T_TERMINATED;
	
	t = curr_thread;
	t++;
	while(t->state != T_RUNNABLE && t->tid != curr_thread->tid){
		t++;
		if(t >= &threads[MAX_UTHREADS])
			t = threads;
	}	  
	
	if (t->state == T_RUNNABLE){
		goto threads_remaining;
	}
	else{
		if(curr_thread->tstack)
		  free((void*)curr_thread->tstack);
		exit();
	}
	
threads_remaining:	  	
	
	curr_thread->state = T_UNUSED;
	curr_thread->tid = -1;
	curr_thread->tstack = 0;
	sigsend(getpid(), SIGALRM);

	alarm(UTHREAD_QUANTA);	
}

int uthread_self(void){
	return curr_thread->tid;
}

int uthread_join(int tid){
	alarm(0);
	struct thread* t;
	int found = 0;
	if (curr_thread){
		for(t = threads; !found && t < &threads[MAX_UTHREADS]; t++){
			if(t->tid == tid){
				found = 1;
				break;
			}
		}
		if(found){
			curr_thread->joined = tid;
			curr_thread->state = T_JOINED;
		}
	}
	sigsend(getpid(), SIGALRM);
	return 0;
}

int uthread_sleep(int ticks){
	int temp_sleep = uptime() + ticks;
	curr_thread->sleep = temp_sleep;
	curr_thread->state = T_SLEEPING;
	sigsend(getpid(), SIGALRM);
	return 0;
}


void uthread_semaphore(int locked){
	alarm(0);
	if (locked == -1){
		curr_thread->state = T_SEM_FREE;
		sigsend(getpid(), SIGALRM);
	}
	else if (locked == 0){
		curr_thread->state = T_SEM_WAIT;
		sigsend(getpid(), SIGALRM);
	}
}

///////////////////////////////////////
//////////////END-THREADS//////////////
///////////////////////////////////////