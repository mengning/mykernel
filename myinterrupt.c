/*
 *  linux/mykernel/myinterrupt.c
 *
 *  Kernel internal my_timer_handler
 *
 *  Copyright (C) 2013  Mengning
 *
 */
#include <linux/types.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/tty.h>
#include <linux/vmalloc.h>

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

