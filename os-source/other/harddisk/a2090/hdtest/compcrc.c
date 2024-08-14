
	/* temporary CRC routine, actually just returns checksum */

long comp_crc(crc_data,data_length)
char *crc_data; register short data_length;
{
	register short	i;
	register long	checksum;

	for (i = 0, checksum = 0L;i < data_length;
	     i++, checksum += (long)*crc_data++);
	return(checksum);
}
