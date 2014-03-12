/*	mykernel--Simple simulation of the linux OS  process schedule
 *
 *  linux/mykernel/myinterrupt.c
 *
 *  Kernel internal my_timer_handler
 *
 *  Copyright (C) 2013  Mengning
 *  
 *  Modified 2014 zhyq
 * 
 * 
 *  You can redistribute or modify this program under the terms
 *  of the GNU General Public License as published by
 *  the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/kernel_stat.h>
#include <linux/export.h>
#include <linux/interrupt.h>
#include <linux/percpu.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/pid_namespace.h>
#include <linux/notifier.h>
#include <linux/thread_info.h>
#include <linux/time.h>
#include <linux/jiffies.h>
#include <linux/posix-timers.h>
#include <linux/cpu.h>
#include <linux/syscalls.h>
#include <linux/delay.h>
#include <linux/tick.h>
#include <linux/kallsyms.h>
#include <linux/irq_work.h>
#include <linux/sched.h>
#include <linux/sched/sysctl.h>
#include <linux/slab.h>

#include <asm/uaccess.h>
#include <asm/unistd.h>
#include <asm/div64.h>
#include <asm/timex.h>
#include <asm/io.h>

#include <linux/types.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/tty.h>
#include <linux/vmalloc.h>

#include "mypcb.h"

#define CREATE_TRACE_POINTS
#include <trace/events/timer.h>

extern tPCB task[MAX_TASK_NUM];
extern tPCB * my_current_task;
extern volatile int my_need_sched;
volatile int time_count = 0;

/*
* Called by timer interrupt.
* it runs in the name of current running process,
* so it use kernel stack of current running process
*/
void my_timer_handler(void)
{
#if 1
    // make sure need schedule after system circle 2000 times.
    if(time_count%2000 == 0 && my_need_sched != 1)
    {
        my_need_sched = 1;
	//time_count=0;
    }
    time_count ++ ;
#endif
    return;
}

void all_task_print(void);

tPCB * get_next(void)
{
	int pid,i;
	tPCB * point=NULL;
	tPCB * hig_pri=NULL;//points to the the hightest task
	all_task_print();
	hig_pri=my_current_task;
	for(i=0;i<MAX_TASK_NUM;i++)
		if(task[i].priority<hig_pri->priority)	
			hig_pri=&task[i];
	printk("                higst process is:%d priority is:%d\n",hig_pri->pid,hig_pri->priority);
	return hig_pri;

}//end of priority_schedule

void my_schedule(void)
{
    tPCB * next;
    tPCB * prev;
    // if there no task running or only a task ,it shouldn't need schedule
    if(my_current_task == NULL
        || my_current_task->next == NULL)
    {
	printk(KERN_NOTICE "                time out!!!,but no more than 2 task,need not schedule\n");
     return;
    }
    /* schedule */

    next = get_next();
    prev = my_current_task;
    printk(KERN_NOTICE "                the next task is %d priority is %u\n",next->pid,next->priority);
    if(next->state == 0)/* -1 unrunnable, 0 runnable, >0 stopped */
    {//save current scene
     /* switch to next process */
     asm volatile(	
         "pushl %%ebp\n\t" /* save ebp */
         "movl %%esp,%0\n\t" /* save esp */
         "movl %2,%%esp\n\t" /* restore esp */
         "movl $1f,%1\n\t" /* save eip */	
         "pushl %3\n\t"
         "ret\n\t" /* restore eip */
         "1:\t" /* next process start here */
         "popl %%ebp\n\t"
         : "=m" (prev->thread.sp),"=m" (prev->thread.ip)
         : "m" (next->thread.sp),"m" (next->thread.ip)
     );
     my_current_task = next;//switch to the next task
    printk(KERN_NOTICE "                switch from %d process to %d process\n                >>>process %d running!!!<<<\n\n",prev->pid,next->pid,next->pid);

  }
    else
    {
        next->state = 0;
        my_current_task = next;
    printk(KERN_NOTICE "                switch from %d process to %d process\n                >>>process %d running!!!<<<\n\n\n",prev->pid,next->pid,next->pid);

     /* switch to new process */
     asm volatile(	
         "pushl %%ebp\n\t" /* save ebp */
         "movl %%esp,%0\n\t" /* save esp */
         "movl %2,%%esp\n\t" /* restore esp */
         "movl %2,%%ebp\n\t" /* restore ebp */
         "movl $1f,%1\n\t" /* save eip */	
         "pushl %3\n\t"
         "ret\n\t" /* restore eip */
         : "=m" (prev->thread.sp),"=m" (prev->thread.ip)
         : "m" (next->thread.sp),"m" (next->thread.ip)
     );
    }
    return;	
}//end of my_schedule

void all_task_print(void)
{
	int i,cnum=62;//
	printk(KERN_NOTICE "\n                current task is:%d   all task in OS are:\n",my_current_task->pid);

	printk("        ");
	for(i=0;i<cnum;i++)
		printk("-");
	printk("\n        |  process:");
	for(i=0;i< MAX_TASK_NUM;i++)
		printk("| %2d ",i);
	printk("|\n        | priority:");
	for(i=0;i<MAX_TASK_NUM;i++)
		printk("| %2d ",task[i].priority);

	printk("|\n        ");
	for(i=0;i<cnum;i++)
		printk("-");
	printk("\n");
}
