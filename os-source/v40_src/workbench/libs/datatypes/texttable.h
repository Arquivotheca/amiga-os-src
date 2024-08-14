#ifndef TEXTTABLE_H
#define TEXTTABLE_H


/****************************************************************************/


/* This file was created automatically by CatComp.
 * Do NOT edit by hand!
 */


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif


/****************************************************************************/


#define DTERROR_UNKNOWN_DATATYPE 10000
#define DTERROR_UNKNOWN_DATATYPE_STR "Unknown data type for %s"

#define DTERROR_COULDNT_SAVE 10001
#define DTERROR_COULDNT_SAVE_STR "Couldn't save %s"

#define DTERROR_COULDNT_OPEN 10002
#define DTERROR_COULDNT_OPEN_STR "Couldn't open %s"

#define DTERROR_COULDNT_SEND_MESSAGE 10003
#define DTERROR_COULDNT_SEND_MESSAGE_STR "Couldn't send message"

#define DTMSG_TYPE_BINARY 11000
#define DTMSG_TYPE_BINARY_STR "Binary"

#define DTMSG_TYPE_ASCII 11001
#define DTMSG_TYPE_ASCII_STR "ASCII"

#define DTMSG_TYPE_IFF 11002
#define DTMSG_TYPE_IFF_STR "IFF"

#define DTMSG_TYPE_MISC 11003
#define DTMSG_TYPE_MISC_STR "Miscellaneous"

#define MSG_GID_SYSTEM 1937339252
#define MSG_GID_SYSTEM_STR "System"

#define MSG_GID_TEXT 1952807028
#define MSG_GID_TEXT_STR "Text"

#define MSG_GID_DOCUMENT 1685021557
#define MSG_GID_DOCUMENT_STR "Document"

#define MSG_GID_SOUND 1936684398
#define MSG_GID_SOUND_STR "Sound"

#define MSG_GID_INSTRUMENT 1768846196
#define MSG_GID_INSTRUMENT_STR "Instrument"

#define MSG_GID_MUSIC 1836413801
#define MSG_GID_MUSIC_STR "Music"

#define MSG_GID_PICTURE 1885954932
#define MSG_GID_PICTURE_STR "Picture"

#define MSG_GID_ANIMATION 1634625901
#define MSG_GID_ANIMATION_STR "Animation"

#define MSG_GID_MOVIE 1836021353
#define MSG_GID_MOVIE_STR "Movie"


/****************************************************************************/


#ifdef STRINGARRAY

struct AppString
{
    LONG   as_ID;
    STRPTR as_Str;
};

struct AppString AppStrings[] =
{
    {DTERROR_UNKNOWN_DATATYPE,(STRPTR)DTERROR_UNKNOWN_DATATYPE_STR},
    {DTERROR_COULDNT_SAVE,(STRPTR)DTERROR_COULDNT_SAVE_STR},
    {DTERROR_COULDNT_OPEN,(STRPTR)DTERROR_COULDNT_OPEN_STR},
    {DTERROR_COULDNT_SEND_MESSAGE,(STRPTR)DTERROR_COULDNT_SEND_MESSAGE_STR},
    {DTMSG_TYPE_BINARY,(STRPTR)DTMSG_TYPE_BINARY_STR},
    {DTMSG_TYPE_ASCII,(STRPTR)DTMSG_TYPE_ASCII_STR},
    {DTMSG_TYPE_IFF,(STRPTR)DTMSG_TYPE_IFF_STR},
    {DTMSG_TYPE_MISC,(STRPTR)DTMSG_TYPE_MISC_STR},
    {MSG_GID_SYSTEM,(STRPTR)MSG_GID_SYSTEM_STR},
    {MSG_GID_TEXT,(STRPTR)MSG_GID_TEXT_STR},
    {MSG_GID_DOCUMENT,(STRPTR)MSG_GID_DOCUMENT_STR},
    {MSG_GID_SOUND,(STRPTR)MSG_GID_SOUND_STR},
    {MSG_GID_INSTRUMENT,(STRPTR)MSG_GID_INSTRUMENT_STR},
    {MSG_GID_MUSIC,(STRPTR)MSG_GID_MUSIC_STR},
    {MSG_GID_PICTURE,(STRPTR)MSG_GID_PICTURE_STR},
    {MSG_GID_ANIMATION,(STRPTR)MSG_GID_ANIMATION_STR},
    {MSG_GID_MOVIE,(STRPTR)MSG_GID_MOVIE_STR},
};


#endif /* STRINGARRAY */


/****************************************************************************/


#endif /* TEXTTABLE_H */
