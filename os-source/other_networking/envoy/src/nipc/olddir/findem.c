
#include <exec/types.h>
#include <stdio.h>

int strfnd(char *sin, char *sfor,int lkat)
{

 int lnfor, lnin;
 int x;

 lnfor = strlen(sfor);
 lnin = strlen(sin);

 for (x = lkat; x < lnin; x++)
 {
    if (!strncmp(&sin[x],sfor,lnfor))
        return(x);
 }

 return(-1);

}


main(argc,argv)
char **argv;
int argc;
{

 char tolookfor[200][80];
 char oneline[256];
 int x,y;

 FILE *tmp1;
 FILE *tmp2;

 x = 0;
 while (!feof(stdin))
 {
    gets(tolookfor[x]);
    x++;
 }

 tmp2 = fopen("t:fetemp","w");
 tmp1 = fopen(argv[1],"r");

 while(!feof(tmp1))
 {
    fgets(oneline,256,tmp1);
    for (y = 0; y < x-1; y++)
    {
        int sat=0;
        while(TRUE)
        {
            int z;
            if ((z=strfnd(oneline,tolookfor[y],sat)) >= 0)
            {

                char p1[256],p2[256];
                sat = z+5;

                if (z)
                {
                    strncpy(p1,oneline,z);
                    p1[z]=0;
                }
                else
                    p1[0]=0;
                strmid(oneline,p2,z+1,160);
                sprintf(oneline,"%sgb->%s",p1,p2);
            }
            else
                break;
        }
    }
    fprintf(tmp2,"%s",oneline);
 }

 fclose(tmp1);
 fclose(tmp2);

 sprintf(oneline,"copy t:fetemp to %s",argv[1]);
 printf("%s\n",oneline);
 system(oneline);

}

