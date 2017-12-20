#ifndef H_LOCK_H_
#define H_LOCK_H_

#include "proc.h"
/* the constant is also defined in proc.h*/
#ifndef NLOCKS
#define NLOCKS 50 /* Number of locks as given is the description */
#endif

/* Definition of constants for type of the lock */
#define READ   '\01'       /* Read lock */
#define WRITE  '\02'        /* Write lock */
#define UNDEFINED    '\03'  /* Initial state (No read nor write) */


struct lentry{
    int lprio; /* to hold maximum priority of process in wait queue (ie.q[lqtail].qprev.qkey */
    char lstate;  /* lock is available or deleted */
    char ltype;   /* Type of the lock */
    int no_of_readers; /* Keeps track of number of processes holding read lock */
    int lock_head;
    int lock_tail;
    int proc_lock_type[NPROC];
};

extern struct lentry locktab[NLOCKS];
void linit();
int lcreate();
int ldelete (int lockdescriptor);
int releaseall (int, long, ...);
int lock (int ldes1, int type, int priority);

void update_lprio(int ldes_lprio);
void update_inherited_prio(int pid);
int select_proc_to_wakeup(int ldesc);
void transitivity_check(int ldes);
int get_curr_prio(int pid);

#endif
