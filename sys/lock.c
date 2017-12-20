/* lock.c */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <lock.h>
#include <stdio.h>

extern unsigned long ctr1000; // this variable provide the current system time as used in PA0
void update_lprio(int ldes_lprio);
void transitivity_check(int ldes);
int get_curr_prio(int pid);
int lock(int ldes1, int type, int priority) {
    STATWORD ps;
    struct pentry *pptr;
    disable(ps);
    
    /* If invalid lock descriptor or lock not created or the process already has a read | write | deleted in this lock, return SYSERR */
    if ((ldes1 < 0 || ldes1 > NLOCKS) || locktab[ldes1].lstate == DELETED || proctab[currpid].lock_proc_type[ldes1] != UNDEFINED) {
        restore(ps);
        return (SYSERR);
    }
    int i;
    proctab[currpid].lock_wait_ret = OK;
    switch(locktab[ldes1].ltype)
    {
    
        case UNDEFINED :
            locktab[ldes1].ltype = type;
            proctab[currpid].lock_proc_type[ldes1] = type;
            locktab[ldes1].proc_lock_type[currpid] = type;
            if (type == READ)
            {
                locktab[ldes1].no_of_readers++;
            }
            break;
            
        case WRITE:
            pptr = &proctab[currpid];
            pptr->pstate = PRLOCK;
            pptr->lockid = ldes1;
            pptr->lock_proc_type[ldes1] = type;
            pptr->lock_req_time = ctr1000;
            if(get_curr_prio(currpid) > locktab[ldes1].lprio)
            {
                locktab[ldes1].lprio = get_curr_prio(currpid);
            }
            insert(currpid, locktab[ldes1].lock_head, priority);
            //int curr_proc_prio = get_curr_prio(currpid);
            transitivity_check(ldes1);
            resched();
            break;
        
        case READ:
        if (type == WRITE)
        {
            pptr = &proctab[currpid];
            pptr->pstate = PRLOCK;
            pptr->lockid = ldes1;
            pptr->lock_proc_type[ldes1] = type;
            pptr->lock_req_time = ctr1000;
            if(get_curr_prio(currpid) > locktab[ldes1].lprio)
            {
                locktab[ldes1].lprio = get_curr_prio(currpid);
            }
            insert(currpid, locktab[ldes1].lock_head, priority);
            //int curr_proc_prio = get_curr_prio(currpid);
            transitivity_check(ldes1);
            resched();
        }
        else if (type == READ)
        {
            int flag = 0;
            int temp;
            int head = locktab[ldes1].lock_head;
            int tail = locktab[ldes1].lock_tail;
            for (temp = q[tail].qprev; temp != head ; temp = q[temp].qprev)
            {
                if (proctab[temp].lock_proc_type[ldes1] == WRITE && q[temp].qkey > priority) {
                    flag = 1;
                    break;
                }
            }
            if (flag == 1)
            {
                pptr = &proctab[currpid];
                pptr->pstate = PRLOCK;
                pptr->lockid = ldes1;
                pptr->lock_proc_type[ldes1] = type;
                pptr->lock_req_time = ctr1000;
                if(get_curr_prio(currpid) > locktab[ldes1].lprio)
                {
                    locktab[ldes1].lprio = get_curr_prio(currpid);
                }
                insert(currpid, locktab[ldes1].lock_head, priority);
                //int curr_proc_prio = get_curr_prio(currpid);
                transitivity_check(ldes1);
                resched();
            }
            else
            {
                locktab[ldes1].ltype = type;
                locktab[ldes1].no_of_readers++;
                proctab[currpid].lock_proc_type[ldes1] = type;
                locktab[ldes1].proc_lock_type[currpid] = type;
            }
        }
            break;
    }
    restore(ps);
    return (proctab[currpid].lock_wait_ret); // returns OK or DELETED
}

void transitivity_check(int ldes)
{
    int wait_prio = locktab[ldes].lprio;
    int i;
    for(i=0;i<NPROC;i++)
    {
        if(locktab[ldes].proc_lock_type[i] == locktab[ldes].ltype)
        {
            int tmp = get_curr_prio(i);
            if(tmp < wait_prio) /* this condition will ensure that priority is changed only when waiting priority is greater */
            {
                proctab[i].pinh = wait_prio;
                if(proctab[i].lockid != -1) /* This is kind of stopping condition for the recursion */
                {
                    update_lprio(proctab[i].lockid);
                    transitivity_check(proctab[i].lockid);
                }
                
            }
        }
    }
}
int get_curr_prio(int pid)
{
    int curr_holding_prio;
    struct pentry *pptr = &proctab[pid];
    if(pptr -> pinh == 0)
    {
        curr_holding_prio = pptr -> pprio;
        
    }
    else
    {
        curr_holding_prio = pptr -> pinh;
    }
    return curr_holding_prio;
}

