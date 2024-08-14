#ifndef LISTREQ_H
#define LISTREQ_H

/* ======================================================================== *
     Prototypes and structures for listreq.lib (scrolling list requester)
                             Written by Joe Pearce
 * ======================================================================== *
                          $VER: listreq.h 0.1 (19.2.93)
 * ======================================================================== */

#include "std_headers.h"

#ifndef LIBRARIES_ASL_H
#include <libraries/asl.h>
#endif

#define LISTREQ_Window		   		ASLFR_Window
#define LISTREQ_Screen	       		ASLFR_Screen
#define LISTREQ_PubScreenName  		ASLFR_PubScreenName
#define LISTREQ_PrivateIDCMP   		ASLFR_PrivateIDCMP
#define LISTREQ_IntuiMsgFunc   		ASLFR_IntuiMsgFunc
#define LISTREQ_SleepWindow    		ASLFR_SleepWindow
#define LISTREQ_UserData	   		ASLFR_UserData
#define LISTREQ_TextAttr	   		ASLFR_TextAttr
#define LISTREQ_Locale	       		ASLFR_Locale
#define LISTREQ_TitleText      		ASLFR_TitleText
#define LISTREQ_PositiveText   		ASLFR_PositiveText
#define LISTREQ_NegativeText   		ASLFR_NegativeText
#define LISTREQ_InitialLeftEdge		ASLFR_InitialLeftEdge
#define LISTREQ_InitialTopEdge 		ASLFR_InitialTopEdge
#define LISTREQ_InitialWidth  		ASLFR_InitialWidth
#define LISTREQ_InitialHeight  		ASLFR_InitialHeight
#define LISTREQ_Labels		   		ASLFR_InitialFile
#define LISTREQ_Selected	     	ASLFR_Flags2
#define LISTREQ_Buffer				ASLFR_InitialDrawer
#define LISTREQ_BufferSize			ASLFR_DoSaveMode
#define LISTREQ_HookFunc	     	ASLFR_HookFunc

LONG ListRequestA(void *req, struct TagItem *ti);
LONG ListRequest(void *req, Tag tag, ... );

void *AllocListRequestA(struct TagItem *ti);
void *AllocListRequest( Tag tag, ... );
void FreeListRequest(void *req);

#endif
