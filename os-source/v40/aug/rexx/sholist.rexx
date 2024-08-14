/*   ShoList.rexx -  updated for ARexx Revision 1.10 - 9-October-1989 */

/*      copyright 1989 Richard Lee Stockton and Gramma Software.      */
/*    This code is freely distributable as long as this copyright     */
/*   notice remains, unchanged, at the start of the code. Thank you.  */

/*  'rx ShoList ?' will show USAGE. Output may be redirected to file  */
/*                  ie, 'rx ShoList >ram:ports p' writes a formatted  */
/*                    textfile of the current ports to a file in ram. */


/* If the support lib not found then quit helpfully. */

CALL ADDLIB('rexxsupport.library',0,-30,0)
IF ~show('L',"rexxsupport.library") THEN
  DO
    SAY 'libs:rexxsupport.library is not available'
    EXIT 10
  END


/* These are the USAGE strings we output if we see a '?', or a bad letter */

LF     = '0A'x
USAGE1 = LF"  ARexx USAGE: rx ShoList [ACDFHILMPRSTVWX?]"LF
USAGE2 = ,
" A=directories  C=clip list    D=devices  F=open files     H=handlers"LF,
"I=interrupts   L=libraries    M=memory   P=ports          R=resources"LF,
"S=semaphores   T=ready_tasks  V=volumes  W=waiting_tasks  X=REXX tasks",
LF || LF"                ShoList with no argument shows ALL"



/************************  Program starts here  **************************/


/* get the argument from CLI and make it all caps. (ARG=PARSE ARG UPPER) */

ARG x


/* if no argument, give 'em everything */

IF length(x)=0 THEN x = 'ACDFHILMPRSTVWX'


/* if '?', output USAGE stuff and die */

IF (x=='?') THEN DO
                    SAY USAGE1
                    SAY USAGE2
                    EXIT 5
                 END


/* Take each letter in the argument, one at a time */

DO i=1 TO length(x)
   y=substr(x,i,1)


/* Select an appropriate title for this letter */

   SELECT
       WHEN y='A' THEN title = 'Assigned Directories [DOS list]'
       WHEN y='C' THEN title = 'Clips [AREXX list]'
       WHEN y='D' THEN title = 'Device Drivers'
       WHEN y='F' THEN title = 'Open Files [local]'
       WHEN y='H' THEN title = 'Handlers [DOS list]'
       WHEN y='I' THEN title = 'Interrupts'
       WHEN y='L' THEN title = 'Libraries'
       WHEN y='M' THEN title = 'MemoryList Items'
       WHEN y='P' THEN title = 'Ports'
       WHEN y='R' THEN title = 'Resources'
       WHEN y='S' THEN title = 'Semaphores'
       WHEN y='T' THEN title = 'Ready Tasks'
       WHEN y='V' THEN title = 'Volumes [DOS list]'
       WHEN y='W' THEN title = 'Waiting Tasks'
       WHEN y='X' THEN title = 'REXX Tasks'
       OTHERWISE
         DO          /* Bad Letter in argument. Complain and die */
           SAY
           SAY '  Usage Error!'  USAGE1 ' --->' y '?'
           SAY USAGE2
           EXIT 10
         END
   END


/* ARexx scans system lists. This is where the good stuff gets done. */

   IF y='X' THEN CALL AStatus()
   ELSE IF((y='C')|(y='F')) THEN list = show(y,,';')
                            ELSE list = showlist(y,,';')


/* everything below is just formatting and printing the list to the CLI */
/* based on the longest name in the list, and the number of list items. */

   listlength=length(list)    /* length in characters of the whole list */
   longest=0
   j=1
   items=0
   position=1

   DO WHILE(position>0)      /* how many total and how long is longest? */
       position=pos(';',list,j) /* <-- returns 0 when the list runs out */
       IF((position-j)>longest) THEN longest=position-j
       IF(position>0) THEN j=position+1
       items=items+1
   END
   IF((listlength+1-j)>longest) THEN longest=listlength+1-j /* last item */

   columns = 77%(longest+2)           /* we assume an 80 column display */
   position=1
   count=0
   j=1
   line = ""
   IF(listlength==0) THEN items=0
   SAY '      -*-*-*-' items title  '-*-*-*-'
   IF(listlength==0) THEN ITERATE         /* no list, go to next letter */
   DO WHILE(position>0)
       position=pos(';',list,j)
       IF(position>0) THEN
         DO
           nextItem = ""
           nextItem = left(substr(list,j,position-j),longest)
           IF LENGTH(STRIP(nextItem))==0 THEN
             nextItem = left("<blank>",longest)
           line = line || nextItem || '  '
           j = position + 1
         END
       ELSE line = line || left(substr(list,j),longest)
       count = count + 1
       IF(count=columns) THEN
         DO
           SAY line
           count=0
           line=""
         END
   END
   IF(count>0) THEN SAY line        /* Only print when the line is full */
END
EXIT

/* AStatus.rexx   ARexx task list display   by BaudMan */

AStatus:
RxsBase = findlib("rexxsyslib.library")
Call RxsOffsets()
TaskList = import(offset(RxsBase,RxsBase.rl_TaskList),4)
n=1
list=""
do j=1 TO 999 WHILE( import(TaskList,4) ~= NULL())
    name=Import(Import(Offset(TaskList,56),4)) 
    if name='' then do
        name='String_'n
        n=n+1
    end
    IF j=1 THEN list=name
    ELSE list=list';'name
    TaskList = import(TaskList,4)
END
RETURN;


/* Find a given library in the system - copied from Status.rexx */
findlib:
    parse arg tofind
    execbase = import('00000004'x,4)
    nodebase = import(offset(execbase, 378), 4)

    do while(import(nodebase,4) ~== NULL())
        if import(import(offset(nodebase,10),4)) == tofind then
            return nodebase
        nodebase = import(nodebase,4)
    end

    say 'Could not find' tofind
    exit(20)


RxsOffsets:
RxsBase.rl_TaskList =168/* List     */
RxsBase.rl_NumTask  =182    /* WORD     */
RxsBase.rl_LibList  =184    /* List     */
RxsBase.rl_NumLib   =198    /* WORD     */
RxsBase.rl_ClipList =200    /* List     */
RxsBase.rl_NumClip  =214    /* WORD     */
RxsBase.rl_MsgList  =216    /* List     */
RxsBase.rl_NumMsg   =230    /* WORD     */
RETURN;

/* end of ShoList.rexx   |   28 August 1990 */
