/*******************

    SaveAs8SVX.c

      932805

********************/


#include <exec/types.h>
#include <dos/dos.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include <string.h>	// for setmem()

#include "cdxl/pan.h"
#include "cdxl/cdxlob.h"
#include "cdxl/runcdxl.h"
#include "cdxl/blitdef.h"
#include "cdxl/runcdxl_protos.h"
#include "cdxl/xledit.h"


#define MakeID(a,b,c,d) ((LONG)(a)<<24L | (LONG)(b)<<16L | (c)<<8 | (d))

#define ID_FORM MakeID('F','O','R','M')
#define ID_VHDR MakeID('V','H','D','R')
#define ID_BODY MakeID('B','O','D','Y')
#define ID_8SVX MakeID('8','S','V','X')
#define ID_CHAN MakeID('C','H','A','N')

#define	MAXLEN_FILENAME	30
#define	MAXLEN_PATHNAME	255

IMPORT UBYTE	FileNameBuf[256];


/* General IFF Structures */
typedef struct
{
	LONG ckID;
	LONG ckSize;
} Chunk_Header;

typedef struct
{
	LONG ckID;
	LONG ckSize;
	LONG formtype;
} FORM_Header;

typedef struct VOICE_HEADER
{
    ULONG oneshot;
	ULONG repeat;
	ULONG samples_per_cycle;
	UWORD samples_per_sec;
	UBYTE octives;
	UBYTE compression;
	LONG  volume;
} VOICE_HEADER;


STATIC	VOICE_HEADER	VHead;
STATIC	FORM_Header	FHead;
STATIC	Chunk_Header	CHead;


/***********************************************************************/
/* This returns a pointer to the filename at the end of the path or NULL
** if there is no filename at the end.
*/

UBYTE *
RootName( UBYTE * path )
{
    UBYTE	*root;

    if ( !path )
	return ( NULL );

    if ( root = strrchr( path, '/' ) ) {
	root++;
    } else if ( root = strrchr( path, ':' ) ) {
	root++;
    } else {
	root = path;
    }

    if ( !(*root) )
	root = NULL;

    return ( root );

}  // RootName()


/***********************************************************************/
/* This appends the filename extension 'ext' to 'name', only if
** it doesn't already exist.
** 'name' is expected to be FILENAME_LENGTH bytes long.
** Returns FALSE if not able to append within size constraints.
** Also returns FALSE if filename will be too long for AmigaDOS.
** 'ext' should not contain a period.
** 'ext' must NOT contain wildcard characters.
*/

BOOL
AppendExtension( UBYTE * name, UBYTE * ext )
{
    UBYTE	* cp;
    UBYTE	* rootname;

    if ( !name || !ext || !(*ext) )
	return ( TRUE );

    if ( *ext == '.' ) // skip over the period, if present
	ext++;

    if ( cp = strrchr( name, '.' ) )  {
	if ( !stricmp( cp+1, ext ) )  {
	    return ( TRUE );
	}
    }

    if ( rootname = RootName(name) )  {
	if ( strlen(rootname) + strlen(ext) + 1 > MAXLEN_FILENAME )
	    return ( FALSE );
    }

    if ( strlen(name) + strlen(ext) + 1 > MAXLEN_PATHNAME )
	return ( FALSE );

    strcat( name, ".");
    strcat( name, ext );
    return ( TRUE );

} // AppendExtension()


WritePad( BPTR fh )
{
    short nil = 0;
    return ( (Write(fh,&nil,1) == 1) ? 1 : 0 );

} // WritePad()


SaveAs8svx( UBYTE * body, LONG size , UBYTE * name )
{
    BPTR	  fh = NULL;
    LONG	  chan = 4,ret = RC_OK;
    UBYTE	  file[256];
    BOOL	  Pad = FALSE,FPad = FALSE;


    strcpy( file, name );
//    RemoveExtension( file, "*?" );

    kprintf("SaveAs8SVX ENTERED with body= 0x%lx, size= %ld, name= '%ls'\n",
	body,size,name);

    if( !AppendExtension( file, "8svx" ) ) {
	ret = RC_FNAME_LONG;
	goto error;
    }

    if( !(fh = Open( file, MODE_NEWFILE )) ) {
	ret = RC_CANT_OPEN;
	printf("Could NOT Open '%ls'\n",file);
	goto error;
    }

    kprintf("SaveAs8SVX 1\n");

		// Body   VoiceHeader	  Chan
    FHead.ckSize = size + sizeof(VHead) + sizeof(LONG) + (3 * 8) + 4;
    if( FHead.ckSize & 1 ) {
	FHead.ckSize++;
	FPad = TRUE;
    }

    if( Write( fh, &FHead, sizeof(FHead) ) != sizeof(FHead) ) {
	ret = RC_WRITE_ERROR;
	goto error;
    }

    kprintf("SaveAs8SVX 2\n");

    VHead.repeat = size;

    CHead.ckID = ID_VHDR;
    CHead.ckSize = sizeof(VHead);
    if( CHead.ckSize & 1 ) {
	CHead.ckSize++;
	Pad = TRUE;
    }

    if( Write( fh, &CHead, sizeof(CHead) ) != sizeof(CHead) ) {
	ret = RC_WRITE_ERROR;
	goto error;
    }

    kprintf("SaveAs8SVX 3\n");

    if( Write( fh, &VHead, sizeof(VHead) ) != sizeof(VHead) ) {
	ret = RC_WRITE_ERROR;
	goto error;
    }

    kprintf("SaveAs8SVX 4\n");

    if( Pad ) {
	if (!WritePad(fh) ) {
	    ret = RC_WRITE_ERROR;
	    goto error;
	}
	Pad = FALSE;
    }

    kprintf("SaveAs8SVX 5\n");

    CHead.ckID = ID_CHAN;
    CHead.ckSize = sizeof(LONG);
    if( CHead.ckSize & 1 ) {
	CHead.ckSize++;
	Pad = TRUE;
    }

    if( Write( fh, &CHead, sizeof(CHead) ) != sizeof(CHead) ) {
	ret = RC_WRITE_ERROR;
	goto error;
    }

    kprintf("SaveAs8SVX 6\n");

    if( Write( fh, &chan, sizeof(LONG) ) != sizeof(LONG) ) {
	ret = RC_WRITE_ERROR;
	goto error;
    }

    kprintf("SaveAs8SVX 7\n");

    if( Pad ) {
	if (!WritePad(fh) ) {
	    ret = RC_WRITE_ERROR;
	    goto error;
	}
	Pad = FALSE;
    }

    kprintf("SaveAs8SVX 8\n");

    CHead.ckID = ID_BODY;
    CHead.ckSize = size;
    if( CHead.ckSize & 1 ) {
	CHead.ckSize++;
	Pad = TRUE;
    }

    kprintf("SaveAs8SVX 9\n");

    if( Write( fh, &CHead, sizeof(CHead) ) != sizeof(CHead) ) {
	ret = RC_WRITE_ERROR;
	goto error;
    }

    kprintf("SaveAs8SVX A\n");

    if( Write( fh, body, size ) != size ) {
	ret = RC_WRITE_ERROR;
	goto error;
    }

    kprintf("SaveAs8SVX B\n");

    if( Pad ) {
	if (!WritePad(fh) ) {
	    ret = RC_WRITE_ERROR;
	    goto error;
	}
	Pad = FALSE;
    }

    kprintf("SaveAs8SVX C\n");

    if( FPad ) {
	if (!WritePad(fh) ) {
	    ret = RC_WRITE_ERROR;
	    goto error;
	}
	Pad = FALSE;
    }

    kprintf("SaveAs8SVX D\n");

    Close( fh );

    printf("%s  to  %s\n",name,file);

    return( ret );

error:

    if( fh )
	Close( fh );

    return( ret );

} /* SaveAs8svx() */


SaveXLAs8SVX( XLEDIT * xledit, struct Window * win )
{
    int		  ret = RC_OK;


    if ( !(xledit && xledit->cdxlob && xledit->cdxlob->AudioSize) ) {
	DisplayBeep( NULL );
	return( RC_FAILED );
    }

    kprintf("SaveXLAs8SVX() ENTERED\n");

    if ( FileReq( win ) ) {

	setmem( &VHead, sizeof(VHead), 0 );
//	VHead.samples_per_sec = 14435;
	VHead.samples_per_sec = INTDIV( (( xledit->cdxlob->ReadXLSpeed * DEFAULT_SECTOR_SIZE) * xledit->cdxlob->AudioSize ), xledit->cdxlob->FrameSize );

	VHead.octives = 1;
	VHead.volume = 65536;

	setmem( &FHead, sizeof(FHead), 0 );
	setmem( &CHead, sizeof(CHead), 0 );

	FHead.ckID = ID_FORM;
	FHead.formtype = ID_8SVX;

	kprintf("SaveXLAs8SVX() 1\n");

	ret = SaveAs8svx( xledit->cdxlob->audio[xledit->cdxlob->curVideo], 
	    xledit->cdxlob->AudioSize, FileNameBuf );
    }

    kprintf("SaveXLAs8SVX() END ret= %ld\n",ret);

    return( ret );

} // SaveXLAs8SVX()

