#include "types.h"
#include "x86.h"


enum threadtate { T_UNUSED, T_EMBRYO, T_RUNNING, T_RUNNABLE, T_JOINED, T_SLEEPING, T_SEM_WAIT, T_SEM_FREE, T_TERMINATED };

// Per-thread state
struct thread {
  uint tstack;                					// Bottom of thread stack for this thread
  enum threadtate state;        				// Thread state
  uint tid;                     					// Thread ID
  struct proc *parent;         					// Parent process
  struct thread *tparent;         				// Parent thread
  struct trapframe tf;        					// Trap frame for current syscall
  struct context *context;     					// swtch() here to run thread
  void *chan;                  					// If non-zero, sleeping on chan
  int killed;                  					// If non-zero, have been killed
  int sleep;
  int joined;
};

int uthread_init(void);
void uthread_schedule(struct trapframe*);
void uthread_exit(void);
int uthread_self(void);
int uthread_join(int);
int uthread_sleep(int);
int uthread_create(void (*start_func)(void*), void* arg);
void uthread_semaphore(int locked);



