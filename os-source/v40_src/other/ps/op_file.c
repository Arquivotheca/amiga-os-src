/************************************************************************

  Module :  Postscript "File Operators"		© Commodore-Amiga
            (started March 1991)
			Display Postscript extensions by L. Vanhelsuwe (May '91)

  Purpose:  This file contains C-entries for operators called directly
            by the interpreter using the Postscript execution model.

  Conventions: -The order in which functions appear is identical to the one
                in the Adobe Red Book (Chapter 6 Operator Summary).
               -Variables called 'tos' and 'nos' point to the Top Of Stack
                and Next Of Stack elements resp. (on Operand stack).

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "errors.h"
#include "exec/types.h"
#include "pstypes.h"
#include "stream.h"
#include "memory.h"
#include "objects.h"

#include "misc.h"
#include "stack.h"
#include "gstate.h"
#include "context.h"
#include "scanner.h"

#include <proto/dos.h>
#include <proto/exec.h>

#define VALID_HEX(c)	(((c)>='0'&&(c)<='9')||((c)>='a'&&(c)<='f')||((c)>='A'&&(c)<='F'))

#define StdOUT	(ctxt->space.StdOut->obj.streamval)
#define StdIN	(ctxt->space.StdIn->obj.streamval)

//--------------------------------------------------------------------
// Standard Postscript operators
//------------------------------
error ps_currentfile	(DPSContext ctxt);
error ps_file			(DPSContext ctxt);
error ps_token			(DPSContext ctxt);
error ps_closefile		(DPSContext ctxt);
error ps_print			(DPSContext ctxt);
error ps_flush			(DPSContext ctxt);
error ps_read			(DPSContext ctxt);
error ps_write			(DPSContext ctxt);
error ps_run			(DPSContext ctxt);
error ps_status			(DPSContext ctxt);
error ps_bytesavailable (DPSContext ctxt);
error ps_readhexstring	(DPSContext ctxt);
error ps_writehexstring	(DPSContext ctxt);
error ps_readstring		(DPSContext ctxt);
error ps_writestring	(DPSContext ctxt);
error ps_readline		(DPSContext ctxt);
error ps_flushfile		(DPSContext ctxt);
error ps_resetfile		(DPSContext ctxt);
error ps_echo			(DPSContext ctxt);

// Display Postscript operators
//-----------------------------
error ps_deletefile		(DPSContext ctxt);
error ps_renamefile		(DPSContext ctxt);
error ps_filenameforall	(DPSContext ctxt);
error ps_setfileposition(DPSContext ctxt);
error ps_fileposition	(DPSContext ctxt);

//--------------------------------------------------------------------

extern int			   Scan	(DPSContext,stream *,int);

//--------------------------------------------------------------------
// Prototypes for other functions..


int CompString(int,char *,char *);
error GetStatement(DPSContext);
error dummy(DPSContext);

error SeekAndDestroy(DPSContext ,stream *);
error AddFile(DPSContext ,ps_obj *);

/************************************************************************/
/************************ File  Operators *******************************/
/************************************************************************/

error ps_currentfile(DPSContext ctxt) {	//		"currentfile" | fileobj
	ps_obj *s_sp;

	s_sp = EXECSP(ctxt);

	// find first File Object on the execution stack

	while(OBJ_TYPE(s_sp) != TYPE_FILE) s_sp++;
	PUSHOPER(ctxt,s_sp);
	return ERR_OK;
}
/************************************************************************/
error ps_file(DPSContext ctxt) {	//	str_fname str_mode "file" | fileobj

	ps_obj *filename,*mode,file_object;
	BPTR f_handle;
	stream *s;
	uchar name[256];
	int i;

	NEED_ARGS(2);

	mode = OPERSP(ctxt);
	filename = mode + 1;
	
	if((s = (stream *)AllocVM(VM,sizeof(stream)))==NULL) return ERR_VMerror;

	file_object.type = (ATTR_LITERAL|TYPE_FILE);
	file_object.len  = 0;
	SAVE_LEVEL(&file_object);

	// test for %stdin ,%stdout and %statementeedit first

	if((CompString(filename->len,filename->obj.stringval,"%stdin"))) {
		if(mode->obj.stringval[0] == 'r' && mode->len == 1) {
			if((f_handle = Open("*",MODE_OLDFILE))==NULL) {
				return ERR_ioerror;
			}
			if((MakeFileStream(s,f_handle,S_READABLE))==FALSE) {
				Close(f_handle);
				return ERR_ioerror;
			}	
			file_object.obj.streamval = s;
		} else return ERR_invalidfileaccess;

	} else if((CompString(filename->len,filename->obj.stringval,"%stdout"))) {
		if(mode->obj.stringval[0] == 'w' && mode->len == 1) {
			if((f_handle = Open("*",MODE_OLDFILE))==NULL) {
				return ERR_ioerror;
			}
			if((MakeFileStream(s,f_handle,S_WRITEABLE))==FALSE) {
				Close(f_handle);
				return ERR_ioerror;
			}	
			file_object.obj.streamval = s;
		} else return ERR_invalidfileaccess;

	} else if((CompString(filename->len,filename->obj.stringval,"%statementedit"))) {
		if(mode->obj.stringval[0] == 'r' && mode->len ==1) {
			return GetStatement(ctxt);
		} else return ERR_ioerror;
	} else {

		// It's not a standard file so open it up here

		for(i=0;i<filename->len;i++) name[i] = filename->obj.stringval[i];
		name[i] = '\0';
		if(mode->obj.stringval[0] == 'r' && mode->len == 1) {
			if((f_handle = Open(name,MODE_OLDFILE))==NULL) {
				return ERR_ioerror;
			}
			if((MakeFileStream(s,f_handle,S_READABLE))==FALSE) {
				Close(f_handle);
				return ERR_VMerror;
			}
			file_object.obj.streamval = s;

		} else if((mode->obj.stringval[0] =='w') && (mode->len == 1)) {
			if((f_handle = Open(name,MODE_NEWFILE))==NULL) {
				return ERR_ioerror;
			}
			if((MakeFileStream(s,f_handle,S_WRITEABLE))==FALSE) {
				Close(f_handle);
				return ERR_VMerror;
			}
			file_object.obj.streamval = s;
		} else return ERR_invalidfileaccess;

	} 

	POPOPER(ctxt); POPOPER(ctxt);
	PUSHOPER(ctxt,(&file_object));
	AddFile(ctxt,&file_object);
	return ERR_OK;
}
/************************************************************************/
error ps_token(DPSContext ctxt) {	//	file.str "token" | any true | false
	
	ps_obj *obj;
	int scan_result,i;
	stream file_stream;
	NEED_ARGS(1);

	obj = OPERSP(ctxt);

	// If it's a file, give the scanner the stream

	if(OBJ_TYPE(obj) == TYPE_FILE) {
		scan_result = Scan(ctxt,obj->obj.streamval,0);

		if(scan_result == CONTINUE) {
			*obj = *(obj-1);
			POPOPER(ctxt);
			PUSHTRUE;
		} else if(scan_result == COMPLETED) {

			// I close the stream when the scanner receives EOF, but 
			// I don't DeAlloc it

			sclose(obj->obj.streamval);
			POPOPER(ctxt);
			PUSHFALSE;
		} else {
			sclose(obj->obj.streamval);
			POPOPER(ctxt);
			PUSHFALSE;
			return ERR_syntaxerror;
		}	
	} else if(OBJ_TYPE(obj) == TYPE_STRING) {

		// With string's, I create a temporary file stream on this string
		// give it to the scanner and then close it regardless of the scanner
		// result.

		if(!obj->len) {
			POPOPER(ctxt); PUSHFALSE;
			return(ERR_OK);
		} else if(!MakeStringStream(&file_stream,obj->obj.stringval,obj->len,S_READABLE)) {
			return ERR_ioerror;
		}
		scan_result = Scan(ctxt,&file_stream,0);
		if(scan_result == CONTINUE) {
			/* THIS POKES DIRECTLY INTO THE STREAM STRUCT, BY-PASSING STEVES FUNCTIONS */
			obj->len = file_stream.remain;
			if(obj->len == 0) {
				obj->obj.stringval = NULL;
			} else if((obj->obj.stringval = (uchar *)AllocVM(VM,file_stream.remain))==NULL) {
				sclose(&file_stream);
				return ERR_VMerror;
			}
			for(i=0;i<file_stream.remain;i++) 
				obj->obj.stringval[i] = file_stream.current[i] ;
			PUSHTRUE;
		} else if(scan_result == COMPLETED) {
			POPOPER(ctxt); PUSHFALSE;
		}
		sclose(&file_stream);
	} else {
		return ERR_typecheck;
	}
	return ERR_OK;
}
/************************************************************************/
error ps_print(DPSContext ctxt) {	// string "print" | str -> StdOUT
	ps_obj *tos;
	
	NEED_ARGS(1);
	tos = OPERSP(ctxt);

	if(OBJ_TYPE(tos)!=TYPE_STRING) return ERR_typecheck;
	sputs(StdOUT, tos->obj.stringval,tos->len);
	sflush(StdOUT);
	POPOPER(ctxt);

	return ERR_OK;
}
/************************************************************************/
error ps_flush(DPSContext ctxt) {	// "flush"
	
	sflush(StdOUT);
	return ERR_OK;
}
/************************************************************************/
error ps_read(DPSContext ctxt) {	// file "read" | byte TRUE | FALSE
	ps_obj *tos,val;
	int value;

	NEED_ARGS(1);
	tos = OPERSP(ctxt);

	if(OBJ_TYPE(tos) != TYPE_FILE) return ERR_typecheck;
	value = sgetc(tos->obj.streamval);
	if(value == EOF) {
		SeekAndDestroy(ctxt,tos->obj.streamval);
//		sclose(tos->obj.streamval);
		POPOPER(ctxt);
		PUSHFALSE;
	} else {
		val.type = (TYPE_INT|ATTR_LITERAL);
		val.len = 0;
		val.obj.intval = value;
		val.tag = 0;
		POPOPER(ctxt); 
		ENOUGH_ROOM(2);
		PUSHOPER(ctxt,(&val));
		PUSHTRUE;
	}
	return ERR_OK;
}
/************************************************************************/
error ps_write(DPSContext ctxt) {	// file int "write" | ...
	ps_obj *file,*value;

	NEED_ARGS(2);
	value = OPERSP(ctxt);    // TOP OF STACK
	file  = value + 1;       // TOP OF STACK + 1
	if(OBJ_TYPE(file)==TYPE_FILE && OBJ_TYPE(value)==TYPE_INT) {
		if((sputc(file->obj.streamval,(value->obj.intval%256)))==EOF) {
			return ERR_ioerror;
		}
		POPOPER(ctxt); POPOPER(ctxt);
	} else return ERR_typecheck;

	return ERR_OK;
}
/************************************************************************/
error ps_closefile(DPSContext ctxt) {	//	file "closefile" | ...
	ps_obj *tos;
	
	NEED_ARGS(1);
	tos = OPERSP(ctxt);
	if(OBJ_TYPE(tos)!=TYPE_FILE) {
		return ERR_typecheck;
	}
	SeekAndDestroy(ctxt,tos->obj.streamval);
//	sclose(tos->obj.streamval);
	POPOPER(ctxt);
	return ERR_OK;
}
/************************************************************************/
error ps_run(DPSContext ctxt) {		//		str_fname "run" | ...
	ps_obj *tos,exec_obj,null;
	uchar name[256];
	int i;	

	NEED_ARGS(1);
	tos = OPERSP(ctxt);
	if(OBJ_TYPE(tos)!=TYPE_STRING) return ERR_typecheck;

	if((exec_obj.obj.streamval = (stream *)AllocVM(VM,sizeof(stream)))==NULL) {
		return ERR_VMerror;
	}

	for(i=0;i<tos->len;i++) name[i] = tos->obj.stringval[i];
	name[i] = '\0';
	exec_obj.type = (ATTR_EXECUTE|TYPE_FILE);
	exec_obj.len = 0;
	SAVE_LEVEL(&exec_obj);
	if((OpenFileStream(exec_obj.obj.streamval,name,S_READABLE))==FALSE) {
		FreeVM(VM,(void *)exec_obj.obj.streamval,sizeof(stream));
		return ERR_ioerror;
	}

	// Stick the new file object on the execution stack and let the interp do 
	// the rest.
	
	null.type = TYPE_OPERATOR|ATTR_EXECUTE;
	null.len  = RUN_MARKER;
	null.tag = 0;
	null.obj.operval = dummy;		// internal operator without side-effects

	POPOPER(ctxt);
	ENOUGH_EROOM(2);
	PUSHEXEC(ctxt,&null);			// Push run context marker for "exit"
	PUSHEXEC(ctxt,(&exec_obj));

	AddFile(ctxt,&exec_obj);

	return ERR_OK;
}
/************************************************************************/
error ps_status(DPSContext ctxt) {		// file "status" | bool
	ps_obj *tos;
	
	NEED_ARGS(1);
	tos = OPERSP(ctxt);
	if(OBJ_TYPE(tos)!=TYPE_FILE) return ERR_typecheck;

	/* THIS PEEKS DIRECTLY AT THE STREAM STRUCTURE */
	if(tos->obj.streamval->flags&S_OPEN) {
		POPOPER(ctxt);
		PUSHTRUE;
	} else {
		POPOPER(ctxt);
		PUSHFALSE;
	}
	return ERR_OK;
}
/************************************************************************/
error ps_bytesavailable(DPSContext ctxt) {	//
	ps_obj *tos,value;
	int current,end;

	NEED_ARGS(1);
	tos = OPERSP(ctxt);
	if(OBJ_TYPE(tos)!=TYPE_FILE) return ERR_typecheck;

	value.type = TYPE_INT|ATTR_LITERAL;
	value.len = 0;
	SAVE_LEVEL(&value);
	if((current = sseek(tos->obj.streamval,INT_MAX))==EOF) return ERR_ioerror;

	end = sseek(tos->obj.streamval,0);	// end is the true size of file
	sseek(tos->obj.streamval,current);	// restore stream pointer
	value.obj.intval = end;

	POPOPER(ctxt); PUSHOPER(ctxt,(&value));
	return ERR_OK;
}
/************************************************************************/
error ps_readhexstring(DPSContext ctxt) {	//
	ps_obj *file,*string,result;
	int len=0,ch,hex_value=0,nibble,digit_count=0;

	NEED_ARGS(2);
	string = OPERSP(ctxt); file = string + 1;
	if(OBJ_TYPE(string)!=TYPE_STRING||OBJ_TYPE(file)!=TYPE_FILE) return ERR_typecheck;

	while((ch = sgetc(file->obj.streamval))!=EOF) {
		if(VALID_HEX(ch)) {
			hex_value <<=4;
			nibble = ch-'0';
			if(nibble>9) nibble -= 'A';
			if(nibble>15) nibble -= 'a';
			hex_value |= nibble;
			digit_count++;

			// only when have read two nibbles do we stick it in the string
			if(digit_count==2) {
				string->obj.stringval[len++] = (uchar)hex_value;
				hex_value = 0;
				digit_count = 0;
				if(len>string->len) {
					len--;
					break;
				}
			}
		}
	}
	if(ch==EOF) {
//		sclose(file->obj.streamval);
		SeekAndDestroy(ctxt,file->obj.streamval);
	}
	result.type = (TYPE_STRING|ATTR_LITERAL);
	result.len = len;
	SAVE_LEVEL(&result);
	result.obj.stringval = string->obj.stringval;
	POPOPER(ctxt); POPOPER(ctxt);
	PUSHOPER(ctxt,&result);
	if(len!=string->len) {
		PUSHFALSE;
	} else {
		PUSHTRUE;
	}
	return ERR_OK;
}
/************************************************************************/
error ps_writehexstring(DPSContext ctxt) {
	ps_obj *file,*string;
	int ch,loop,nibble;
	

	NEED_ARGS(2);
	string = OPERSP(ctxt); file = string + 1;
	if(OBJ_TYPE(string)!=TYPE_STRING||OBJ_TYPE(file)!=TYPE_FILE) return ERR_typecheck;

	for(loop=0;loop<string->len;loop++) {
		ch = string->obj.stringval[loop];
		nibble = (ch&0xf0)>>4;
		if(nibble>9) nibble +=('a'-10); else nibble += '0';
		if((sputc(file->obj.streamval,nibble))==EOF) return ERR_ioerror;
		nibble = (ch&0x0f);
		if(nibble>9) nibble +=('a'-10); else nibble += '0';
		if((sputc(file->obj.streamval,nibble))==EOF) return ERR_ioerror;
	}
	sflush(file->obj.streamval);
	POPOPER(ctxt); POPOPER(ctxt);
	return ERR_OK;
}
/************************************************************************/
error ps_readstring(DPSContext ctxt) {
	ps_obj *file,*string,result;
	int loop,ch;

	NEED_ARGS(2);
	string = OPERSP(ctxt); file = string + 1;
	if(OBJ_TYPE(string)!=TYPE_STRING||OBJ_TYPE(file)!=TYPE_FILE) return ERR_typecheck;
	
	result.obj.stringval = string->obj.stringval;
	for(loop=0;loop<string->len;loop++) {
		if((ch = sgetc(file->obj.streamval))==EOF) { 
//			sclose(file->obj.streamval);
			SeekAndDestroy(ctxt,file->obj.streamval);
			break;
		}
		result.obj.stringval[loop] = (uchar)ch;
	}
	result.type = (ATTR_LITERAL|TYPE_STRING);
	SAVE_LEVEL(&result);
	POPNUMOPER(ctxt,2);
	result.len = loop;
	PUSHOPER(ctxt,&result);
	if(loop!=string->len) { PUSHFALSE; }
	else { PUSHTRUE; }

	return ERR_OK;
}
/*************************************************************************/
error ps_writestring(DPSContext ctxt) {
	ps_obj *string,*file;
	NEED_ARGS(2);
	string = OPERSP(ctxt); file = string + 1;
	if(OBJ_TYPE(string)!=TYPE_STRING||OBJ_TYPE(file)!=TYPE_FILE) return ERR_typecheck; 

	if((sputs(file->obj.streamval,string->obj.stringval,string->len))==EOF) return ERR_ioerror;
	
	POPNUMOPER(ctxt,2);
	return ERR_OK;
}
/*************************************************************************/
error ps_readline(DPSContext ctxt) {
	ps_obj *string,*file,result;
	int loop,ch;
	uchar *p;

	NEED_ARGS(2);
	string = OPERSP(ctxt); file = string + 1;
	if(OBJ_TYPE(string)!=TYPE_STRING||OBJ_TYPE(file)!=TYPE_FILE) return ERR_typecheck; 
	result.obj.stringval = file->obj.stringval;
	loop = string->len;

	p = string->obj.stringval;

	while((ch=sgetc(file->obj.streamval))!='\n') {
		if(ch==EOF) {
//			sclose(file->obj.streamval);
			SeekAndDestroy(ctxt,file->obj.streamval);
			break;
		}
		if(!loop) return ERR_rangecheck;
		*p = ch; loop--;p++;
		
	}

	result.len = string->len-loop;
	result.type = TYPE_STRING|ATTR_LITERAL;
	result.obj.stringval = string->obj.stringval;
	SAVE_LEVEL(&result);
	POPNUMOPER(ctxt,2);
	PUSHOPER(ctxt,&result);
	if(ch==EOF) {
		PUSHFALSE;
	} else {
		PUSHTRUE;
	}
	return ERR_OK;
}
/*************************************************************************/
error ps_flushfile(DPSContext ctxt) {
	ps_obj *file;
	NEED_ARGS(2);
	
	file = OPERSP(ctxt);
	if(OBJ_TYPE(file)!=TYPE_FILE) return ERR_typecheck;

	if((sflush(file->obj.streamval))==FALSE) return ERR_ioerror;
	
	POPOPER(ctxt);
	return ERR_OK;
}
/*************************************************************************/
error ps_resetfile(DPSContext ctxt) {
	ps_obj *file;

	/* A Dangerous function ! */

	NEED_ARGS(1);
	file = OPERSP(ctxt);
	if(OBJ_TYPE(file)!=TYPE_FILE) return ERR_typecheck;


	/* Currently, this is a NOP */

	POPOPER(ctxt);
	return ERR_OK;
}
/*************************************************************************/
error ps_echo(DPSContext ctxt) {
	ps_obj *bool;
	NEED_ARGS(1);
	bool = OPERSP(ctxt);
	if(OBJ_TYPE(bool)!= TYPE_BOOL) return ERR_typecheck;
	if(bool->obj.boolval == TRUE) {
		ctxt->space.Echo = TRUE;
	} else {
		ctxt->space.Echo = FALSE;
	}
	POPOPER(ctxt);
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
/************ Display Postscript File Operator Extensions ***************/
/************************************************************************/
error ps_deletefile(DPSContext ctxt) { // fname_str "deletefile" | ...

	char str_buff[256];
	pso *tos;
	int i;

	NEED_ARGS(1);

	tos = OPERSP(ctxt);

	if (OBJ_TYPE(tos)!=TYPE_STRING) return ERR_typecheck;

	i = tos->len;
	while(i--) {
		str_buff[i]=tos->obj.stringval[i];
	}
	str_buff[tos->len] = '\0';		// NULL-terminate string

	DeleteFile(str_buff);	

	return ERR_OK;				
}
/*************************************************************************/
error ps_renamefile(DPSContext ctxt) { // old_fname new_fname "renamefile" | ...

	char oldname[256],newname[256];
	pso *tos,*nos;
	int i;

	NEED_ARGS(2);

	tos = OPERSP(ctxt);
	nos = tos +1;

	if (OBJ_TYPE(tos)!=TYPE_STRING || OBJ_TYPE(nos)!=TYPE_STRING)
		return ERR_typecheck;

	i = tos->len;
	while(i--) {
		newname[i]=tos->obj.stringval[i];
	}
	newname[tos->len] = '\0';		// NULL-terminate string

	i = nos->len;
	while(i--) {
		oldname[i]=nos->obj.stringval[i];
	}
	newname[nos->len] = '\0';		// NULL-terminate string

	Rename(oldname,newname);	

	return ERR_OK;
}
/*************************************************************************/
error ps_filenameforall(DPSContext ctxt) { // pattern proc scratch "" |
	return ERR_NOT_IMPLEMENTED;
}
/*************************************************************************/
error ps_setfileposition(DPSContext ctxt) { // 
	return ERR_NOT_IMPLEMENTED;
}
/*************************************************************************/
error ps_fileposition(DPSContext ctxt) { // 
	return ERR_NOT_IMPLEMENTED;
}
/*************************************************************************/
/*************************************************************************/
/************************ Misc Functions ********************************/
/************************************************************************/

int CompString(int len,char *buf1,char *buf2) {
	for(;len>0;len--) if((*buf1++-*buf2++)!=0) return NULL;
	return 1;
}

#define STATEMENT_BUF	1024

error GetStatement(DPSContext ctxt) {
	stream file,*f,*s;
	uchar *buf1,*exchange;
	int hex=0,curly=0,string=0,quit=FALSE,count=0,loop,last_ch,ch;
	ps_obj result;
	

	f = &file;
	if((s = (stream *)AllocVM(VM,sizeof(stream)))==NULL) {
		FreeVM(VM,(void *)f,sizeof(stream));
		return ERR_VMerror;
	}

	if((OpenFileStream(f,"*",S_READABLE))==FALSE) return ERR_ioerror;
	if((buf1 = (uchar *)AllocVM(VM,STATEMENT_BUF))==NULL) {
		sclose(f);
		FreeVM(VM,(void *)s,sizeof(stream));
		return ERR_VMerror;
	}
	last_ch = 0;
	while(quit==FALSE) {
		if((ch = sgetc(f))==EOF) {
			quit = TRUE;
		} else {
			buf1[count++] = (uchar)ch;
			switch(ch) {
			case '{' :
				curly++; break;
			case '(' : 
				if(last_ch!='\\') {
					string++;
				}
				 break;
			case '<' :
				hex++; break;
			case '>' :
				if(hex==0) break;
				hex--; break;
			case '}' :
				if(curly==0) break;
				curly--; break;
			case ')' :
				if(last_ch!='\\') {
					if(string==0) break;
					string--; 
				}
				break;
			case '\n' :
				if(curly==0&&string==0&&hex==0) quit=TRUE;
				break;
			default : break;
			}
		}
		if(ch=='\\'&&last_ch=='\\') {
			last_ch = 0;
		} else {
			last_ch = ch;
		}
	}
	if(curly!=0||string!=0) {
		sclose(f);
		FreeVM(VM,buf1,STATEMENT_BUF);
		FreeVM(VM,(void *)s,sizeof(stream));
		return ERR_ioerror;
	}
	if((exchange = (uchar *)AllocVM(VM,count))==NULL) {
		sclose(f);
		FreeVM(VM,(void *)s,sizeof(stream));
		FreeVM(VM,buf1,STATEMENT_BUF);
		return ERR_VMerror;
	}
	
	for(loop=0;loop<count;loop++) {
		exchange[loop] = buf1[loop];
	}
	FreeVM(VM,(void *)buf1,STATEMENT_BUF);
	sclose(f);
	MakeStringStream(s,exchange,count,S_READABLE);
	result.type = TYPE_FILE|ATTR_EXECUTE;
	result.len = 0;
	SAVE_LEVEL(&result);
	result.obj.streamval = s;
	POPNUMOPER(ctxt,2);
	PUSHEXEC(ctxt,&result);
	AddFile(ctxt,&result);
	result.obj.streamval->flags &=~S_STRING;
	return ERR_OK;
}
/*************************************************************************/
error dummy(DPSContext ctxt) {
	return ERR_OK;
}
/*************************************************************************/


error AddFile(DPSContext ctxt,ps_obj *file) {
	FileList *new;
	
	if((new=(FileList *)AllocVM(VM,sizeof(FileList)))==NULL) {
		return ERR_VMerror;
	}
	new->next = ctxt->space.flist;
	ctxt->space.flist = new;

	if((new->file = (ps_obj *)AllocVM(VM,sizeof(ps_obj)))==NULL) {
		FreeVM(VM,(void *)new,sizeof(FileList));
		return ERR_VMerror;
	}
	new->file->type = file->type;
	new->file->tag = file->tag;
	new->file->len = file->len;
	new->file->obj.streamval = file->obj.streamval;
	
	return ERR_OK;	
}

error SeekAndDestroy(DPSContext ctxt,stream *s) {
	FileList **head,*current;

	head = &ctxt->space.flist;

	while(current = *head) {
		if (current->file->obj.streamval == s) {	// time to remove
			if(sclose(s)) {
				// ****NOTE****
				// FreeVM(VM,(void *)s,sizeof(stream));
			}
			*head=current->next;
			// Free NODE
			FreeVM(VM,(void *)current->file,sizeof(ps_obj));
			FreeVM(VM,(void *)current,sizeof(FileList));
			return ERR_OK;
		}
		else {	// go to next node
			head = &current->next;
		}
	}
	return -1;
}


