#include <defs.h>
#include <stdio.h>
#include <string.h>
#include <console.h>
#include <monitor.h>
#include <picirq.h>
#include <trap.h>
#include <clock.h>
#include <intr.h>
#include <pmm.h>
#include <proc.h>
#include <thumips_tlb.h>
#include <sched.h>
//kyle begin
#define WRITE_IO_CHAR(addr) (volatile unsigned char *)(addr)
#define         OFS_SEND_BUFFER         0
void uart_printf(char * x)
{
	int i = 0;
	for(i=0; x[i]!='\0'; i++)
	{
		*WRITE_IO_CHAR(COM1+OFS_SEND_BUFFER) = x[i];
		delay(1);
	}
}
//kyle end



void __noreturn kern_init(void)
{
	tlb_invalidate_all();
	char *p = 0x7ffff000;
	mp_init();
	pic_init();		// init interrupt controller
	cons_init();		// init the console
	clock_init();		// init clock interrupt
	check_initrd();
	const char *message = "OS is loading ...\n\r\n\r";
	kprintf(message);
	print_kerninfo();

	pmm_init();		// init physical memory management

	sched_init();
	proc_init();		// init process table
	ide_init();
	fs_init();
	intr_enable();		// enable irq interrupt
	cpu_idle();
}
