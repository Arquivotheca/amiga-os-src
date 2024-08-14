#include "ed.h"
/* This file contains the key binding routines */

extern int	nkeybound;	/* number of keys bound */
extern WORD	*BindKey;
extern WORD	*TBindKey;

extern WORD 	upgradekey();

typedef struct  {
        short   k_code;                 /* Key code                     */
        int     (*k_fp)();              /* Routine to handle it         */
        struct IntuiText  *k_string;
}       KEYTAB;

extern KEYTAB keytab[];
extern int nkeytab;


WORD dobindkey(c)
WORD c;
{
int i;

for(i=0; i< nkeybound; i++) {
    if(c==BindKey[i])return(TBindKey[i]);
}
return(c);
}


int bindkey(f,n) 
int f,n;
{
UBYTE buffer[80];
int s,c,i,j,dup=FALSE;

    if(nkeybound >= NBINDS) {
	mlwrite("Too many bindings");
	return(FALSE);
    }
    if((s=mlreply("Bind: ",buffer,78))!=TRUE)return(FALSE);
    if((i=nwspace(buffer)<0))return(FALSE);
    if((s= FindCommand(&buffer[i]))<0) {
	mlwrite("No function found");
	return(FALSE);
    }
    if(!(j= nextarg(&buffer[i]))) {
	if(buffer[strlen(buffer)-1] == ' ')  {
	    j=strlen(buffer)-1;
	}
	else {
	    mlwrite("Key not found");
	    return(FALSE);
	}
    }
    c=upgradekey(&buffer[j]);
    for(i=0; i<nkeybound; i++) {
	if(c == BindKey[i]) {
	    TBindKey[i]=(WORD)s;
	    dup=TRUE;
	}
    }
    if(!dup) {
	BindKey[nkeybound]=(WORD)c;
	TBindKey[nkeybound]=(WORD)s;
	nkeybound++;
    }
    return(TRUE);
}

int whichbind(f,n)
    int f,n;
{
UBYTE buffer[80];
int s,c,i;
KEYTAB *ktp;

    if((s=mlreply("Describe key: ",buffer,78))!=TRUE)return(FALSE);
    if((i=nwspace(buffer)<0)) {
	if(buffer[strlen(buffer)-1] == ' ') 
	    i=strlen(buffer)-1; /* describe the space */
	else return(FALSE);
    }
    c=dobindkey(upgradekey(&buffer[i]));
    ktp = &keytab[0];   /* Look in key table.   */
    while (ktp < &keytab[nkeytab]) {
        if (ktp->k_code == c) {
	mlwrite("Function: %s",ktp->k_string->IText);
	return(TRUE);
	}
    ++ktp;
    }
mlwrite("Not bound");
return(FALSE);
}

int unbindkey(f,n)
int f,n;
{
    UBYTE buffer[80];
    int s,i,c,j;

    if(nkeybound <=0) return(TRUE);

    if((s=mlreply("Unbind: ",buffer,78))!=TRUE)return(FALSE);
    if((i=nwspace(buffer))<0) { 
	if(buffer[0]==' ')c = 32; /* unbind the space */
	else return(FALSE);
    }
    else {
        if(!strnicmp(&buffer[i],"all",3)) {
	    for(i=0; i< nkeybound; i++)BindKey[i]=TBindKey[i]=0;
	    nkeybound=0;
	    return(TRUE);
	}
        c=upgradekey(&buffer[i]);
    }

    for(i=0; i < nkeybound; i++) {
    	if(c == BindKey[i] ) {
	    nkeybound--;
	    for(j=i; j<nkeybound; j++) {
		BindKey[j]=BindKey[j+1];	/* remove it from the table */
		TBindKey[j]=TBindKey[j+1];
	    }
	    BindKey[j]=TBindKey[j]=0;
	    return(TRUE);
	}
    }
    mlwrite("Key is not bound");
    return(FALSE);
}

WORD upgradekey(string)
UBYTE *string;
{
WORD c;
ULONG flag=0;

    c= *string++;
    if (c == METACH ) { 
	flag |= META; 
	c = *string++; 
    }
    if (c == 0x18 ) { 
	flag=CTLX|flag; 
	c = *string; 
    }
    if (c < 32) c = (c+'@') | CTRL;
    if (flag) {
    if (c>='a' && c<='z')c -= 0x20; /* Force to upper */
    }
    c=c|flag;

    return(c);
}
