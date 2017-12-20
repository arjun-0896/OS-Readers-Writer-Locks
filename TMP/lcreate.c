/* lcreate.c */
/* lock has to bre created ie) state is changed so that it can be used by lock.c */
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <stdio.h>
#include <lock.h>

LOCAL int newldes();

int lcreate() {
    STATWORD ps;
    int ldes;
    disable(ps);
    if ((ldes = newldes()) == SYSERR) {
        restore(ps);
        return (SYSERR);
    }
    restore(ps);
    return (ldes);
}

LOCAL int newldes() {
    int i;
    
    for (i = 0; i < NLOCKS; i++) {
        if (locktab[i].lstate == DELETED)
        {
            locktab[i].lstate = 1;
            locktab[i].ltype = UNDEFINED;
            return i;
        }
    }
    return SYSERR;
}




