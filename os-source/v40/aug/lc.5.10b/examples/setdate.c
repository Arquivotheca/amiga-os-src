#include "stdio.h"
/*
*	This is a sample program to set the date and time on the Amiga
*	There is lots of room for improvement, but it makes a nice
*	example of using the date and clock functions.
*/

static char *day_of_week[] = {	"Sun",
				"Mon",
				"Tue",
				"Wed",
				"Thu",
				"Fri",
				"Sat"
				};
void main()
{
	char d[8],date[9],time[13],buf[41];
	int a,b,c,e,f,g;
	short i;

	getclk(d);
	stpdate(date,3,&d[1]);
	stptime(time,2,&d[4]);
	i = d[0];
	printf("Current date is %s  %s\n",day_of_week[i],date);
	while(1)
	{
		printf("Enter new date (mm-dd-yy): ");
		fgets(buf,40,stdin);
		if(strlen(buf) < 2)
		{
			a = d[2];
			b = d[3];
			c = d[1] + 80;
			break;
		}
		if ((sscanf(buf,"%d-%d-%d",&a,&b,&c) != 3) || (c < 80))
		{
			printf("Invalid date\n");
			continue;
		}
		else break;
	}
	printf("Current time is: %s\n",time);
	while(1)
	{
		printf("Enter new time (hh:mm:ss): ");
		fgets(buf,40,stdin);
		if(strlen(buf) < 2)
		{
			e = d[4];
			f = d[5];
			g = d[6];
			break;
		}
		if((i = sscanf(buf,"%d:%d:%d",&e,&f,&g)) != 3)
			{
			if (i == 2) 
				{
				g = 0;
				break;
				}
			else
				{
				printf("Invalid time\n");
				continue;
				}
			}
		else break;
	}
	d[0] = 0;
	d[1] = c - 80;
	d[2] = a;
	d[3] = b;
	d[4] = e;
	d[5] = f;
	d[6] = g;
	d[7] = 0;
	if(chgclk(d))	printf("Error setting clock...\n");
}

