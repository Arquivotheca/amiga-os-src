/*
 * This file contains the set
 * command processing functions
 */
#include	"ed.h"

extern int      tabsize;     /* Tab size (0: use real tabs)  */
extern int	CheckCase;
extern int	indentsize;
extern int	nkeybound;	/* number of keys bound */
extern int	RememberFile;

int set(f,n)
    int f,n;
{
UBYTE buffer[80];
int s,i,j;
EWINDOW *wp;

    if((s=mlreply("Set what: ",buffer,78))!=TRUE)return(FALSE);
	if((i=nwspace(buffer) <0))return(FALSE); /* eat leading white space */

        j=nextarg(&buffer[i]);	/* find first argument */
	if(!strnicmp(&buffer[i],"wrap",4)) {
		i=j;
		if(!i)s=mlgetnumber("Column: ",fillcol);
                else s=atoi(&buffer[i]);
		if(s>=0)fillcol=s;
 	}
    	else if(!strnicmp("case",&buffer[i],4)) {
	    i=j;
	    if(!i) s=mlreply("Check case: ",buffer,78);
	    if((s)&&(!strnicmp(&buffer[i],"on",2)))CheckCase=TRUE;
	    else CheckCase=FALSE;
	}
    	else if(!strnicmp(&buffer[i],"backup",6)) {
	    i=j;
	    if(!i) s=mlreply("Backup: ",buffer,78);
	    if((s)&&(!strnicmp(&buffer[i],"safe",4)))BackupBefore=0;
	    else if((s)&&(!strnicmp(&buffer[i],"on",2)))BackupBefore= -1;
	    else BackupBefore=1;
	}
    	else if(!strnicmp(&buffer[i],"tab",3)) {
	    i=j;
	    if(!i)s=mlgetnumber("Tab: ",tabsize);
            else s=atoi(&buffer[i]);
	    if(s >= 0)tabsize=s;
	}
    	else if(!strnicmp(&buffer[i],"indent",6)) {
	    i=j;
	    if(!i)s=mlgetnumber("Indent: ",indentsize);
            else s=atoi(&buffer[i]);
	    if(s >= 0)indentsize=s;
	}
    	else if(!strnicmp(&buffer[i],"left",4)) {
	    i=j;
	    if(!i)s=mlgetnumber("Left margin: ",LeftMargin);
            else s=atoi(&buffer[i]);
	    if( (s<0) || (s>=RightMargin)) return(FALSE);
	    LeftMargin = s;
	    curwp->w_flag |= WFHARD;
	    lmargin();
	}
    	else if(!strnicmp(&buffer[i],"right",5)) {
	    i=j;
	    if(!i)s=mlgetnumber("Right margin: ",RightMargin);
            else s=atoi(&buffer[i]);
	    if ((s<LeftMargin) || (s < 0)) return(FALSE);
	    if (s > term.t_ncol) RightMargin= term.t_ncol;
	    else RightMargin = s;
	    curwp->w_flag |= WFHARD;
	    lmargin();
	    wp = wheadp;                            /* Update mode lines.   */
	    while (wp != NULL) {
	    if (wp->w_bufp == curbp)
		wp->w_flag |= WFMODE;
		wp = wp->w_wndp;
	    }
	}
#if AMIGA
	else if(!strnicmp(&buffer[i],"screen",6)) {
		return(SetLace(TRUE,3));
	}
	else if(!strnicmp(&buffer[i],"interlace",9)) {
	    if(!(i=j))SetLace(FALSE,0);
	    else if(!strnicmp(&buffer[i],"on",2)) SetLace(TRUE,2);
	    else SetLace(TRUE,1);
	}
	else if(!strnicmp(&buffer[i],"lastfile",8)) {
	    i=j;
	    if(!i) s=mlreply("Remember last file: ",buffer,78);
	    if((s)&&(!strnicmp(&buffer[i],"on",2)))RememberFile=TRUE;
	    else RememberFile=FALSE;
	}
#endif
	else if(!strnicmp(&buffer[i],"mode",4)) {
	    i=j;
	    if(!i) if( (s=mlreply("Mode: ",buffer,78))==ABORT)return(FALSE);
    	    if (!s)curbp->b_mode=0;   
	    else {
		while (i < strlen(buffer)) { 
	    	    if((buffer[i]==' ') || (buffer[i]=='|')) i++;
	    	    if(buffer[i]=='-') {
			i++;
    			if(!strnicmp(&buffer[i],"cmode",5)) {
		    	curbp->b_mode &= ~CMODE;
		    	i +=5;
			}
    			else if(!strnicmp(&buffer[i],"wrap",4)) {
		    	curbp->b_mode &= ~WRAPMODE;
		    	i += 4;
			}
		    }
    	    	    else {
    			if(!strnicmp(&buffer[i],"cmode",5)) {
		    	curbp->b_mode |= CMODE;
		    	i += 5;
			}
    			else if(!strnicmp(&buffer[i],"wrap",4)) {
		    	curbp->b_mode |= WRAPMODE;
		    	i += 4;
			}
		        else i++;
		    }

		}
	    }
    	curwp->w_flag |= WFMODE;
	}
        else {
	    mlwrite("Error: unknown variable to set");
	    return(FALSE);
	}
        return(TRUE);
}
