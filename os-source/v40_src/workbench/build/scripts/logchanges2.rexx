
/* This REXX script compares two WB build log files and generates a list of
 * differences between them. The result is posted to amiga.release as a
 * WB Build Change List.
 */

PARSE ARG Version newrev oldrev

Version = STRIP(Version)
newrev  = STRIP(newrev)
oldrev  = STRIP(oldrev)

IF Version = "" THEN DO
  SAY "Requires Current Workbench Version Number As First Parameter"
  EXIT
END

IF newrev = "" THEN DO
  SAY "Requires Current Workbench Revision Number As Second Parameter"
  EXIT
END

IF oldrev = "" THEN DO
  oldrev = newrev-1
END


/*****************************************************************************/


BuildLogDir  = 'V:src/workbench/build/logfiles/'
PostingName  = 'wbchangelog'
PostingFile  = 'VAX:'PostingName
LogDateLine  = 3    /* date of a log file is on line 3       */
LogSkipLines = 8    /* number of lines to skip in a log file */


/*****************************************************************************/


/* Try to open the new log file */
NewLogFile = BuildLogDir || 'Log_'Version'.' || newrev || '-Regular'
IF OPEN(FakeEXECIO,NewLogFile,READ) = 0 THEN DO
  SAY "Could not open logfile '"NewLogFile"'"
  EXIT
END

/* now read and store all the lines from the new logfile */
newlinenum = 0;
DO UNTIL EOF(FakeEXECIO)
  newlines.newlinenum = READLN(FakeEXECIO)
  newlinenum = newlinenum + 1;
END

CALL CLOSE(FakeEXECIO)


/*****************************************************************************/


IF oldrev > 0 THEN DO
  /* Try to open the old log file */
  OldLogFile = BuildLogDir || 'Log_'Version'.' || oldrev || '-Regular'
  IF OPEN(FakeEXECIO,OldLogFile,READ) = 0 THEN DO
    SAY "Could not open logfile '"OldLogFile"'"
    EXIT
  END

  oldlinenum = 0;
  DO UNTIL EOF(FakeEXECIO)
    oldlines.oldlinenum = READLN(FakeEXECIO)
    oldlinenum = oldlinenum + 1
  END

  CALL CLOSE(FakeEXECIO)

END; ELSE DO
  oldlinenum = 0
  OldLogFile = "None"
END


/*****************************************************************************/


CALL OPEN(ResultFile,PostingFile,WRITE)
CALL WRITELN(ResultFile,' ')
CALL WRITELN(ResultFile,'Workbench 'Version'.'newrev' can now be installed from')
CALL WRITELN(ResultFile,'  SoftServe:Public/WorkbenchDisks or SoftPublic:WorkbenchDisks')
CALL WRITELN(ResultFile,' ')
CALL WRITELN(ResultFile,'Workbench Disk Change Log Generated On' DATE(W) DATE(N) 'At' TIME(N))
CALL WRITELN(ResultFile,' ')
CALL WRITELN(ResultFile,'Previous Build Log:' OldLogFile)
CALL WRITELN(ResultFile,'                    'Version'.' || oldrev || ' (' || SUBSTR(oldlines.3,18)')')
CALL WRITELN(ResultFile,' ')
CALL WRITELN(ResultFile,' Current Build Log:' NewLogFile)
CALL WRITELN(ResultFile,'                    'Version'.' || newrev || ' (' || SUBSTR(newlines.3,18)')')
CALL WRITELN(ResultFile,' ')
CALL WRITELN(ResultFile,'------------------------------------------------------------')
CALL WRITELN(ResultFile,' ')


/*****************************************************************************/


CALL WRITELN(ResultFile,'Changed Items Since Previous Build:')

DO i = 1 TO oldlinenum-1
  linedone.i  = FALSE
  oldnames.i  = SUBSTR(oldlines.i,41)
  addedline.i = FALSE
END

none = TRUE
DO i = LogSkipLines TO newlinenum-1

  /* for each line in the new log, see if it is in the old. If it is,
   * see if it changed from the old. If it did, add a line to the change
   * file. Then delete the old line. If there was no old line, then mark
   * the new line as an addition
   */

  newname = SUBSTR(newlines.i,41)
  found = FALSE;
  DO j = LogSkipLines TO oldlinenum-1

    IF linedone.j = FALSE THEN DO

      IF COMPARE(newname,oldnames.j) = 0 THEN DO
        found      = TRUE
        linedone.j = TRUE

        IF (COMPARE(newlines.i,oldlines.j) > 0) THEN DO
          CALL WRITELN(ResultFile,newlines.i);
          none = FALSE
        END
        LEAVE
      END
    END
  END

  IF found = FALSE THEN DO
    addedline.i = TRUE;
  END
END

IF none = TRUE THEN DO
  CALL WRITELN(ResultFile,"    NONE")
END

CALL WRITELN(ResultFile,' ')


/*****************************************************************************/


CALL WRITELN(ResultFile,'New Items Since Previous Build:');

none = TRUE
DO i = LogSkipLines TO newlinenum-1
  IF addedline.i = TRUE THEN DO
    CALL WRITELN(ResultFile,newlines.i);
    none = FALSE
  END
END

IF none = TRUE THEN DO
  CALL WRITELN(ResultFile,"    NONE")
END

CALL WRITELN(ResultFile,' ')


/*****************************************************************************/


CALL WRITELN(ResultFile,'Deleted Items Since Previous Build:');

none = TRUE
DO i = LogSkipLines TO oldlinenum-1
  IF linedone.i = FALSE THEN DO
    CALL WRITELN(ResultFile,oldlines.i);
    none = FALSE
  END
END

IF none = TRUE THEN DO
  CALL WRITELN(ResultFile,"    NONE")
END


/*****************************************************************************/


CALL WRITELN(ResultFile,' ')
CALL WRITELN(ResultFile,'------------------------------------------------------------')
CALL WRITELN(ResultFile,' ')
CALL WRITELN(ResultFile,'Disk Space Usage:')
CALL WRITELN(ResultFile,' ')
CALL WRITELN(ResultFile,'                  Used (DOS\3)  Used (DOS\5)  Avail')
CALL WRITELN(ResultFile,'                  ------------  ------------  -----')

maxDiskName = -1

DO i = LogSkipLines TO newlinenum-2
  endName  = INDEX(newlines.i,':',41)
  diskName = SUBSTR(newlines.i,41,endName-41)
  itemSize = STRIP(SUBSTR(newlines.i,20,9))
  IF itemSize ~= 'Disk' THEN DO

    IF itemSize == 'Drawer' THEN DO
      itemSize      = 1
      itemSize_DCFS = 2

    END; ELSE DO

      itemSize = (itemSize + 511) % 512              /* # of data blocks */
      itemSize = itemSize + ((itemSize-1) % 72) + 1  /* 1 for file header, 1 more for every 72 data blocks */
      itemSize_DCFS = itemSize

    END

    found = FALSE
    DO j = 0 TO maxDiskName
      IF diskName = diskNames.j THEN DO
        found = TRUE
        diskBlocks.j = diskBlocks.j + itemSize
        diskBlocks_DCFS.j = diskBlocks_DCFS.j + itemSize_DCFS
      END
    END

    IF found = FALSE THEN DO
      maxDiskName = maxDiskName + 1
      diskNames.maxDiskName = diskName
      diskBlocks.maxDiskName = 2;
      diskBlocks_DCFS.maxDiskName = 3;
    END
  END
END

DO i = 0 TO maxDiskName
  CALL WRITELN(ResultFile,'   ' SUBSTR(diskNames.i,1,13,' ') '    ' SUBSTR(diskBlocks.i,1,11) ' ~' SUBSTR(diskBlocks_DCFS.i,1,7) ' 1760')
END


/*****************************************************************************/


CALL CLOSE(ResultFile)


/*****************************************************************************/

OPTIONS FAILAT 11
CommandLine = 'inet:c/rsh <NIL: HOST cbmvax CMD /usr/local/lib/news/inews -t "Disk Build 'Version'.'newrev'" -n amiga.release <'PostingName
ADDRESS COMMAND CommandLine
