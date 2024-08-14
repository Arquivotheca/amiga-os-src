/*
 *	Peter's crazy buffer-overflow sampler
 *
 *	Adds an interrupt server that pulls data from the serial bus
 *	as quickly as it becomes available, then checks for the overrun
 *	condition.  If so, it stores the return address from the interrupt,
 *	in hopes of figuring out who's been disabling for so long.
 *
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>

#include <dos/dos.h>

#include <clib/exec_protos.h>

extern (*handler)();

extern struct Custom custom;

struct Interrupt myInt =
{
    {
	NULL,
	NULL,
	NT_INTERRUPT,
	0,
	"Serial Overrun Tester",
    },
    NULL,			/* is_Data */
    NULL,
};

#define BUFFER_ENTRIES	1000		/*  1000 return addresses */
#define DATABUFFER_SIZE	(BUFFER_ENTRIES) * 4 + 8

main()
{
    ULONG *databuffer, *ptr, *end;
    struct Interrupt *old;

    if ( databuffer = AllocMem( DATABUFFER_SIZE, MEMF_CLEAR ) )
    {    
	*databuffer = (ULONG) (databuffer + 2);
	*(databuffer+1) = (ULONG) (databuffer + BUFFER_ENTRIES + 1);
	myInt.is_Code = (APTR) &handler;
	myInt.is_Data = databuffer;
	printf("Next Avail: %lx\n", *databuffer );

	old = SetIntVector( INTB_RBF, &myInt );

	custom.intena = INTF_SETCLR|INTF_RBF;

	Wait( SIGBREAKF_CTRL_C );

	custom.intena = INTF_RBF;

	SetIntVector( INTB_RBF, old );


	printf("Next Avail: %lx\n", *databuffer );

	ptr = databuffer + 2;
	end = (ULONG *) (*databuffer);
	while ( ptr < end )
	{
	    printf("%lx\n", *ptr++);
	}

	FreeMem( databuffer, DATABUFFER_SIZE );
    }
}
