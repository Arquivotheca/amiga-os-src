#include <exec/types.h>
#include <lattice/stdio.h>
#include "ddefs.h"

extern	record_bad_blocks;
extern	char stop_on_comp_error;		/* Stop when compare error if true */
extern	int stop_test;		/* Not 0 means <ctrl-c> typed */
extern	unsigned pass;
extern	ERR_LIST err_list[MAX_HISTORY];	/* Error history */
extern	int idx_err_list;
extern	int cmd_index;
extern	char commands[80];		/* requested commands */
extern	long	bad_blocks[300];	/* bad block list */
extern	int	idx_bad_blocks;
extern	int	errors;			/* total # of errors curent pass */
extern	int	s_errors;		/* total # of soft errors curent pass */

void	clear_hist()			/* Clear error history */
{
	idx_err_list = -1;
}

void	err_hist(error_number)		/* Record error */
char	error_number;
{
	register ERR_LIST *ep;

	if (idx_err_list < MAX_HISTORY)
	{
		idx_err_list++;		/* Point to next error entry */	
		ep = &err_list[idx_err_list];
		ep->err_no = error_number;
		ep->pass_no = pass;
		ep->test_ch = commands[cmd_index];
	}
}

int soft_error(i)	/* Returns TRUE if soft or no error,*/
			/*	 else FALSE */
register int i;
{	
	register int sft_error;

	i &= 0x7F;			/* Clear unneded bits		*/
	if (i == 0) return(1);		/* No error reported as soft	*/
	sft_error = (	(i == 0x18) ||	/* Correctable ECC error	*/
			(i == 0x22) ||	/* Soft Header error in read	*/
			(i == 0x23) ||	/* Soft Header error in write	*/
			(i == 0x24) ||	/* Soft Data error		*/
			(i == 0x28));	/* Soft DMA error		*/
	return(sft_error);
}

void	display_history()	/* Display error history */
{
	int	i;
	register ERR_LIST *ep;
	int	line_count;
	char	junk[80];

	line_count = 0;
	if (idx_err_list >= 0)	/* If any errors exit */
		for (i=0;i <= idx_err_list;i++)
		{
			ep = &err_list[i];
			puts("Pass: ");
			putint(ep->pass_no);
			puts(" 	Error: ");
			putbhex(ep->err_no);
			puts("  Command: ");
			CDPutChar(ep->test_ch);
			puts("\n");
			if ((line_count++) > 20)
			{
				puts("Press <RETURN> to continue");
				gets(junk);
				line_count = 0;
			}
		}
}

int new_error(block_number,err_no)
register long block_number;
int err_no;
{
	register WDCMD *cbp;
 
	cbp = CMDBLKPADDR;

	if (!record_bad_blocks || soft_error(err_no)) return(1);
	if (findlong(block_number,bad_blocks,idx_bad_blocks) == -1)
	{
		addlong(block_number,bad_blocks,&idx_bad_blocks,
			((sizeof(bad_blocks)/sizeof(bad_blocks[0]))-1));
		return(1);
	}
	else	return(0);
}

void derror(s,i)
char *s; int i;
{
	puts("\n");
	puts(s);
	puts(" error: ");
	putbhex(i);
	puts("\n");
	putcmdblk();
	if (!soft_error(i)) ++errors;
}

void block_err(cp,sn,i)
char *cp; long sn; int i;
{
	if (new_error(sn,i))
	{
		err_hist((char)geterrbits());	/* Record block error */
		printf("%s error, block #%ld, error #%2x",cp,sn,i);
		puts("\n");
		puts(cp);
		puts(" error: block=");
		putlong(sn);
		puts(", i=");
		putbhex(i);
		if (soft_error(i))
		{
			s_errors++;
			printf(" soft");
			puts(" soft");
		}
		else
			++errors;
		puts("\n");
		putcmdblk();
	}
}

void rd_blk_err(sn,i)
long sn; int i;
{
	block_err("read",sn,i);
}

void wrt_blk_err(sn,i)
long sn; int i;
{
	block_err("write",sn,i);
}

void compare_error(sn,cp,bp,i)
register long sn;
register int i;
register char *cp, *bp;
{
	if (new_error(sn,i))
	{
  printf("Compare error, block #%ld, addr %lx val %2x != addr %lx val %2x\n",
	sn,cp,*cp,bp,*bp);
		err_hist((char)0);		/* Record compare error */
		stop_test |= stop_on_comp_error;
		puts("\nread data compare error block=");
		putlong(sn);
		puts(", 0x");
		putlhex(cp);
		puts(":0x");
		putbhex(*cp);
		puts(" != 0x");
		putlhex(bp);
		puts(":0x");
		putbhex(*bp);
		puts(", offset=");
		putint(i);
		puts("\n");
		++errors;
	}
}
