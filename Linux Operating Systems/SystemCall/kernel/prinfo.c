/*This file prinfo.c hs to be placed in "linux2.6.28/kernel" folder */

#include <linux/compat.h>
#include <linux/syscalls.h>
#include <linux/kprobes.h>
#include <linux/user_namespace.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/unistd.h>
#include <linux/prinfo.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/linkage.h>
#include <linux/types.h>
#include <asm/current.h>
#include <linux/times.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/utsname.h>
#include <linux/mman.h>
#include <linux/smp_lock.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/prctl.h>
#include <linux/highuid.h>
#include <linux/fs.h>
#include <linux/resource.h>
#include <linux/kernel.h>
#include <linux/kexec.h>
#include <linux/workqueue.h>
#include <linux/capability.h>
#include <linux/device.h>
#include <linux/key.h>
#include <linux/posix-timers.h>
#include <linux/security.h>
#include <linux/dcookies.h>
#include <linux/suspend.h>
#include <linux/tty.h>
#include <linux/signal.h>
#include <linux/cn_proc.h>
#include <linux/getcpu.h>
#include <linux/task_io_accounting_ops.h>
#include <linux/seccomp.h>
#include <linux/cpu.h>
#include <asm/uaccess.h>
#include <asm/io.h>

asmlinkage long sys_getprocessinfo(struct prinfo *info)
{

struct prinfo *val;   

val=(struct prinfo *)kmalloc(sizeof(struct prinfo),GFP_KERNEL);
                     
struct task_struct  *cur_task;                                                /* points to task_struct of current process*/
struct task_struct *cur_parent;                                               /* points to task_struct of parent of current process*/
struct list_head *prev_child;
struct task_struct  *child_young;                                             /* points to task_struct of younger sibling of current process*/
struct list_head *next_head;                                                  /* points to list_head of younger sibling of current process*/
struct task_struct  *sib_young;                                               /* points to task_struct of younger sibling of current process*/
struct list_head *prev_head;                                                  /* points to list_head of older sibling of current process*/
struct task_struct  *sib_old;                                                 /* points to task_struct of older sibling of current process*/
struct task_cputime cputime;
cputime_t cutime, cstime;
struct timespec timemid;
struct tms *buf= (struct tms *)kmalloc(sizeof(struct tms),GFP_KERNEL);;;
clock_t tcutime, tcstime, tutime, tstime;
int i;

cur_task = get_current();                                                     /*Get the task_struct current process*/

long priomid = cur_task->prio;                                    
val->nice = priomid - 100 -20;                                                /*returns the nice value of current process*/
                                
val->state = cur_task->state;                                                 /*returns the state of current process*/
val->pid = cur_task->pid;                                                     /*returns the pid of current process*/

cur_parent = cur_task->parent;                                                /*Get the task_struct of parent current process*/
val->parent_pid = cur_parent->pid;                                            /*returns the pid of parent of current process*/


prev_child = cur_task->children.prev;                                         /*returns the list_head of youngest child of current process*/
child_young = list_entry(prev_child, struct task_struct, children);           /* get the task_struct of youngest child using its list_head*/
val->youngest_child_pid = child_young->pid;                                  /*returns the pid of youngest child of current process*/

/*returns -1 if no child exists, ie the next and prev field point the same list_head of the parent*/
if (val->youngest_child_pid == cur_task->pid)
val->youngest_child_pid = -1;                                                

next_head = cur_task->sibling.next;     
sib_young = list_entry(next_head, struct task_struct, sibling);               /* get the task_struct of younger sibling using its list_head*/
val->younger_sibling_pid = sib_young->pid;                                   /*returns the pid of younger sibling of current process*/
if (val->younger_sibling_pid == 0)
val->younger_sibling_pid = -1;                                               /*returns -1 if no sibling exists*/

prev_head = cur_task->sibling.prev; 
sib_old = list_entry(prev_head, struct task_struct, sibling);               /* get the task_struct of older sibling using its list_head*/
val->older_sibling_pid = sib_old->pid;                                      /*returns the pid of older sibling of current process*/
if (val->older_sibling_pid == 0)
val->older_sibling_pid = -1;                                                /*returns -1 if no sibling exists*/

timemid = cur_task->start_time;                                             /*return the start_time in a struct of type timespec*/
val->start_time = timemid.tv_nsec;                                          /*returns the start time of current process in nanoseconds*/


	spin_lock_irq(&current->sighand->siglock);                          /*Code to get the user and system time of the current process*/
	thread_group_cputime(current, &cputime);                            /* It is found in kernel/sys.c as a function times() */
	cutime = current->signal->cutime;
	cstime = current->signal->cstime;
	spin_unlock_irq(&current->sighand->siglock);
	buf->tms_utime = cputime_to_clock_t(cputime.utime);
	buf->tms_stime = cputime_to_clock_t(cputime.stime);
	buf->tms_cutime = cputime_to_clock_t(cutime);
	buf->tms_cstime = cputime_to_clock_t(cstime); 

tutime =  buf->tms_utime;  /* user time */
tstime =  buf->tms_stime;  /* system time */
tcutime =  buf->tms_cutime; /* user time of children */
tcstime =  buf->tms_cstime; /* system time of children */


val->user_time = (((long)tutime)*1000)/(long)CLOCKS_PER_SEC;           /*returns the CPU time spent by the current process in user mode in milliseconds*/
val->sys_time = (((long)tstime)*1000)/(long)CLOCKS_PER_SEC;            /*returns the CPU time spent by the current process in system mode in milliseconds*/
val->cutime = (((long)tcutime)*1000)/(long)CLOCKS_PER_SEC;             /*returns the total CPU time spent by the children  in user mode in milliseconds*/
val->cstime = (((long)tcstime)*1000)/(long)CLOCKS_PER_SEC;             /*returns the total CPU time spent by the children  in system mode in milliseconds*/


val->uid=(long)(cur_task->uid);                                            /*returns the User id of current process*/
strncpy (val->comm,cur_task->comm,16);                                     /*returns the name of current process*/

if (copy_to_user(info,val,sizeof(struct prinfo)))                          /* Copies the information from kernel space to user time*/
return -EFAULT;

return 0;
}




