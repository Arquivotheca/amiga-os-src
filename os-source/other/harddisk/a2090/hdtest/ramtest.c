#include <exec/types.h>
#include <lattice/stdio.h>
#include "ddefs.h"

extern	char	stop_on_comp_error; /* Stop when compare error if true */
extern	int	errors;
extern	UWORD	*getlong();
	char	ram_prompt=1;	/* Not 0 means need to prompt for ram address */
static	UWORD	*ram_start;	/* Address to start RAM test */
static	UWORD	*ram_end;	/* Address to end RAM test */
extern	int	stop_test;	/* Not 0 means <ctrl-c> typed */

void ram_pass(old_value,new_value)
register UWORD	new_value;
register UWORD	old_value;
{
	register UWORD	*ram_ptr;
	register int	count;
	register UWORD	save_val;

	puts("\r          Replacing ");
	putihex(old_value);
	puts(" with ");
	putihex(new_value);
	
#ifdef junk
	for (ram_ptr = ram_start,count = 0;
		(!check_stop_test()) && (!stop_test) && (count < 0x1000) &&
		((ram_ptr+0x0FFF) < ram_end);)
	{
	    puts("\r");
	    putlhex(ram_ptr);
	    for (;(count < 0x1000) && (!stop_test);count++)
	    {
#else
	for (ram_ptr = ram_start; ram_ptr < ram_end;)
	{
#endif
		save_val = *ram_ptr;
		if (save_val == old_value)
		{
			*ram_ptr = new_value;
			save_val = *ram_ptr;
		}
		if (save_val != new_value)
		{
			puts("\naddress ");
			putlhex(ram_ptr);
			puts(" contains ");
			putihex(save_val);
			errors++;
			stop_test = stop_on_comp_error;
		}
	        ram_ptr++;
#ifdef junk
	    }
#endif
	}
}

void RAMTEST()
{
	register UWORD *ram_ptr;
		 UWORD new_value;
		 UWORD old_value;
	
	if (ram_prompt)
	{
		puts("\nEnter starting address : ");
		ram_start = getlong(-1L);
		puts("\nEnter ending address : ");
		ram_end = getlong(-1L);
		ram_prompt = 0;
		puts("\nStarting address = ");
		putlhex(ram_start);
		puts(" Ending address = ");
		putlhex(ram_end);
	}

	puts("\nFilling RAM with '0000'\n");
	new_value = 0;
	for (ram_ptr = ram_start; ram_ptr < ram_end;)
		*ram_ptr++ = new_value;

	old_value = new_value;
	new_value = 0xFFFF;
	ram_pass(old_value,new_value);
	old_value = new_value;
	new_value = 0x5555;
	ram_pass(old_value,new_value);
	old_value = new_value;
	new_value = 0xAAAA;
	ram_pass(old_value,new_value);
}
