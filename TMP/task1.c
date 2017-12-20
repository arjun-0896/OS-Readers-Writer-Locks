#include <conf.h>
#include <kernel.h>
#include <sem.h>
#include <lock.h>
#include <proc.h>
#include <stdio.h>
extern unsigned long ctr1000;
int get_curr_prio(int pid);
int sem1;
int lock1;
int procA;
int procB;
int procC;
int procD;
int procE;
int procF;

void samp_proc(void)
{
    unsigned long i = 0, j = 0;
    
    kprintf("%s is started\n", proctab[currpid].pname);
    for (j = 0; j < 2; ++j) {
        sleep1000(2);
        while (++i)
        {
            if (i == 100)
                break;
        }
    }
    
    kprintf("%s priority %d\n", proctab[currpid].pname, get_curr_prio(currpid));
    kprintf("%s is ended\n", proctab[currpid].pname);
    
    return;
}


void sem_proc(void)
{
    unsigned long time1, time2, i = 0, j = 0;
    
    kprintf("%s is started\n", proctab[currpid].pname);
    kprintf("%s to acquire sem\n", proctab[currpid].pname);
    time1 = ctr1000; // Time when process starts waiting on sem1
    wait(sem1);
    time2 = ctr1000; // Time when process acquires the semaphore
    kprintf("%s acquired sem\n", proctab[currpid].pname);
    
    for (j = 0; j < 2; ++j) {
        sleep1000(2);
        while (++i)
        {
            if (i == 100)
                break;
        }
    }
    
    kprintf("%s priority %d\n", proctab[currpid].pname, get_curr_prio(currpid));
    kprintf("%s to release sem\n", proctab[currpid].pname);
    signal(sem1);
    kprintf("%s released sem\n", proctab[currpid].pname);
    kprintf("%s spent %u time waiting on the sem\n", proctab[currpid].pname, time2 - time1);
    kprintf("%s is ended\n", proctab[currpid].pname);
    return;
}

void lock_proc(void)
{
    unsigned long time1, time2, i = 0, j = 0;
    
    kprintf("%s is started\n", proctab[currpid].pname);
    kprintf("%s to acquire lock\n", proctab[currpid].pname);
    time1 = ctr1000; // Time when process starts waiting on lock1
    lock(lock1, WRITE, 50);
    time2 = ctr1000; // Time when process starts waiting on lock1
    kprintf("%s acquired lock\n", proctab[currpid].pname);
    
    for (j = 0; j < 2; ++j) {
        sleep1000(2);
        while (++i)
        {
            if (i == 100)
                break;
        }
    }
    
    kprintf("%s priority %d\n", proctab[currpid].pname, get_curr_prio(currpid));
    kprintf("%s to release lock\n", proctab[currpid].pname);
    releaseall(1, lock1);
    kprintf("%s released lock\n", proctab[currpid].pname);
    kprintf("%s spent %u time waiting on the lock\n", proctab[currpid].pname, time2 - time1);
    kprintf("%s is ended\n", proctab[currpid].pname);
    return;
}

void create_sem_procs()
{
    procA = create((int *) sem_proc, 512, 20, "procA", 0, 0);
    procB = create((int *) samp_proc,512, 25, "procB", 0, 0);
    procC = create((int *) sem_proc, 512, 30, "procC", 0, 0);
    resume(procA);
    resume(procB);
    resume(procC);
    return;
}

void create_lock_procs()
{
    procA = create((int *) lock_proc, 512, 20, "procA", 0, 0);
    procB = create((int *) samp_proc, 512, 25, "procB", 0, 0);
    procC = create((int *) lock_proc, 512, 30, "procC", 0, 0);
    resume(procA);
    resume(procB);
    resume(procC);
    return;
}

void task1(void)
{
    /* Semaphore implementation */
    kprintf("Testing Xinu semaphores\n");
    sem1 = screate(1);
    kprintf("Semaphore is created\n");
    create_sem_procs();
    sleep(1);
    wait(sem1);
    signal(sem1);
    sdelete(sem1);
    kprintf("Semaphore is deleted\n");
    
    sleep(2);
    
    /* Lock Implementation */
    kprintf("\nTesting Readers/Writers Locks\n");
    lock1 = lcreate();
    kprintf("Lock is created\n");
    create_lock_procs();
    sleep(1);
    lock(lock1, WRITE, 50);
    releaseall(1, lock1);
    ldelete(lock1);
    kprintf("Lock is deleted\n");
    
    return;
}
