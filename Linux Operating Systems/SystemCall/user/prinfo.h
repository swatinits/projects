#include <linux/unistd.h>

#include<linux/types.h>
#define __NR_getprocessinfo 333


struct prinfo {
	long state;				//current state of process
	long nice;				//process nice value
	pid_t pid;				//process id
	pid_t parent_pid;			//process id of parent
	pid_t youngest_child_pid;		//process id of youngest child
	pid_t younger_sibling_pid;		//pid of younger sibling
	pid_t older_sibling_pid;		//pid of older sibling
	unsigned long start_time;		//process start time
	long user_time;				//cpu time spent in user mode
	long sys_time;				//cpu time spent in system mode
	long cutime;				//total user time of children
	long cstime;				//total system time of children
	long uid;				//user id of process owner
	char comm[16];				//name of program executed
};





