/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>
void update_lprio(int ldes_lprio);
void update_inherited_prio(int pid);
void transitivity_check(int ldes);

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
    struct  lentry  *lptr;
	int	dev;
    int i;
	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:
            pptr->pstate = PRFREE;	/* suicide */
            /* clear all the locks since the process is going to suicide 8*/
            for (i = 0; i<NLOCKS ; i++)
            {
                if (proctab[pid].lock_proc_type[i] != UNDEFINED)
                    release(pid,i);
            }
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;
            
    case PRLOCK:
            dequeue(pid);
            update_lprio(pptr->lockid);
            lptr = &locktab[pptr->lockid];
            for(i=0; i<NPROC; i++)
            {
                if(lptr->proc_lock_type[i] == READ || lptr->proc_lock_type[i] == WRITE)
                    update_inherited_prio(i);
            }
            pptr->pstate = PRFREE;
            break;
            
	case PRREADY:
            pptr->pstate = PRFREE;
            /* clear all the locks since the process is going to be killed */
            for (i = 0; i<NLOCKS ; i++)
            {
                if (proctab[pid].lock_proc_type[i] != UNDEFINED)
                    release(pid,i);
            }
            dequeue(pid); //remove from ready queue
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:
            pptr->pstate = PRFREE;
            /* clear all the locks since the process is going to be killed */
            for (i = 0; i<NLOCKS ; i++)
            {
                if (proctab[pid].lock_proc_type[i] != UNDEFINED)
                    release(pid,i);
            }
	}
	restore(ps);
	return(OK);
}
