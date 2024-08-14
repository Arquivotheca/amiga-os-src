#ifndef TEXTTABLE_H
#define TEXTTABLE_H


/****************************************************************************/


/* This file was created automatically by CatComp.
 * Do NOT edit by hand!
 */


#include <exec/types.h>


/****************************************************************************/


#ifdef MORE
#define MSG_MO_CONSPEC 4000
#define MSG_MO_CONSPEC_STR "CON:0000/0000/0640/0200/ More --- Copyright (c) 1986-1992 CBM /close"

#define MSG_MO_USEFIRST 4010
#define MSG_MO_USEFIRST_STR "CLI usage:  More filename  or  More < PIPE:pipename"

#define MSG_MO_USE2 4011
#define MSG_MO_USE2_STR "            (More opens its own window if RUN)"

#define MSG_MO_USE3 4012
#define MSG_MO_USE3_STR "WB  usage:  Click More, Shift/Doubleclick text file"

#define MSG_MO_USELAST 4013
#define MSG_MO_USELAST_STR "While viewing file, press H for help screen"

#define MSG_MO_HELPFIRST 4020
#define MSG_MO_HELPFIRST_STR "Commands:"

#define MSG_MO_HELP1 4021
#define MSG_MO_HELP1_STR "<Space> ........ Next Page (More)"

#define MSG_MO_HELP2 4022
#define MSG_MO_HELP2_STR "<Return> ....... Next Line"

#define MSG_MO_HELP3 4023
#define MSG_MO_HELP3_STR "q or ctrl/c .... Quit"

#define MSG_MO_HELP4 4024
#define MSG_MO_HELP4_STR "h .............. Help"

#define MSG_MO_HELP5 4025
#define MSG_MO_HELP5_STR "/string ........ Search for string (case sensitive) (<- \\)"

#define MSG_MO_HELP6 4026
#define MSG_MO_HELP6_STR ".string ........ Search for string (not case sensitive) (<- ,)"

#define MSG_MO_HELP7 4027
#define MSG_MO_HELP7_STR "n .............. Find next occurence of string (<- p)"

#define MSG_MO_HELPPLAST 4028
#define MSG_MO_HELPPLAST_STR "CTRL/L ......... Refresh window"

#define MSG_MO_HELP9 4029
#define MSG_MO_HELP9_STR "< .............. First Page"

#define MSG_MO_HELP10 4030
#define MSG_MO_HELP10_STR "> .............. Last Page"

#define MSG_MO_HELP11 4031
#define MSG_MO_HELP11_STR "%N ............. Move N% into file"

#define MSG_MO_HELP12 4032
#define MSG_MO_HELP12_STR "b or <BackSpace> Previous Page (Less)"

#define MSG_MO_HELPLAST 4033
#define MSG_MO_HELPLAST_STR "E .............. Edit using editor set in ENV:EDITOR"

#define MSG_MO_NODOS 4040
#define MSG_MO_NODOS_STR "V1.2 required"

#define MSG_MO_NOINTUITION 4041
#define MSG_MO_NOINTUITION_STR "No Intuition"

#define MSG_MO_NOWINDOW 4042
#define MSG_MO_NOWINDOW_STR "Can't open window"

#define MSG_MO_NOPACKET 4043
#define MSG_MO_NOPACKET_STR "Console packet failed"

#define MSG_MO_NOFILE 4044
#define MSG_MO_NOFILE_STR "Can't open file"

#define MSG_MO_NOSTAR 4045
#define MSG_MO_NOSTAR_STR "Can't More *"

#define MSG_MO_BINARY 4046
#define MSG_MO_BINARY_STR "Warning: File may contain binary, q = quit"

#define MSG_MO_NOTFOUND 4047
#define MSG_MO_NOTFOUND_STR "Not Found"

#define MSG_MO_EOF 4048
#define MSG_MO_EOF_STR "End of File"

#define MSG_MO_MORE 4049
#define MSG_MO_MORE_STR "More"

#define MSG_MO_PREVPAGE 4050
#define MSG_MO_PREVPAGE_STR "Finding previous page"

#define MSG_MO_NOFILE2 4051
#define MSG_MO_NOFILE2_STR "Can't reopen file"

#define MSG_MO_PRESSH 4052
#define MSG_MO_PRESSH_STR "Press h for help"

#define MSG_MO_CSEARCH 4053
#define MSG_MO_CSEARCH_STR "Searching: %s"

#define MSG_MO_ISEARCH 4054
#define MSG_MO_ISEARCH_STR "searching: %s"

#define MSG_MO_QUIT 4055
#define MSG_MO_QUIT_STR "CTRL-C or q to exit"

#define MSG_MO_FILEREQ 4056
#define MSG_MO_FILEREQ_STR "Text File to View"

#define MSG_MO_FILEASK 4057
#define MSG_MO_FILEASK_STR "Enter filename, or <Return> to exit: "

#define MSG_MO_NOMEM 4058
#define MSG_MO_NOMEM_STR "Not enough free memory"

#define MSG_MO_EDITOR 4059
#define MSG_MO_EDITOR_STR "ENV:EDITOR"

#endif /* MORE */


/****************************************************************************/


#ifdef STRINGARRAY

struct AppString
{
    LONG   as_ID;
    STRPTR as_Str;
};

struct AppString AppStrings[] =
{
#ifdef MORE
    {MSG_MO_CONSPEC,MSG_MO_CONSPEC_STR},
    {MSG_MO_USEFIRST,MSG_MO_USEFIRST_STR},
    {MSG_MO_USE2,MSG_MO_USE2_STR},
    {MSG_MO_USE3,MSG_MO_USE3_STR},
    {MSG_MO_USELAST,MSG_MO_USELAST_STR},
    {MSG_MO_HELPFIRST,MSG_MO_HELPFIRST_STR},
    {MSG_MO_HELP1,MSG_MO_HELP1_STR},
    {MSG_MO_HELP2,MSG_MO_HELP2_STR},
    {MSG_MO_HELP3,MSG_MO_HELP3_STR},
    {MSG_MO_HELP4,MSG_MO_HELP4_STR},
    {MSG_MO_HELP5,MSG_MO_HELP5_STR},
    {MSG_MO_HELP6,MSG_MO_HELP6_STR},
    {MSG_MO_HELP7,MSG_MO_HELP7_STR},
    {MSG_MO_HELPPLAST,MSG_MO_HELPPLAST_STR},
    {MSG_MO_HELP9,MSG_MO_HELP9_STR},
    {MSG_MO_HELP10,MSG_MO_HELP10_STR},
    {MSG_MO_HELP11,MSG_MO_HELP11_STR},
    {MSG_MO_HELP12,MSG_MO_HELP12_STR},
    {MSG_MO_HELPLAST,MSG_MO_HELPLAST_STR},
    {MSG_MO_NODOS,MSG_MO_NODOS_STR},
    {MSG_MO_NOINTUITION,MSG_MO_NOINTUITION_STR},
    {MSG_MO_NOWINDOW,MSG_MO_NOWINDOW_STR},
    {MSG_MO_NOPACKET,MSG_MO_NOPACKET_STR},
    {MSG_MO_NOFILE,MSG_MO_NOFILE_STR},
    {MSG_MO_NOSTAR,MSG_MO_NOSTAR_STR},
    {MSG_MO_BINARY,MSG_MO_BINARY_STR},
    {MSG_MO_NOTFOUND,MSG_MO_NOTFOUND_STR},
    {MSG_MO_EOF,MSG_MO_EOF_STR},
    {MSG_MO_MORE,MSG_MO_MORE_STR},
    {MSG_MO_PREVPAGE,MSG_MO_PREVPAGE_STR},
    {MSG_MO_NOFILE2,MSG_MO_NOFILE2_STR},
    {MSG_MO_PRESSH,MSG_MO_PRESSH_STR},
    {MSG_MO_CSEARCH,MSG_MO_CSEARCH_STR},
    {MSG_MO_ISEARCH,MSG_MO_ISEARCH_STR},
    {MSG_MO_QUIT,MSG_MO_QUIT_STR},
    {MSG_MO_FILEREQ,MSG_MO_FILEREQ_STR},
    {MSG_MO_FILEASK,MSG_MO_FILEASK_STR},
    {MSG_MO_NOMEM,MSG_MO_NOMEM_STR},
    {MSG_MO_EDITOR,MSG_MO_EDITOR_STR},
#endif /* MORE */
};


#endif /* STRINGARRAY */


/****************************************************************************/


#endif /* TEXTTABLE_H */
