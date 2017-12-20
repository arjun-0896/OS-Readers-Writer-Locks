/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */

    optr = &proctab[currpid];
    int pid, max_prio, max_pid,curr_proc_prio;
    max_pid = 0;
    max_prio = -1;
    
    if(proctab[currpid].pinh == 0)
    {
        curr_proc_prio = proctab[currpid]. pprio;
        
    }
    else
    {
        curr_proc_prio = proctab[currpid].pinh;
    }
    
    if ((optr->pstate == PRCURR) && isempty(rdyhead))
        return OK; /* no process in ready queue */
    
    pid = q[rdytail].qprev;
    int tmp;
    while (pid != rdyhead) {
        if(proctab[pid].pinh == 0)
        {
            tmp = proctab[pid]. pprio;
            
        }
        else
        {
            tmp = proctab[pid].pinh;
        }
        if (tmp > max_prio) {
            max_prio = tmp;
            max_pid = pid;
        }
        pid = q[pid].qprev;
    }
    /* no switch needed if current process priority higher than next*/

	if ( ( optr -> pstate == PRCURR) && max_prio < curr_proc_prio) {
		return(OK);
	}
	
	/* force context switch */

	if (optr->pstate == PRCURR) {
		optr->pstate = PRREADY;
		insert(currpid,rdyhead,optr->pprio);
	}

	/* remove highest priority process at end of ready list */

    currpid = max_pid;
    nptr = &proctab[max_pid];
    dequeue(max_pid);
	nptr->pstate = PRCURR;		/* mark it currently running	*/
#ifdef	RTCLOCK
	preempt = QUANTUM;		/* reset preemption counter	*/
#endif
	
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
	/* The OLD process returns here when resumed. */
	return OK;
}
