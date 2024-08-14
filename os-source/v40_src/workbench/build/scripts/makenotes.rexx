/* script to generate release notes */


/*****************************************************************************/


LastNoteFile = 'V:src/workbench/build/scripts/.lastnote'
VAXDir       = 'VAX:'
KeyLine      = 'SUBJECT: WB:'


/*****************************************************************************/


/* get the number of the last release note that was scanned */
IF OPEN(in,LastNoteFile,READ) = 0 THEN DO
  SAY "Could not open "LastNoteFile
  EXIT
END

lowNote = READLN(in)
CALL CLOSE(in)


/*****************************************************************************/


/* get the number of the highest release note currently out there */
'RSH >PIPE:highNote cbmvax grep amiga.release /local/lib/news/active'

IF OPEN(in,"PIPE:highNote",READ) = 0 THEN DO
  SAY "Could not open PIPE:highNote file"
  EXIT
END

highNote = WORD(READLN(in),2)
CALL CLOSE(in)


/*****************************************************************************/


/* create a file in T: containing all the release notes we want */

'Echo >'VAXDir'.makenotes ;'

IF OPEN('sfile',VAXDir'.cpscript',WRITE) = 0 THEN DO
  SAY "Could not open ' VAXDir '.cpscript"
  EXIT
END

DO i = lowNote+1 TO highNote
  CALL WRITELN('sfile','cat >>.makenotes /news/amiga/release/' || i);
END

CALL CLOSE('sfile');

'RSH cbmvax csh .cpscript'

'MakeDir T:Notes'
'Copy 'VAXDir'.makenotes T:makenotes'
'Delete 'VAXDir'.makenotes 'VAXDir'.cpscript QUIET FORCE'


/*****************************************************************************/


IF OPEN('sfile','T:makenotes',READ) = 0 THEN DO
  SAY "Could not open T:makenotes"
  EXIT
END

DO UNTIL EOF('sfile')
  CALL FindArticle
  CALL SkipHeaders
  CALL OutputArticle
END

CALL CLOSE('sfile')

'Echo >'LastNoteFile highNote

'Join T:Notes/#? AS RAM:ReleaseNotes'
'Delete T:makenotes T:Notes ALL QUIET'

RETURN


/*****************************************************************************/


FindArticle:

  DO UNTIL EOF('sfile')
    inline = READLN('sfile')
    IF (POS(KeyLine,UPPER(inline)) = 1) THEN DO
      IF (WORDS(inline) >= 3) THEN DO
        module = UPPER(WORD(inline,3))
        RETURN 1
      END
    END
  END

  RETURN 1


/*****************************************************************************/


SkipHeaders:

  DO UNTIL EOF('sfile')
    inline = READLN('sfile')
    IF (RIGHT(WORD(inline,1),1) ~= ':') THEN RETURN 1
  END

  RETURN 1


/*****************************************************************************/


OutputArticle:

  result = OPEN('outfile','T:Notes/' || module,APPEND)

  IF (result = 0) THEN DO
    result = OPEN('outfile','T:Notes/' || module,WRITE)
    CALL WRITELN('outfile','')
    CALL WRITELN('outfile',"---------")
    CALL WRITELN('outfile','')
  END; ELSE DO
    CALL WRITELN('outfile',"")
  END

  IF (result = 0) THEN DO
    SAY "Couldn't open T:Notes/" || module
    EXIT
  END

  DO UNTIL EOF('sfile')
    inline = READLN('sfile')

    IF (LEFT(inline,2)='--') THEN LEAVE
    IF (LEFT(inline,2)='==') THEN LEAVE
    IF (LEFT(inline,5)='~DNJ~') THEN LEAVE

    IF (LEFT(inline,1) ~= ';') THEN DO
      CALL WRITELN('outfile',inline)
    END
  END

  CALL CLOSE('outfile')

  RETURN 1
