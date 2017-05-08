struct counting_semaphore{
	int s1;
	int s2;
	int value;
};
//struct counting_semaphore;

void down(struct counting_semaphore *sem);
void up(struct counting_semaphore *sem);
void csem_init(struct counting_semaphore *sem, int count);
void csem_free(struct counting_semaphore *sem);