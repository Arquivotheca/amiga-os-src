#include <stdio.h>
/****************************************************************/
/*		Copyright Dale Luck                             */
/*   no portion of this file can be redistributed without       */
/*   permission from author, Dale Luck                          */
/****************************************************************/

/* program originally written in Pascal */
/* That should explain most wierdnesses */
/* Revision history : */
/* Somtime in 1987 parts redone to be more efficient */
/* later in 1987 changed pieces to make to compile under manx C */
/* April 18, 1988    changed pieces to compile under lattice 4.0 */
/*  		     hope it still compiles under manx, it still */
/*		     compiles under Greenhills */
/*	also linted it a little */



char copyright[] = "Copyright 1987,1988 Dale Luck.";
char copyright2[] = " This version free to recopy, as long as ";
char copyright3[] = " file spp.doc, is copied too.";

#define FALSE 0
#define TRUE 1

#define NEWSTUFF

#define  num_of_symbols 7
#define MAXCHARS    256
#define SYMBLSIZ    6
/*#define makelabel   cpystr(ibuff,lab,":",NULL); printbuff();*/
#define makelabel {strcpy(ibuff,lab);strcat(ibuff,":");printbuff();} 
/*#define DEBUG*/
/* type */
char k=0;
char  symbol[SYMBLSIZ+1]= {0};
char  *sym_table[num_of_symbols]=
{
    "repeat",
    "until",
    "while",
    "whend",
    "if",
    "else",
    "endif"
};

char  save_buff[MAXCHARS]={0},ibuff[MAXCHARS]={0};
int repcount=0,whilecount=0,ifcount=0,z=0,buff_index=0,
    i=0,j=0,l=0;
char lab[9]={0};  /* 8 character symbol and null for 9th */
int factor[5]={0};
int lab_stack[100]={0};
int tos=0;
int lng=0;
int first_space=0;  /* pnts to character after preprocessor symbol */
char *sptr=0;   /* pnts to preprocessor symbol first character in ibuff */
char cc1=0,cc2=0;
int un_signed=0;
FILE *fin=0;
FILE *fout=0;
int error=0;
int verbose = 0;
int debug = 0;

void printbuff()
{
    register char *c;
    c = ibuff;
    /* discard empty lines */
    while  (wspace(*c++));
    if (*c == 0)
        return;
    c = ibuff;
	fputs( c, fout);
    putc('\n', fout);
}

int wspace(c)
char c;
{
    if (c == ' ')
        return(1);
    if (c == '\t')
        return(1);
    return(0);
}

void insert(k,i)
char *k;
int i;
{
    int j;
    for(j = 0;j<6;j++)
        ibuff[i-1+j] = k[j];
}

void putlabl(i)
int i;
{
    char *l,*s;
    l = lab;
    s = &ibuff[i];
    do
    {
        *s++ = *l++;
    }
    while(*l != 0);
    *s = 0;
}

void genlabl(j,s)
int j;
char *s;
{
    int i,k,l;
#ifdef DEBUG
    printf("generate label\n");
#endif
    i = 0;
    do
    {
        lab[i] = s[i];
        i = i+1;
    }
    while(!(s[i]==0));
    for(k = 0;k<5;k++)
    {
        l = j / factor[k];
        lab[i] = '0'+l;
        j = j - l*factor[k];
        i = i+1;
    }
    lab[i] = 0;
}

void strip_label()
{
    char *cptr;
    cptr = ibuff;
    if (!wspace(*cptr)) 
    {
        /* search for end of label */
        while (!wspace(*cptr) && (*cptr != 0))
            cptr++;
        /* assume : is already present */
        *cptr = 0;
        printbuff();
    }
}

void spacebuff(s)
char *s;
{
	for ( i=0 ; i<MAXCHARS ; i++)	*s++ = ' ';
}

void space(s)
char *s;
{
    for(;s<&ibuff[MAXCHARS-2];*s++=' ');
    ibuff[MAXCHARS-1] = 0;
}

void gencond(st)
char *st;
{
    int     skip;
	register char c;
    char *s,*k,*j,*t;
#ifdef DEBUG
    printf("generate cond code\n");
#endif
    lng = 0;
    skip = 0;
    un_signed = 0;
    j = &st[first_space];   /* skip white space */
    while(wspace(*j))
        j++;    /* look for relation */
    s = &ibuff[buff_index];
    k = &ibuff[buff_index+4];
    space(s);
    t = j;

    do
    {
        if (*t=='.')    /* process forced condition code */
        {
            *t++ = ' '; /* kill the '.' */
            c = *t; /* look for .cs .cc etc */
            if( (c=='c')||(c=='v')||(c=='p')||(c=='m')||(c=='P')
                    || (c=='M')||(c=='C')||(c=='V')) 
            {
                cc1 = c;
                cc2 = t[1];
                *t = ' ';
                t[1] = ' ';
                skip = 1;
                if (j==t-1)
                    j = j+3;
                t = t+2;
            }
            else
            {
                if ((c=='e')||(c=='E')) 
                {
                    lng = 1;
                    *t = ' ';
                    t[1] = ' ';
                    if (j==t-1)
                        j=j+3;
                    t = t+2;
                }
                else
                {
                    if ((c=='u')||(c=='U'))
                    {
                        un_signed = 1;
                        *t = ' ';
                        t[1] = ' ';
                        if (j==t-1)
                            j=j+3;
                        t = t+2;
                    }
                    else    
                    {
                        k = s+6;
                        s[3] = '.';
                        s[4] = *t;
                        s[5] = ' ';
                        *t = ' ';
                        if (j==t-1)
                            j = j+2;
                        t = t+1;
                    }
                }
            }
        }
        else
            t++;
    }
    while(!(*t==0));
    if (skip==0) 
    {
        t = j;
        c = *t;
        if ((c!='<')&&(c!='>')&&(c!='=')) 
        {   /* gather first symbol of relation */
            do
            {
do_again:
                *k++ = *t++;
                c = *t;
            }
            while(!( (c=='<')||(c=='>')||(c=='=')||(c==0)||wspace(c)));
	if ( (c == t[1]) &&
		((c == '<') || (c == '>')) )
	{
		/* leave asm macros alone */
		*k++ = c;
		*k++ = c;
		*k = 0;	/* null terminate for now */
		t += 2;
		goto do_again;
	}
            while (wspace(*t))
                t++;
            if ((*t!=0)&&(*t!='<')&&(*t!='>')&&(*t!='=')&&(*k!=0))
                *k++ = ' ';
            /* copy something else */
            while ((*t!='<')&&(*t!='>')&&(*t!='=')&&(*t!=0)&&(*k!=0))
                *k++ = *t++;
		*k = 0; 	/* null terminate again */
            /*if ((*t==0)||(*k==0)) */   /* Dale 4-8-90 */
            if (*t==0)
            {
                cc1 = '<';
                cc2 = '>';
                goto lab1;
            }
            else
            {
                cc1 = *t++;
                cc2 = *t;
                if ((cc2=='<')||(cc2=='>')||(cc2=='='))
                    t++;
                while (*t==' ')
                    t++;
                /* process 2nd parameter of relation */
                if ((*t=='#')&&(t[1]=='0')&&(wspace(t[2])||(t[2]==0))) 
                {   /* use tst instruction */
lab1:               *s++ = 't';
                    *s++ ='s';
                    *s++ ='t';
                    goto lab2;  /* reverse condition codes */;
                }
                else
                {
                    *k++ = ',';
                    while (*t!=' ')
                        *k++ = *t++;
		    *k = 0; /* another null terminator */
                    *s++ = 'c';
                    *s++ = 'm';
                    *s++ = 'p';
                }
            }
        }
        else
        {
            cc1 = *t++;
            cc2 = *t++;
lab2:       if (((cc1=='<')||(cc1=='>'))&&(cc2!='>')) 
            {   /*reverse sense here*/
                if (cc1=='<')
                    cc1 = '>';
                else
                    cc1 = '<';
            }
        }
    }
}

void tform(a,b,c,d,e)
char a,b,*c,*d;
int e;
{
#ifdef DEBUG
    printf("in tform a=%c b=%c unsigned=%d\n",a,b,e);
#endif
    if (a=='=')
    {
        *c = 'e';
        *d = 'q';
    }
    else
    {
        if (a=='<') 
        {
            if (b=='>')
            {
                *c = 'n';
                *d = 'e';
            }
            else
            {
                if (b=='=')
                {
                    if (e==0)
                        {
                        *c = 'g';
                        *d = 'e';
                        }
                    else
                        {
                        *c = 'h';
                        *d = 's';
                        }
                }
                else
                {
                    if (e==0)
                        {
                        *c = 'g';
                        *d = 't';
                        }
                    else
                        {
                        *c = 'h';
                        *d = 'i';
                        }
                }
            }
        }
        else
        {
            if (b=='=')
            {
                if (e==0)
                    {
                    *c = 'l';
                    *d = 'e';
                    }
                else
                    {
                    *c = 'l';
                    *d = 's';
                    }
            }
            else
            {
                if (e==0)
                    {
                    *c = 'l';
                    *d = 't';
                    }
                else
                    {
                    *c = 'l';
                    *d = 'o';
                    }
            }
        }
    }
    if ((a=='c')||(a=='v')||(a=='C')||(a=='V')) 
    {
        *c = a;	/* Dale added * to these 10-19-84 */
        *d = b;
    }
#ifdef DEBUG
	printf("exit tform c=%c d=%c\n",*c,*d);
#endif
}

void gencc(a,b,i)
char a,b;
int i;
{
    char *k;
    k = &ibuff[i];
    if ((a>='a')&&(a<='z'))
        a=a-'a'+'A';
    if ((b>='a')&&(b<='z'))
        b=b-'a'+'A';
    switch(a) 
    {
        case 'C' :
            *k++ = 'c';
            if (b=='C')
                *k++ = 's';
            else
                *k++ = 'c';
            break;
        case 'E' :
            *k++ = 'n';
            *k++ = 'e';
            break;
        case 'F' :
            *k++ = 't';
            break;
        case 'G' :
            *k++ = 'l';
            if (b=='T')
                *k++ = 'e';
            else
                *k++ = 't';
            break;
        case 'H' :
            *k++ = 'l';
            if (b=='I')
                *k++ = 's';
            else
                *k++ = 'o';
            break;
        case 'L' :
            if ((b=='S')||(b=='O'))
            {
                *k++ = 'h';
                if (b=='S')
                    *k++ = 'i';
                else
                    *k++ = 's';
            }
            else
            {
                *k++ = 'g';
                if (b=='E')
                    *k++ = 't';
                else
                    *k++ = 'e';
            }
            break;
        case 'M' :
            *k++ = 'p';
            *k++ = 'l';
            break;
        case 'N' :
            *k++ = 'e';
            *k++ = 'q';
            break;
        case 'P' :
            *k++ = 'm';
            *k++ = 'i';
            break;
        case 'T' :
            *k++ = 'f';
            break;
        case 'V' :
            *k++ = 'v';
            if (b=='C')
                *k++ = 's';
            else
                *k++ = 'c';
            break;
        } /*case*/
    if (lng) 
    {
        *k++ = ' ';
        *k++ = ' ';
    }
    else
    {
        *k++ = '.';
        *k++ = 's';
    }
    *k++ = ' ';
}
void cpbuff(to,from)
register char *to,*from;
{
    int i;
    for(i=0;i<MAXCHARS;i++)
        *to++ = *from++;
}

void repsrv() /*process repeat statement*/
{
    genlabl(repcount,"_re");
    strip_label();
    makelabel;
    lab_stack[tos] = repcount;
    tos = tos+1;
    repcount = repcount+1;
}

void until_srv()
{
    char i,j;
    tos= tos-1;
    genlabl(lab_stack[tos],"_re");
    gencond(save_buff);
    printbuff();
    tform(cc1,cc2,&i,&j,un_signed);
    cpbuff(ibuff,save_buff);
    ibuff[buff_index] = 'b';
    lng = 1;    /* let assembler figure out short or long */
    gencc(i,j,buff_index+1);
    putlabl(buff_index+6);
    printbuff();
}

void while_srv()
{
    int k;
    char i,j;
    genlabl(whilecount,"_wl");
    lab_stack[tos] = whilecount;
    tos = tos+1;
    whilecount = whilecount+1;
    strip_label();
    putlabl(0);
    ibuff[8] = ' ';
    k = buff_index;
    buff_index = 9;
    gencond(save_buff);
    buff_index = k;
    printbuff();
    tform(cc1,cc2,&i,&j,un_signed);
    cpbuff(ibuff,save_buff);
    ibuff[buff_index] = 'b';
    gencc(i,j,buff_index+1);
    lab[2] = 'e';
    putlabl(buff_index+6);
    printbuff();
}

void endwhile()
{
    int i;
    tos = tos - 1;
    genlabl(lab_stack[tos],"_wl");
    for(i = buff_index;i<16;i++)
        ibuff[i] = ' ';
    if (buff_index<15)
        buff_index = 15;
    ibuff[buff_index] = 'b';
    ibuff[buff_index+1] = 'r';
    ibuff[buff_index+2] = 'a';
    ibuff[buff_index+3] = ' ';
    for(i = 0;i< 11;i++)
        ibuff[buff_index+4+i] = lab[i];
    printbuff();
    lab[2] = 'e';
    makelabel;
}

void ifsrv()
{
    char    i,j;
#ifdef DEBUG
    printf("in ifsrv\n");
#endif
    genlabl(ifcount,"_if");
    gencond(save_buff);
    printbuff();
    tform(cc1,cc2,&i,&j,un_signed);
    space(&ibuff[buff_index]);
    ibuff[buff_index] = 'b';
    gencc(i,j,buff_index+1);
    ibuff[buff_index+5] = ' ';
    putlabl(buff_index+6);
    printbuff();
    lab_stack[tos] = ifcount;
    ifcount = ifcount + 1;
    tos = tos +1 ;
}

void else_srv()
{
    int i;
    strip_label();
    lng = 0;
    for(i = 1;i<72;i++)
        if (ibuff[i]=='.') 
            if ((ibuff[i+1]=='e')||(ibuff[i+1]=='E'))
                lng = 1;
    spacebuff(ibuff);
    if (lng)
        insert("bra   ",16);
    else
        insert("bra.s ",16);
    genlabl(ifcount,"_if");
    for(i = 0;i<11;i++)
        ibuff[21+i] = lab[i];
    printbuff();
    spacebuff(ibuff);
    tos = tos - 1;
    genlabl(lab_stack[tos],"_if");  /*if*/
    makelabel;
    lab_stack[tos] = ifcount;
    tos = tos+1;
    ifcount = ifcount+1;
}

void endif()
{
    strip_label();
    spacebuff(ibuff);
    tos = tos-1;
    genlabl(lab_stack[tos],"_if");
    makelabel;
}

void (*func[])() = 
{
	repsrv,
	until_srv,
	while_srv,
	endwhile,
	ifsrv,
	else_srv,
	endif,
	printbuff
};

void sppfile()
{
    int qwe;
    register char *cptr;

    for(;;)
    {
        cptr = ibuff;
        qwe = 0;
        for(;;)
        {
            if ((k = getc(fin)) == EOF)
                return;
            if (k == '\n')
            {
                *cptr = 0;
                break;
            }
            *cptr++ = k;
            if (++qwe == MAXCHARS)
            {
                fprintf(stderr,"line too long for preprocessor\n");
                error |= 1;
                while (getc(fin) != '\n');
                break;
            }
        }
        cptr = ibuff;
        if ((*cptr==0)||(*cptr=='*')||(*cptr==';'))
        {
            if (verbose)	printbuff();
        }
        else
        {
            /*search for preprocess invocation*/
            /* skip labels in first column */
            while (!wspace(*cptr)&&(*cptr != 0))
                cptr++;
            if (*cptr==0)
                printbuff();
            else
            {
                /*skipblanks*/
                while (wspace(*cptr)&&(*cptr!=0))
                    cptr++;
                if (*cptr==0)
                    printbuff();
                else
                {
                    /* get tentative symbol */
                    buff_index = (long)(cptr) - (long)(&ibuff[0]);
                    j = 0;
                    while ((*cptr != 0)&&!wspace(*cptr)&&(j<SYMBLSIZ))
                    {
                        symbol[j++] = *cptr++;  /* convert upper to lower */;
                    }
                    symbol[j] = 0;
                    first_space = (long)(cptr)-(long)(&ibuff[0]);
                    /*check for complete symbol*/
                    if ((j<SYMBLSIZ)||wspace(*cptr)||(*cptr == 0)) 
                    { /*symbol was delimited by a space or eoln*/
                        for(l = 0;l<num_of_symbols;l++)
                            if (strcmp(symbol,sym_table[l]) == 0)
                                break;
                        if (l!=num_of_symbols) 
                        {
                            cpbuff(save_buff,ibuff);
			  if (verbose)
			  {
                            if (ibuff[0]==' ') 
                            {   /* space out label to put in comment *> */
                                j = 0;
                                while (ibuff[j]==' ')
                                    ibuff[j++] = ' ';
                            }
                            else
                                if (ibuff[0] == '\t')
                                {
                                    /* shift line to the right by two chars */
                                    for(j=MAXCHARS-1;j>1;j--)
                                        ibuff[j] = ibuff[j-2];
                                    ibuff[1] = '>';
                                }
                            ibuff[0] = '*';
                            if (ibuff[1]==' ')
                                ibuff[1] = '>';
                            printbuff();
#ifdef NOLIST
                            ibuff[2] = 'n';
                            ibuff[3] = 'o';
                            ibuff[4] = 'l';
                            printbuff();    /*suppress prep listing*/
#endif
                            cpbuff(ibuff,save_buff);
			  }
                        }
#ifdef NEWSTUFF
			(*func[l])();
#else
                        switch (l)
                        {
                            case 0:
                                repsrv();
                                break;
                            case 1:
                                until_srv();
                                break;
                            case 2:
                                while_srv();
                                break;
                            case 3:
                                endwhile();
                                break;
                            case 4:
                                ifsrv();
                                break;
                            case 5:
                                else_srv();
                                break;
                            case 6:
                                endif();
                                break;
                            case num_of_symbols :
                                printbuff();
                                break;
                        } /*case*/
#endif
#ifdef NOLIST
                        if (l!=num_of_symbols) 
                        {
                            spacebuff(ibuff);
                            ibuff[2] = 'l';
                            ibuff[3] = 'i';
                            ibuff[4] = 's';
                            ibuff[5] = 't';
                            printbuff();
                        }
#endif;
                    }
                    else
                        printbuff();
                }
            }
        }
    }
}


void main(argc, argv)
int argc;
char *argv[];
{
	char **argp;
	char *outfilename = 0;
	char *infilename = 0;
    factor[0] = 10000;
    factor[1] = 1000;
    factor[2] = 100;
    factor[3] = 10;
    factor[4] = 1;

    repcount = 0;
    whilecount = 0;
    ifcount = 0;
    tos = 1;

    error = 0;
    fin = stdin;
    fout = stdout;
	for (argp = argv+1; argp < argv+argc; argp++)
	{
		if ((*argp)[0] == '-')
		{
			switch ((*argp)[1])
			{
				case 'd':	debug = TRUE;	break;
				case 'v':	verbose = TRUE; break;
				case 'o':	argp++;
						outfilename = *argp;
						break;
				default:
					fprintf(stderr,"unknown option:%c\n",(*argp)[1]);
					exit(1);
					break;
			}
		}
		else if ((*argp)[0] == '?')
		{
			printf("usage: spp <infile.asm >file\n");
			printf("usage: spp infile.asm -o file\n");
			printf("-v for verbose,preserve comments, etc \n");
			exit(0);
		}
		else /* get input file name */
			infilename = *argp;
	}
	if (infilename && !(fin = fopen(infilename, "r"))) 
        {
        	fprintf(stderr,"can't open %s\n", infilename);
		exit(1);
        }
	if (outfilename && !(fout = fopen(outfilename, "w"))) 
        {
        	fprintf(stderr,"can't open %s\n", outfilename);
		exit(1);
        }
	sppfile();
    if (fout != stdout)
        fclose(fout);
    exit(error);
}
