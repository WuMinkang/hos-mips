#include <defs.h>
#include <arch.h>
#include <stdio.h>
#include <string.h>
#include <picirq.h>
#include <intr.h>
#include <trap.h>
#include <memlayout.h>
#include <sync.h>
//#include <vga.h>

/* stupid I/O delay routine necessitated by historical PC design flaws */
void delay(void)
{
    volatile unsigned int j;
    for (j = 0; j < (500); j++) ; // delay
}

//#define DEBUG_COM1
/***** Serial I/O code *****/
//previous define begin
#define COM_RBR         0x1000  // In:receive buffer
#define COM_THR         0x1000  // Out:Transmitter Holding Register
#define COM_IER         0x1004  // Out: Interrupt Enable Register
#define COM_IIR         0x1008  // In:  Interrupt ID Register
#define COM_FCR         0x1008  // Out: FIFO Control Register
#define COM_LCR         0x100c  // Out: Line Control Register
#define COM_MCR         0x1010  // Out: Modem Control Register
#define COM_LSR         0x1014  // In:  Line Status Register
#define COM_MSB         0x1018  // Modem Status Register
#define COM_DLL         0x1000  // Out: Divisor Latch (Least Significant Byte) Register (DLAB=1)
#define COM_DLM         0x1004  // Out: Divisor Latch (Most Significant Byte) Register (DLAB=1)

#define COM_IER_RDI     0x01    // Enable receiver data interrupt
#define COM_LCR_DLAB    0x80    // Divisor latch access bit
#define COM_LCR_WLEN8   0x03    // Wordlength: 8 bits
#define COM_MCR_RTS     0x02    // RTS complement
#define COM_MCR_DTR     0x01    // DTR complement
#define COM_MCR_OUT2    0x08    // Out2 complement
#define COM_LSR_DATA    0x01    // Data available
#define COM_LSR_TXRDY   0x20    // Transmit buffer avail
#define COM_LSR_TSRE    0x40    // Transmitter off
//previous define end
//kyle begin
//axi4_C fpga.h begin
#define READ_IO(addr)  (volatile unsigned int *)(addr)
#define WRITE_IO(addr) (volatile unsigned int *)(addr)
#define READ_IO_CHAR(addr)  (volatile unsigned char *)(addr)
#define WRITE_IO_CHAR(addr) (volatile unsigned char *)(addr)
/* === END OF CONFIG === */
#define         REG_OFFSET              1
/* register offset */
#define         OFS_RCV_BUFFER          0
#define         OFS_TRANS_HOLD          0
#define         OFS_SEND_BUFFER         0
#define         OFS_INTR_ENABLE         (1*REG_OFFSET)
#define         OFS_INTR_ID             (2*REG_OFFSET)
#define         OFS_FIFO             	(2*REG_OFFSET)
#define         OFS_DATA_FORMAT         (3*REG_OFFSET)
#define         OFS_LINE_CONTROL        (3*REG_OFFSET)
#define         OFS_MODEM_CONTROL       (4*REG_OFFSET)
#define         OFS_RS232_OUTPUT        (4*REG_OFFSET)
#define         OFS_LINE_STATUS         (5*REG_OFFSET)
#define         OFS_MODEM_STATUS        (6*REG_OFFSET)
#define         OFS_RS232_INPUT         (6*REG_OFFSET)
#define         OFS_SCRATCH_PAD         (7*REG_OFFSET)

#define         OFS_DIVISOR_LSB         (0*REG_OFFSET)
#define         OFS_DIVISOR_MSB         (1*REG_OFFSET)
//axi4_C fpga.h end
//axi4_C uart.h begin
/* interrupt enable register */
#define	IER_ERXRDY	0x1	/* int on rx ready */
#define	IER_ETXRDY	0x2	/* int on tx ready */
#define	IER_ERLS	0x4	/* int on line status change */
#define	IER_EMSC	0x8	/* int on modem status change */

/* interrupt identification register */
#define	IIR_IMASK	0xf	/* mask */
#define	IIR_RXTOUT	0xc	/* receive timeout */
#define	IIR_RLS		0x6	/* receive line status */
#define	IIR_RXRDY	0x4	/* receive ready */
#define	IIR_TXRDY	0x2	/* transmit ready */
#define	IIR_NOPEND	0x1	/* nothing */
#define	IIR_MLSC	0x0	/* modem status */
#define	IIR_FIFO_MASK	0xc0	/* set if FIFOs are enabled */

/* fifo control register */
#define	FIFO_ENABLE	0x01	/* enable fifo */
#define	FIFO_RCV_RST	0x02	/* reset receive fifo */
#define	FIFO_XMT_RST	0x04	/* reset transmit fifo */
#define	FIFO_DMA_MODE	0x08	/* enable dma mode */
#define	FIFO_TRIGGER_1	0x00	/* trigger at 1 char */
#define	FIFO_TRIGGER_4	0x40	/* trigger at 4 chars */
#define	FIFO_TRIGGER_8	0x80	/* trigger at 8 chars */
#define	FIFO_TRIGGER_14	0xc0	/* trigger at 14 chars */

/* character format control register */
#define	CFCR_DLAB	0x80	/* divisor latch */
#define	CFCR_SBREAK	0x40	/* send break */
#define	CFCR_PZERO	0x30	/* zero parity */
#define	CFCR_PONE	0x20	/* one parity */
#define	CFCR_PEVEN	0x10	/* even parity */
#define	CFCR_PODD	0x00	/* odd parity */
#define	CFCR_PENAB	0x08	/* parity enable */
#define	CFCR_STOPB	0x04	/* 2 stop bits */
#define	CFCR_8BITS	0x03	/* 8 data bits */
#define	CFCR_7BITS	0x02	/* 7 data bits */
#define	CFCR_6BITS	0x01	/* 6 data bits */
#define	CFCR_5BITS	0x00	/* 5 data bits */

/* modem control register */
#define	MCR_LOOPBACK	0x10	/* loopback */
#define	MCR_IENABLE	0x08	/* output 2 = int enable */
#define	MCR_DRS		0x04	/* output 1 = xxx */
#define	MCR_RTS		0x02	/* enable RTS */
#define	MCR_DTR		0x01	/* enable DTR */

/* line status register */
#define	LSR_RCV_FIFO	0x80	/* error in receive fifo */
#define	LSR_TSRE	0x40	/* transmitter empty */
#define	LSR_TXRDY	0x20	/* transmitter ready */
#define	LSR_BI		0x10	/* break detected */
#define	LSR_FE		0x08	/* framing error */
#define	LSR_PE		0x04	/* parity error */
#define	LSR_OE		0x02	/* overrun error */
#define	LSR_RXRDY	0x01	/* receiver ready */
#define	LSR_RCV_MASK	0x1f

/* modem status register */
#define	MSR_DCD		0x80	/* DCD active */
#define	MSR_RI		0x40	/* RI  active */
#define	MSR_DSR		0x20	/* DSR active */
#define	MSR_CTS		0x10	/* CTS active */
#define	MSR_DDCD	0x08    /* DCD changed */
#define	MSR_TERI	0x04    /* RI  changed */
#define	MSR_DDSR	0x02    /* DSR changed */
#define	MSR_DCTS	0x01    /* CTS changed */
//axi4_C uart.h end
//kyle end
/*#define COM_RX          0	// In:  Receive buffer (DLAB=0)
#define COM_TX          0	// Out: Transmit buffer (DLAB=0)
#define COM_DLL         0	// Out: Divisor Latch Low (DLAB=1)
#define COM_DLM         1	// Out: Divisor Latch High (DLAB=1)
#define COM_IER         1	// Out: Interrupt Enable Register
#define COM_IER_RDI     0x01	// Enable receiver data interrupt
#define COM_IIR         2	// In:  Interrupt ID Register
#define COM_FCR         2	// Out: FIFO Control Register
#define COM_LCR         3	// Out: Line Control Register
#define COM_LCR_DLAB    0x80	// Divisor latch access bit
#define COM_LCR_WLEN8   0x03	// Wordlength: 8 bits
#define COM_MCR         4	// Out: Modem Control Register
#define COM_MCR_RTS     0x02	// RTS complement
#define COM_MCR_DTR     0x01	// DTR complement
#define COM_MCR_OUT2    0x08	// Out2 complement
#define COM_LSR         5	// In:  Line Status Register
#define COM_LSR_DATA    0x01	// Data available
#define COM_LSR_TXRDY   0x20	// Transmit buffer avail
#define COM_LSR_TSRE    0x40	// Transmitter off
*/

static bool serial_exists = 0;

static void serial_init(void)
{
/*	volatile unsigned char *uart = (unsigned char *)COM1;
	if (serial_exists)
		return;
	serial_exists = 1;
#ifdef MACH_QEMU
	// Turn off the FIFO
	outb(COM1 + COM_FCR, 0);
	// Set speed; requires DLAB latch
	outb(COM1 + COM_LCR, COM_LCR_DLAB);
	outb(COM1 + COM_DLL, (uint8_t) (115200 / 9600));
	outb(COM1 + COM_DLM, 0);

	// 8 data bits, 1 stop bit, parity off; turn off DLAB latch
	outb(COM1 + COM_LCR, COM_LCR_WLEN8 & ~COM_LCR_DLAB);

	// No modem controls
	outb(COM1 + COM_MCR, 0);
	// Enable rcv interrupts
	outb(COM1 + COM_IER, COM_IER_RDI);
#elif defined MACH_FPGA
	//TODO
#endif

	pic_enable(COM1_IRQ);
        pic_enable(KEYBOARD_IRQ);
*/
    volatile unsigned char *uart = (unsigned char *)COM1;
    if (serial_exists)
        return;
    serial_exists = 1;

    //TODO
    //outw(COM1 + COM_LCR,COM_LCR_DLAB);
    //outw(COM1 + COM_DLL,(uint32_t)(115200 / 9600));
    //outw(COM1 + COM_DLM, 0);
    //outw(COM1 + COM_LCR, COM_LCR_WLEN8);
    //outw(COM1 + COM_IER, 0);
    //*WRITE_IO(COM1 + COM_IER)=0x00000000;
	//previous init begin
    //delay();
    //*WRITE_IO(COM1 + COM_THR)=0x0000000a;
    //delay();
    //*WRITE_IO(COM1 + COM_THR)=0x0000000d;
    //delay();
	//previous init end
    /**WRITE_IO(COM1 + COM_THR)=0x00000055;
    delay();
    outw(COM1 + COM_THR, 0x00000061);
    delay();
    outw(COM1 + COM_THR, 0x00000072);
    delay();
    outw(COM1 + COM_THR, 0x00000074);
    delay();*/
	//previous init begin
    //outw(COM1 + COM_IER, COM_IER_RDI);
    //delay();
	//previous init end
	//kyle bgin
	*WRITE_IO_CHAR(COM1 + OFS_FIFO) = FIFO_ENABLE|FIFO_RCV_RST|FIFO_XMT_RST|FIFO_TRIGGER_4;
	unsigned int divisor;
	*WRITE_IO_CHAR(COM1 + OFS_LINE_CONTROL) = 0x80;
	divisor = 33000000 / 16 / 57600;
	//kyle : upper line 115200 -> 57600 try again!
	*WRITE_IO_CHAR(COM1 + OFS_DIVISOR_LSB) = (unsigned char)(divisor & 0xff);
	*WRITE_IO_CHAR(COM1 + OFS_DIVISOR_MSB) = (unsigned char)((divisor & 0xff00) >> 8);	//success in baudrate 57600
	*WRITE_IO_CHAR(COM1 + OFS_DATA_FORMAT) = 0x03;
	*WRITE_IO_CHAR(COM1 + OFS_MODEM_CONTROL) = 0x3;
	*WRITE_IO_CHAR(COM1 + OFS_INTR_ENABLE) = 0;
	*WRITE_IO_CHAR(COM1 + OFS_INTR_ENABLE) = 0x1;
	//kyle end
    pic_enable(COM1_IRQ);
	//pic_enable(KEYBOARD_IRQ);
}

static void serial_putc_sub(int c)
{	//previous putc begin
	//if ((inw(COM1 + COM_IER) & COM_IER_RDI) == 0) delay();
    ////delay();
    //outw(COM1 + COM_THR, c & 0xFF);
    //delay();
	//previous putc end
	//kyle begin
	*WRITE_IO_CHAR(COM1+OFS_SEND_BUFFER) = c;
	//kyle end
}

/* serial_putc - print character to serial port */
static void serial_putc(int c)
{
	if (c == '\b') {
		serial_putc_sub('\b');
		serial_putc_sub(' ');
		serial_putc_sub('\b');
	} else {
		serial_putc_sub(c);
	}
}

/* serial_proc_data - get data from serial port */
static int serial_proc_data(void)
{
    int c;
	//previous proc begin
    //delay();
    //if ((inw(COM1 + COM_LSR) & COM_LSR_DATA) == 0)
    //    return -1;
    //delay();
    //c = inw(COM1 + COM_RBR) & 0xFF;
    //delay();
    //if (c == 127) {
    //    c = '\b';
    //}
	//previous proc end
	//kyle begin
	delay();
	if ((*READ_IO_CHAR(COM1 + OFS_LINE_STATUS) & 0x01) != 1)
        return -1;
	delay();
	c = *READ_IO_CHAR(COM1 + OFS_RCV_BUFFER);
	delay();
	//kyle end
    return c;
}

void serial_int_handler(void *opaque)
{//corrected by xiaohan: this is actually not serial interrupt handler!
 //This is in fact External Interrupt Controller's interrupt handler!
 //So, remember to read the EIC to know what interrupt is happening. But for simplicity,
 //here I assume that the EIC only represents serial's interrupt. Other interrupt sources are neglected.
 //Next, rememer to write EIC to tell EIC that it's interrupt has been handled!
 //otherwise the OS will fall into the dead loop of dealing with "previous" EIC interrupt.   
    int c = cons_getc();
    extern void dev_stdin_write(char c);
    //here we should tell EIC that the serial interrupt has been handled.
    xilinx_intc_init();
    //the following codes are related to "device drivers".
    dev_stdin_write(c);
}

//key board handler
//static const char KEYCODE_MAP[256] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,96,0,0,0,0,0,0,113,49,0,0,0,122,115,97,119,50,0,0,99,120,100,101,52,51,0,0,32,118,102,116,114,53,0,0,110,98,104,103,121,54,0,0,0,109,106,117,55,56,0,0,44,107,105,111,48,57,0,0,46,47,108,59,112,45,0,0,0,39,0,91,61,0,0,0,0,13,93,0,92,0,0,0,0,0,0,0,0,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,126,0,0,0,0,0,0,81,33,0,0,0,90,83,65,87,64,0,0,67,88,68,69,36,35,0,0,32,86,70,84,82,37,0,0,78,66,72,71,89,94,0,0,0,77,74,85,38,42,0,0,60,75,73,79,41,40,0,0,62,63,76,58,80,95,0,0,0,34,0,123,43,0,0,0,0,13,125,0,124,0,0,0,0,0,0,0,0,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

//#define DEBUG_KBD

#ifdef DEBUG_KBD
#include <stdio.h>
#endif

/*void keyboard_int_handler()
{
  int c = *((int*)KEYBOARD);
  if (c < 0 || c > 256)
	  return;
#ifdef DEBUG_KBD
  kprintf("input key c = %d\r\n", c);
#endif
  c = KEYCODE_MAP[c];
  if (c == 0) return;
  extern void dev_stdin_write(char c);
  dev_stdin_write(c);
}
*/
/* *
 * Here we manage the console input buffer, where we stash characters
 * received from the keyboard or serial port whenever the corresponding
 * interrupt occurs.
 * */

#define CONSBUFSIZE 512

static struct {
	uint8_t buf[CONSBUFSIZE];
	uint32_t rpos;
	uint32_t wpos;
} cons;

/* *
 * cons_intr - called by device interrupt routines to feed input
 * characters into the circular console input buffer.
 * */
static void cons_intr(int (*proc) (void))
{
	int c;
	while ((c = (*proc) ()) != -1) {
		if (c != 0) {
			cons.buf[cons.wpos++] = c;
			if (cons.wpos == CONSBUFSIZE) {
				cons.wpos = 0;
			}
		}
	}
}

/* serial_intr - try to feed input characters from serial port */
void serial_intr(void)
{
	if (serial_exists) {
		cons_intr(serial_proc_data);
	}
}

/* cons_init - initializes the console devices */
void cons_init(void)
{
	serial_init();
	//cons.rpos = cons.wpos = 0;
	if (!serial_exists) {
		kprintf("serial port does not exist!!\n\r");
	}
}

/* cons_putc - print a single character @c to console devices */
void cons_putc(int c)
{
	bool intr_flag;
	local_intr_save(intr_flag);
	{
		serial_putc(c);
//#ifdef MACH_FPGA
//		vga_putch(c);
//#endif
	}
	local_intr_restore(intr_flag);
}

/* *
 * cons_getc - return the next input character from console,
 * or 0 if none waiting.
 * */
int cons_getc(void)
{
	int c = 0;
	bool intr_flag;
	local_intr_save(intr_flag);
	{
		// poll for any pending input characters,
		// so that this function works even when interrupts are disabled
		// (e.g., when called from the kernel monitor).
		serial_intr();

		// grab the next character from the input buffer.
		if (cons.rpos != cons.wpos) {
			c = cons.buf[cons.rpos++];
			if (cons.rpos == CONSBUFSIZE) {
				cons.rpos = 0;
			}
		}
	}
	local_intr_restore(intr_flag);
#ifdef DEBUG_COM1
	if (c) kprintf("cons_get(0x%x)\n\r", c);
#endif
	return c;
}
