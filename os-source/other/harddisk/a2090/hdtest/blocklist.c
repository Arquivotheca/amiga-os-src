/* Routines used for maintaining a list of bad blocks */

/* Find long word in the array.  Returns index if found, -1 if not */
int findlong(item,array,array_end)
register long item;
long array[];
register int array_end;
{
	register int bad_index;		/* Index into the array */

	if (array_end >= 0)
	    for (bad_index = 0;
		(bad_index <= array_end) && (item != array[bad_index]);
		bad_index++);
/*	printf("item: %d array[0]: %d array_end: %d bad_index: %d\n",
		item,array[0],array_end,bad_index);*/
	return(((item == array[bad_index]) && (array_end > -1))
		? bad_index : -1);
}

/*
 * Add long word to a sorted array.  Returns 1 if successful, else 0.
 */

int addlong(item,array,array_end,array_size)
register long	item;
	 long	array[];
register int	*array_end;
	 int	array_size;
{
	int	success;
	register int	bad_index, i, j;

	success = 1;
	if (*array_end == -1)	/* if empty array */
	{
		array[0] = item;
		*array_end = 0;
	} else if (*array_end == array_size)
		success = 0;
	else
	{
		for (bad_index = 0;
			(bad_index <= *array_end) && (item > array[bad_index]);
			bad_index++);
		if (array[bad_index] != item)
		{		
			for (j = *array_end, i = *array_end + 1;
				j >= bad_index; i--,j--)
					array[i] = array[j];
			(*array_end)++;
			array[bad_index] = item;
		}
	}
/*	printf("item: %d array[0]: %d array_end: %d bad_index: %d\n",
		item,array[0],*array_end,bad_index);*/
	return(success);
}

/*
 * Display the passed bad block list, 6 numbers per line
 */

void display_bad(block_list,list_end)
long block_list[];
int list_end;
{
	register int	index;
	register int	column;
	char		outstr[20];

/*	printf("block_list[0]: %d block_list[1]: %d list_end: %d\n",
		block_list[0],block_list[1],list_end);*/
	if (list_end >= 0)
		puts("Bad Blocks:\n");
		for (index = column = 0; index <= list_end; index++)
		{
			sprintf(outstr," %9ld",block_list[index]);
			puts(outstr);
			if (++column > 5)
			{
				column = 0;
				puts("\n");
			}
		}
	if (column) puts("\n");
}

