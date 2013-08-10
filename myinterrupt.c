/*
 *  linux/mykernel/myinterrupt.c
 *
 *  Kernel internal my_timer_handler
 *
 *  Copyright (C) 2013  Mengning
 *
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

#define CREATE_TRACE_POINTS
#include <trace/events/timer.h>
#include <asm/switch_to.h>
#include "mypcb.h"
extern tPCB task[MAX_TASK_NUM];
extern tPCB * my_current_task;
extern int task_count;


/*
 * Called by timer interrupt.
 * it runs in the name of current running process,
 * so it use kernel stack of current running process
 */
void my_timer_handler(void)
{
    tPCB * next;
    tPCB * prev;
    printk(KERN_NOTICE ">>>my_timer_handler here<<<\n");

    if(my_current_task == NULL 
        || my_current_task->next == NULL)
    {
    	return;
    }
#if 0
    if(task_count < MAX_TASK_NUM)
    {
        
        /* fork a new process */
        tPCB * parent = my_current_task;
        tPCB * child = NULL;
        unsigned long ebp;
        int i;
        for(i=0;i<MAX_TASK_NUM;i++)
        {
            if (task[i].pid == -1)
            {
                child = &task[i];
                break;
            }
        }       
    	/* pretended switched state */
    	asm volatile(	
        	"pushl %%ebp\n\t" 	    /* save ebp */
        	"movl %%esp,%0\n\t" 	/* save esp */
        	"movl $1f,%1\n\t"       /* save eip */
        	: "=m" (parent->thread.sp),"=m" (parent->thread.ip)
        	: 
    	); 
        printk(KERN_NOTICE ">>>fork a new process<<<\n");
    	memcpy(child,parent,sizeof(tPCB));
    	task_count = task_count + 1;
    	/* insert new PCB */
    	child->next = parent->next;
    	parent->next = child;
    	child->pid = task_count;
    	child->thread.sp = (unsigned long)&child->stack[KERNEL_STACK_SIZE-1];
    	ebp = child->thread.sp;
    	child->thread.ip = child->task_entry;
    	my_current_task = child;
    	/* fork success,switch to new process */
    	if(child->task_entry == parent->task_entry)
    	    printk(KERN_NOTICE ">>>fork a new process task_entry<<<\n");
    	asm volatile(	
        	"movl %0,%%esp\n\t"     /* new esp */	
        	"movl %0,%%ebp\n\t"     /* new ebp */
        	"pushfl \n\t"
        	"pushl %%cs\n\t"
        	"pushl %1\n\t"
        	"iret\n\t"
        	: 
        	: "m" (child->thread.sp),"m" (child->thread.ip)
    	); 	
    	return;
    }
#endif
    /* schedule */
    next = my_current_task->next;
    prev = my_current_task;
    if(next->state == 0)/* -1 unrunnable, 0 runnable, >0 stopped */
    {
        printk(KERN_NOTICE ">>>switch %d to %d<<<\n",prev->pid,next->pid);
    	/* switch to next process */
    	asm volatile(	
        	"pushl %%ebp\n\t" 	    /* save ebp */
        	"movl %%esp,%0\n\t" 	/* save esp */
        	"movl %2,%%esp\n\t"     /* restore  esp */
        	"movl $1f,%1\n\t"       /* save eip */	
        	"pushl %3\n\t" 
        	"ret\n\t" 	            /* restore  eip */
        	"1:\t"                  /* next process start here */
        	"popl %%ebp\n\t"
        	: "=m" (prev->thread.sp),"=m" (prev->thread.ip)
        	: "m" (next->thread.sp),"m" (next->thread.ip)
    	); 
    	my_current_task = next;
    }
    
    return;	
}

