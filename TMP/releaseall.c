#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>
#include <sleep.h>
int get_curr_prio(int pid);
int release(int pid, int ldes);
void update_lprio(int ldes_lprio);
void transitivity_check(int ldes);
void update_inherited_prio(int pid);
int select_proc_to_wakeup(int ldesc);
int releaseall(int numlocks, long locks, ...)
{
    STATWORD ps;
    int return_flag = OK;
    int i,ldes_id;
    disable(ps);
    
    
    for(i=0; i < numlocks ;i++)
    {
        ldes_id = (int)*((&locks) + i);
        if(release(currpid, ldes_id) == SYSERR)
        {
            return_flag = SYSERR;
        }
        
    }
    resched();
    restore(ps);
    return return_flag;
}
int release(int pid, int ldes)
{
    struct lentry *lptr;
    lptr = &locktab[ldes];
    int tmp_pid,tmp2_pid;
    int flag = 0,nextpid = 0;
    
    if((ldes<0 || ldes > NLOCKS) || lptr->proc_lock_type[pid] == UNDEFINED || locktab[ldes].lstate == DELETED || lptr->ltype == UNDEFINED)
    {
        return SYSERR;
    }
    
    proctab[pid].lock_proc_type[ldes] = UNDEFINED;
    locktab[ldes].proc_lock_type[pid] = UNDEFINED;
    update_inherited_prio(currpid);
    /*update_lprio(proctab[currpid].lockid);
    transitivity_check(proctab[currpid].lockid); */
    
    if(lptr->ltype == READ)
    {
        lptr->no_of_readers--;
        if(lptr->no_of_readers > 0)
        {
            return OK;
        }
        else
        {
            flag = 1;
        }
    }
    else if(lptr->ltype == WRITE)
    {
        flag = 1;
    }
    if(flag == 1) // this indicates we need to wake up processes waiting for the lock
    {
        nextpid = select_proc_to_wakeup(ldes);
        if(nextpid == -1) // No process is waiting for this lock
        {
            locktab[ldes].ltype = UNDEFINED;
            return OK;
        }
        if(proctab[nextpid].lock_proc_type[ldes] == READ)
        {
            tmp_pid= q[nextpid].qprev;
            locktab[ldes].ltype = READ;
            locktab[ldes].no_of_readers++;
            locktab[ldes].proc_lock_type[nextpid] = READ;
            dequeue(nextpid);
            ready(nextpid,RESCHNO);
            proctab[nextpid].lockid = -1;
            /* also wake up other readers having priority no less than the higher priority writer */
            while(tmp_pid != locktab[ldes].lock_head && proctab[pid].lock_proc_type[ldes] != WRITE)
            {
                if(proctab[tmp_pid].lock_proc_type[ldes] == READ)
                {
                    tmp2_pid = tmp_pid;
                    tmp_pid = q[tmp_pid].qprev;
                    locktab[ldes].no_of_readers++;
                    locktab[ldes].proc_lock_type[tmp2_pid] = READ;
                    dequeue(tmp2_pid);
                    ready(tmp2_pid,RESCHNO);
                    proctab[tmp2_pid].lockid = -1;
                }
            }
            if(tmp_pid != locktab[ldes].lock_head)
            {
                int equal_prio = q[tmp_pid].qkey;
                
                tmp_pid = q[tmp_pid].qprev;
                while(tmp_pid != locktab[ldes].lock_head && q[tmp_pid].qkey == equal_prio)
                {
                    if(proctab[pid].lock_proc_type[ldes] == READ)
                    {
                        tmp2_pid = tmp_pid;
                        tmp_pid = q[tmp2_pid].qprev;
                        locktab[ldes].no_of_readers++;
                        locktab[ldes].proc_lock_type[tmp2_pid] = READ;
                        dequeue(tmp2_pid);
                        ready(tmp2_pid,RESCHNO);
                        proctab[tmp2_pid].lockid = -1;
                    }
                }
            }
            /* ------------------------------------------------------------------------------------- */
        }
        else
        {
            dequeue(nextpid);
            ready(nextpid,RESCHNO);
            proctab[nextpid].lockid = -1;
            locktab[ldes].ltype = WRITE;
            locktab[ldes].proc_lock_type[nextpid] = WRITE;
        }
    }
    return OK;
    
}


void update_inherited_prio(int pid)
{
    int max = 0, i;
    for(i=0;i<NLOCKS;i++)
    {
        if(locktab[i].proc_lock_type[pid] == READ || locktab[i].proc_lock_type[pid] == WRITE )
        {
                if(locktab[i].lprio > max)
                max = locktab[i].lprio;
        }
    }
    if(proctab[pid].pprio > max)
    proctab[pid].pinh = 0;
    else
    proctab[pid].pinh = max;
}
void update_lprio(int ldes_lprio)
{
     int max = 0, j;
     int tail = locktab[ldes_lprio].lock_tail;
     int head = locktab[ldes_lprio].lock_head;
     for(j = q[tail].qprev; j != head && get_curr_prio(j) > max; j = q[j].qprev)
     {
         max = get_curr_prio(j);
     }
    locktab[ldes_lprio].lprio = max;
}

int select_proc_to_wakeup(int ldesc)
{
    if(q[locktab[ldesc].lock_tail].qprev == locktab[ldesc].lock_head)
    {
        return SYSERR;
    }
    
    int tmp_pid1 = q[locktab[ldesc].lock_tail].qprev;
    int tmp_pid2,return_pid = -1;
    
    /* In our queue implementation the process which came later will be placed closer to head if processes have equal key ie. priority */
    if(proctab[tmp_pid1].lock_proc_type[ldesc] == WRITE)
    {
        return_pid = tmp_pid1;
    }
    else
    {
        tmp_pid2 = q[tmp_pid1].qprev;
        if(q[tmp_pid1].qkey > q[tmp_pid2].qkey || tmp_pid2 == locktab[ldesc].lock_head)
        {
            return_pid = tmp_pid1;
        }
        /* Search the queue backwards to find any writer with difference less than 1s */
        else if((q[tmp_pid1].qkey == q[tmp_pid2].qkey))
        {
            while((tmp_pid2 != locktab[ldesc].lock_head) && (q[tmp_pid1].qkey == q[tmp_pid2].qkey))
            {
                if(proctab[tmp_pid2].lock_proc_type[ldesc] == READ)
                    return_pid = tmp_pid1;
                else
                {
                    int t = proctab[tmp_pid2].lock_req_time - proctab[tmp_pid1].lock_req_time;
                    if(t <= 1000 && t >= -1000)
                        return_pid = tmp_pid2;
                    else
                        return_pid = tmp_pid1;
                    break;
                }
                tmp_pid2 = q[tmp_pid2].qprev;
            }
        }
    }
    return return_pid;
}
