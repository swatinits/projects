#ifndef _LINUX_PRINFO_H
#define _LINUX_PRINFO_H

#include <linux/types.h>
#include <linux/unistd.h>

/*This file has to be placed in "linux-2.6.28/include/linux" folder*/

struct prinfo {
      long state;			      /* Current state of process */
      long nice;			      /* Process nice value*/
      pid_t pid;			      /* Process id */
      pid_t parent_pid;		     /* Process id of parent */
      pid_t youngest_child_pid;                  /* pid of youngest child */
      pid_t younger_sibling_pid;	    /* pid of younger sibling */
      pid_t older_sibling_pid;	                /* pid of older sibling */
      unsigned long start_time;	 /* process start time */
      long user_time;	              /* CPU time spent in user mode */
      long sys_time;	                              /* CPU time spent in system mode */
      long cutime;	                            /* Total user time of children */
      long cstime;	                           /* Total system time of children */
      long uid;   	                                /* User id of process owner */
      char comm[16];	              /* Name of program executed */
};


#endif /* _PRINFO_H */
