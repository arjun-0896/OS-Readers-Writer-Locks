/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>
void update_lprio(int ldes_lprio);
void update_inherited_prio(int pid);
void transitivity_check(int ldes);
/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	pptr->pprio = newprio;
    
    if(pptr -> pstate == PRREADY) {
        insert( dequeue(pid), rdyhead, newprio);
    }
    
    update_inherited_prio(pid);
    if (pptr->pstate == PRLOCK) {
        update_lprio(pptr->lockid);
        transitivity_check(pptr->lockid);
    }
    
	restore(ps);
	return(newprio);
}
