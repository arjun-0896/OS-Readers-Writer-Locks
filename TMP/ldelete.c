/* ldelete.c */
/* clear te lock datastructures and intimate processes that the lock is no longer available */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <lock.h>
#include <stdio.h>

int ldelete(int ldesc) {
    STATWORD ps;
    struct lentry *lptr;
    int i;
    
    disable(ps);
    if ((ldesc<0 || ldesc > NLOCKS) || locktab[ldesc].lstate == DELETED) {
        restore(ps);
        return (SYSERR);
    }
    lptr = &locktab[ldesc];
    lptr -> lstate = DELETED; //Now this lock is no more available to be use by processes
    lptr -> ltype = UNDEFINED;
    lptr ->lprio = 0;
    lptr -> no_of_readers = 0;
    int head = lptr -> lock_head;
    if (nonempty(head)) {
        int next = q[head].qnext;
        while ((i = next) != EMPTY)
        {
            next = q[next].qnext;
            proctab[i].lock_wait_ret = DELETED; // to wake up other process with deleted lock
            /* To indicate that this process cannot acquire this lock eventhough the descriptor is used by a newly created lock */
            proctab[i].lock_proc_type[ldesc] = DELETED;
            dequeue(i);
            ready(i, RESCHNO);
        }
        resched();
    }
    
    for (i = 1; i < NPROC; i++) {
        if (locktab[ldesc].proc_lock_type[i] == READ || locktab[ldesc].proc_lock_type[i] == WRITE) {
            locktab[ldesc].proc_lock_type[i] = DELETED;
        }
    }
    restore(ps);
    return (OK);
}
