/*
 * REXX Script to check on the ROM size changes
 *
 * See the entries below for more information on how it runs.
 * This *should* work on non-WShell machines but I have not tested
 * that.  It *does* work on WShell machines.  Requires that the
 * build tools are in your path.
 */

/*
 * The ROM Space size includes space for the AutoVecs, CheckSum,
 * ROM Size, and a "perfect" split.
 * These add up to 8*4 + 4 + 4 + 12  (52 bytes)
 *
 * Also, the A3000 build includes the 4K of space for the SuperKickstart
 *
 * Also, the CDGS build includes the slop of a 1Meg dual split...
 */
BuildListCount=5
BuildLists.1='A3000'	; BuildSpace.1= 520140	/* The A3000 SuperKickstart build (must be first!) */
BuildLists.2='A600'	; BuildSpace.2= 524236	/* The A600 build (also A2000 and A500) */
BuildLists.3='A4000'	; BuildSpace.3= 524236	/* The A4000 build */
BuildLists.4='A1200'	; BuildSpace.4= 524236	/* The A1200 build lists */
BuildLists.5='CDGS'	; BuildSpace.5=1048200	/* The CDGS 1Meg build */

BuildDir='V40:disk/kickstart/'
BuildListDir='V40:disk/kickstart/buildlists/'
BuildLogDir='V40:disk/kickstart/romspace/'
PostingFile='V40:disk/kickstart/romspace/ROM_Space_Posting'

VAXHome='MKS:'	/* HOME DIR for the VAX */

/*
 * The posting file we will create
 */
CALL OPEN(ResultFile,PostingFile,WRITE)

CALL WRITELN(ResultFile,' ')
CALL WRITELN(ResultFile,'ROM Usage Changes:' DATE(W) DATE(N) TIME(N))
CALL WRITELN(ResultFile,' ')
CALL WRITELN(ResultFile,'                   ... from the ROM Space WatchDog ...')
CALL WRITELN(ResultFile,'-------------------------------------------------------------------------')

/* Changes count so we know when to post */
TotalChanges=0

DO loop=1 TO BuildListCount

  BuildChanges=0

  OldBuildSize=BuildSpace.loop
  NewBuildSize=BuildSpace.loop

  LogFile=BuildLogDir || 'Log.' || BuildLists.loop

  ModCount=0
  IF OPEN(FakeEXECIO,LogFile,READ)=1 THEN DO
    DO UNTIL EOF(FakeEXECIO)
      line=READLN(FakeEXECIO)
      PARSE VAR line module ';' size junk
      size=STRIP(size)
      IF LENGTH(size) > 1 THEN DO
        OldBuildSize=OldBuildSize-size
        ModCount=ModCount+1
        ModListName.ModCount=STRIP(module)
        ModListSize.ModCount=size
        ModListNew.ModCount=0
      END
    END
    CALL CLOSE(FakeEXECIO)
  END

  'Split V40:disk/kickstart/buildlists/' || BuildLists.loop 'TO' LogFile 'SIZESONLY'

  CALL OPEN(FakeEXECIO,LogFile,READ)
  DO UNTIL EOF(FakeEXECIO)
    line=READLN(FakeEXECIO)
    PARSE VAR line module ';' size junk
    size=STRIP(size)
    IF LENGTH(size) > 1 THEN DO
      module=STRIP(module)
      NewBuildSize=NewBuildSize-size
      found=0
      DO line=1 TO ModCount
        IF ModListName.line=module THEN found=line
      END

      IF found=0 THEN DO
        ModCount=ModCount+1
        ModListName.ModCount=module
        ModListSize.ModCount=0
        found=ModCount
      END

      ModListNew.found=size
    END
  END
  CALL CLOSE(FakeEXECIO)

  DO line=1 TO ModCount
    change=''
    module=ModListName.line
    oldsize=ModListSize.line
    newsize=ModListNew.line

    IF oldsize > newsize THEN DO
      /* We got smaller... :-) */
      change='-' RIGHT(oldsize-newsize,6)
    END

    IF oldsize < newsize THEN DO
      /* We got larger... :-( */
      change='+' RIGHT(newsize-oldsize,6)
    END

    IF LENGTH(change) > 1 THEN DO
      change=change '| ('RIGHT(newsize,6)')' LEFT(module,54)

      IF BuildChanges<1 THEN DO
        CALL WRITELN(ResultFile,' ');
        CALL WRITELN(ResultFile,'Changes in the' BuildLists.loop 'build:')
      END

      CALL WRITELN(ResultFile,change)

      BuildChanges=BuildChanges+1
      TotalChanges=TotalChanges+1
    END
  END

  IF BuildChanges > 0 THEN DO
    CALL WRITELN(ResultFile,'---------|---------------------------------------------------------------')
    change='+'
    size=OldBuildSize-NewBuildSize
    IF size < 0 THEN DO
      change='-'
      size=NewBuildSize-OldBuildSize
    END
    change=change RIGHT(size,6) '| ('RIGHT(NewBuildSize,6)') free in' BuildLists.loop 'build.'

    CALL WRITELN(ResultFile,change)
  END
END

CALL CLOSE(ResultFile)

IF TotalChanges > 0 THEN DO

  group='amiga.release'
  subject='ROM_Space_WatchDog'

  tmpfile=".news."||TIME(S)

  Waiting=10
  OPTIONS FAILAT 11

  /*
   * Wait here until CBMVAX is available
   */
  DO UNTIL Waiting=0
    'Ping >NIL: cbmvax'
    Waiting=RC
  END

  /* Put the file onto the VAX */
  CommandLine='Copy' PostingFile 'TO' VAXHOME||tmpfile
  CommandLine

  /* Now post the news... */
  CommandLine ='rsh <NIL: >NIL: HOST cbmvax CMD /usr/local/lib/news/inews -t 'subject' -n 'group' <'tmpfile
  CommandLine

  /* Now delete the file from the vax... */
  CommandLine='Delete' VAXHOME||tmpfile 'QUIET FORCE'
  CommandLine
END
