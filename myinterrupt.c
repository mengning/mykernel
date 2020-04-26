/*
 *  linux/mykernel/myinterrupt.c
 *
 *  Kernel internal my_timer_handler
 *  Change IA32 to x86-64 arch, 2020/4/26
 *
 *  Copyright (C) 2013, 2020  Mengning
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
extern volatile int my_need_sched;
volatile int time_count = 0;

/*
 * Called by timer interrupt.
 * it runs in the name of current running process,
 * so it use kernel stack of current running process
 */
void my_timer_handler(void)
{
    if(time_count%1000 == 0 && my_need_sched != 1)
    {
        printk(KERN_NOTICE ">>>my_timer_handler here<<<\n");
        my_need_sched = 1;
    } 
    time_count ++ ;  
    return;  	
}

void my_schedule(void)
{
    tPCB * next;
    tPCB * prev;

    if(my_current_task == NULL 
        || my_current_task->next == NULL)
    {
    	return;
    }
    printk(KERN_NOTICE ">>>my_schedule<<<\n");
    /* schedule */
    next = my_current_task->next;
    prev = my_current_task;
    if(next->state == 0)/* -1 unrunnable, 0 runnable, >0 stopped */
    {        
    	my_current_task = next; 
    	printk(KERN_NOTICE ">>>switch %d to %d<<<\n",prev->pid,next->pid);  
    	/* switch to next process */
    	asm volatile(	
        	"pushq %%rbp\n\t" 	    /* save rbp of prev */
        	"movq %%rsp,%0\n\t" 	/* save rsp of prev */
        	"movq %2,%%rsp\n\t"     /* restore  rsp of next */
        	"movq $1f,%1\n\t"       /* save rip of prev */	
        	"pushq %3\n\t" 
        	"ret\n\t" 	            /* restore  rip of next */
        	"1:\t"                  /* next process start here */
        	"popq %%rbp\n\t"
        	: "=m" (prev->thread.sp),"=m" (prev->thread.ip)
        	: "m" (next->thread.sp),"m" (next->thread.ip)
    	); 
    }  
    return;	
}
