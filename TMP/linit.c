
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <mark.h>
#include <bufpool.h>
#include <proc.h>
#include <sem.h>
#include <sleep.h>
#include <mem.h>
#include <tty.h>
#include <q.h>
#include <io.h>
#include <stdio.h>
#include <lock.h>

struct lentry locktab[NLOCKS];
void linit()
{
    int i;
    struct lentry* lptr;
    int pid;
    for (i=0 ; i<NLOCKS ; i++)
    {
        lptr = &locktab[i];
        lptr -> lprio = -1;
        lptr -> lstate = DELETED;
        lptr->ltype = UNDEFINED;
        lptr->no_of_readers = 0;
        lptr->lock_head= newqueue(); /* queue to hold the processes waiting for the lock */
        lptr->lock_tail = 1 + lptr -> lock_head;
        for(pid=0;pid<NPROC;pid++)
        {
            lptr -> proc_lock_type[pid] = UNDEFINED;
        }
    }
}
