/************************************************************************

  Module :  Postscript "Main C entry Point"		© Commodore-Amiga
            (started 26/03/1991)

  Purpose:  This file contains the initialisation code for the a PostScript
            context. It also contains the entry point for the Interpreter.

  Conventions: --

  NOTES: --

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <m68881.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <graphics/gfxbase.h>
#include <intuition/intuition.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/dos.h>
#include <proto/graphics.h>

#include "errors.h"
#include "pstypes.h"
#include "stream.h"
#include "memory.h"
#include "objects.h"

#include "misc.h"
#include "stack.h"
#include "gstate.h"
#include "context.h"

#include "scanner.h"


//--------------------------------------------------------------------
//-------- Internal Debugger values ----------------------------------
//--------------------------------------------------------------------

#define DEBUG_NORMAL	1L
#define DEBUG_STEP		2L

#define VERBOSE_BIT	0x01
#define DEBUG_BIT	0x02

#ifdef MAKELIBRARY

extern long *PostScriptBase;

#pragma libcall PostScriptBase Interpret 2A 801

#endif

//--------------------------------------------------------------------
//-------- Some Useful macros ----------------------------------------
//--------------------------------------------------------------------
#define VM ctxt->space.MemPool

#define OPER_ROOM(N)	(NUMOPER(ctxt)+(N)<O_SIZE)
#define EXEC_ROOM(N)	(NUMEXEC(ctxt)+(N)<E_SIZE)

int verbose;

//--------------------------------------------------------------------
//--- Global READONLY objects :null,mark,true,false,int,real,name,dict
//--------------------------------------------------------------------


ps_obj n_obj	= { (TYPE_NULL | ATTR_LITERAL | ACCESS_UNLIMITED),0,0,0 };
ps_obj m_obj	= { (TYPE_MARK | ATTR_LITERAL | ACCESS_UNLIMITED),0,0,0 };
ps_obj t_obj	= { (TYPE_BOOL | ATTR_LITERAL | ACCESS_UNLIMITED),0,0,TRUE };
ps_obj f_obj	= { (TYPE_BOOL | ATTR_LITERAL | ACCESS_UNLIMITED),0,0,FALSE };
ps_obj int_obj	= { (TYPE_INT  | ATTR_LITERAL | ACCESS_UNLIMITED),0,0,0 };
ps_obj real_obj	= { (TYPE_REAL | ATTR_LITERAL | ACCESS_UNLIMITED),0,0,0 };
ps_obj name_obj	= { (TYPE_NAME | ATTR_LITERAL | ACCESS_UNLIMITED),0,0,0 };
ps_obj dict_obj	= { (TYPE_DICT | ATTR_LITERAL | ACCESS_UNLIMITED),0,0,0 };
ps_obj stop_obj = { (TYPE_STOP | ATTR_EXECUTE | ACCESS_UNLIMITED),0,0,0 };

char *errnames[] = {"dictfull","dictstackoverflow","dictstackunderflow",
					"execstackoverflow","handleerror","interrupt",
					"invalidaccess","invalidexit","invalidfileaccess",
					"invalidfont","invalidrestore","ioerror","limitcheck",
					"nocurrentpoint","rangecheck","stackoverflow",
					"stackunderflow","syntaxerror","timeout","typecheck",
					"undefined","undefinedfilename","undefinedresult",
					"unmatchedmark","unregistered","VMerror",0,0,0,"notimplemented" };


//--------------------------------------------------------------------
//------- Protos -----------------------------------------------------
//--------------------------------------------------------------------
#ifdef MAKELIBRARY
	DPSContext	__saveds __asm NewContext(register __a0 char *);
	void __saveds __asm DestroyContext(register __a0 DPSContext ctxt);
	int __saveds __asm Interpret(register __a0 DPSContext ctxt);
#else
	DPSContext 		NewContext(char *);
	void			DestroyContext(DPSContext ctxt);
	int				Interpret(DPSContext ctxt);
#endif

int		Scan(DPSContext,stream *,int);
BOOL	InitNameSpace(DPSContext);
BOOL	InitSysDict(DPSContext);	
error	HandleError(DPSContext ctxt,int number);
error	ExecPSobj(DPSContext ctxt,ps_obj *obj);

extern	int InitDebug(DPSContext,int);
extern	ps_obj	*NameLookUp(DPSContext,ps_obj *);
extern	void set_round_mode(void);
extern	ps_obj	*FindOperator(DPSContext ctxt, ps_obj *);
extern	uchar *UnPackObject		(DPSContext, uchar *srcp, ps_obj *destobj);
extern 	void Monitor(DPSContext);
extern 	ps_obj *FindDictEntry(DPSContext,ps_obj *,ps_obj *);
extern	char *AddName(DPSContext, char*);
extern	error SeekAndDestroy(DPSContext,stream *);
extern	error AddFile(DPSContext,ps_obj *);

//--------------------------------------------------------------------
//--------------------------------------------------------------------
//          The Interpreter lOOp
//--------------------------------------------------------------------
//--------------------------------------------------------------------

#ifdef MAKELIBRARY
int __saveds __asm Interpret(register __a0 DPSContext ctxt) {
#else
int				Interpret(DPSContext ctxt) {
#endif

int (*function)(void *);

ps_obj *e_sp,*o_sp,*value,file_object;
ps_obj *op_name,op_function;
ps_obj inst;
int scan_result,operator_return,packed_flag;
stream file_stream;

	// Main Interpreter Loop

	while(NUMEXEC(ctxt)>0) {
		packed_flag = 0;
		e_sp = EXECSP(ctxt);

		// Only if Debug Bit is set do we execute monitor 

		if(ctxt->wind != NULL)	Monitor(ctxt);

		// Handle the various objects on the e_stack 

		switch(e_sp->type&TYPE_MASK) {

		case TYPE_FILE :
			if(OPER_ROOM(1)) {
				// Give the file object to the scanner
				if(!(e_sp->obj.streamval->flags&S_STRING)) {
					scan_result = Scan(ctxt,e_sp->obj.streamval,0);
				} else {
					scan_result = Scan(ctxt,e_sp->obj.streamval,1);
				}
				o_sp = OPERSP(ctxt);
				if(scan_result == CONTINUE) {
					if((OBJ_ATTR(o_sp)==ATTR_EXECUTE) &&
					   (OBJ_TYPE(o_sp)!=TYPE_ARRAY) && (OBJ_TYPE(o_sp)!=TYPE_PACKED)) {

						// Push it on the e_stack if it's executeable but not an array

						PUSHEXEC(ctxt,o_sp);
						POPOPER(ctxt);
					}
				} else if(scan_result == COMPLETED) {

					// Close this particular file and pop it off the e_stack

//					sclose(e_sp->obj.streamval);
					SeekAndDestroy(ctxt,e_sp->obj.streamval);
					POPEXEC(ctxt);
				} else if(scan_result == IMMED_ERROR) {
					SeekAndDestroy(ctxt,e_sp->obj.streamval);
					POPEXEC(ctxt);
					PUSHEXEC(ctxt,o_sp);
					POPOPER(ctxt);
				} else {

					// Either a SYNTAX_ERROR or VM_ERROR,
//					sclose(e_sp->obj.streamval);
//					return(ERR_syntaxerror) ;

					SeekAndDestroy(ctxt,e_sp->obj.streamval);
					POPEXEC(ctxt);
					HandleError(ctxt,ERR_syntaxerror);
				}
			} else {
				if(!ctxt->space.recurse) {
					HandleError(ctxt,ERR_stackoverflow);
				} else {
					return ERR_stackoverflow;
				}
			}
			break;

		case TYPE_OPERATOR :

			// Take the operator object off the e_stack before calling
			// the operator. That way, every operator dosn't have to do
			// this every time it's called.

			function = e_sp->obj.operval;
			POPEXEC(ctxt);
			operator_return = (*function)(ctxt);
			if(operator_return) { 
				if(ctxt->space.recurse) {
					return operator_return;
				}

				// This is used to display the name of the function that returned
				// the error
				op_function.type = TYPE_OPERATOR|ATTR_EXECUTE|ACCESS_UNLIMITED;
				op_function.len = 0;
				op_function.tag = 0;
				op_function.obj.operval = function;
				PUSHOPER(ctxt,&op_function);

				// This bit of code if used for our own debug messages !
				//-----------------------------------------------------
//				op_name = FindOperator(ctxt,&op_function);

//				if(OBJ_TYPE(op_name)!=TYPE_NAME) {
//					fprintf(OP,"UNKNOWN ");
//				} else {
//					fprintf(OP,"op_%s() ",op_name->obj.nameval);
//				}
//				fprintf(OP,"returned an error : %d :\n",operator_return);
				//------------------------------------------------------

				// Now carry on with the rest of the error handling

				if(operator_return==ERR_QUIT_INTERPRETER) {
					return NULL;
				} else {
					HandleError(ctxt,operator_return);
				}
			}
			break;

		case TYPE_NAME :
			if(OBJ_ATTR(e_sp)==ATTR_LITERAL) {

				// Check the type of the name, if its literal put it on 
				// the operand stack, else do a name lookup.

				if(OPER_ROOM(1)) {
					PUSHOPER(ctxt,e_sp);
					POPEXEC(ctxt);
				} else {
					return NULL;
				}
			} else {

				// It's an executable so do an immediate name lookup.

				value = NameLookUp(ctxt,e_sp); 
				if(value==NULL) {
					if(!ctxt->space.recurse) {
						PUSHOPER(ctxt,e_sp);
						POPEXEC(ctxt);
						HandleError(ctxt,ERR_undefined);
						// Our Debug message 
						// fprintf(OP,"NameLookUp() could not find [%s]\n",e_sp->obj.nameval);
						break;
					} else {
						return ERR_undefined;
					}
				}
				POPEXEC(ctxt);
				if((value->type&ATTR_MASK)==ATTR_EXECUTE) {
					PUSHEXEC(ctxt,value);
				} else {
					PUSHOPER(ctxt,value);
				}
			}
			break;

		// Apply a tail recursion method to handle executable arrays.
		// If the length is zero, we have finished, else if the length
		// is one then place the "tail" of the array on the e_stack, else
		// place the value on the o_stack if its an int,real or literal array, if
		// not put it on the e_stack
		// The routines for packed/un-packed arrays are very similiar. They only
		// differ in there approach to getting the next object. This is 
		// controlled by the packed_flag.

		case TYPE_PACKED :
			packed_flag = TRUE;
			value = &inst;
		case TYPE_ARRAY :
			if(OBJ_ATTR(e_sp)==ATTR_LITERAL) {
				PUSHOPER(ctxt,e_sp);
				POPEXEC(ctxt);
				break;
			}
			if(e_sp->len==0) {
				POPEXEC(ctxt);
			} else if(e_sp->len) {
				if(packed_flag) {
					e_sp->obj.packval = UnPackObject(ctxt,e_sp->obj.packval,value);
				} else {
					value = (ps_obj *)e_sp->obj.arrayval++;
				}
				if(e_sp->len==1) {
					POPEXEC(ctxt);
				} else {
					e_sp->len--;
				}
				if(OBJ_TYPE(value)==TYPE_ARRAY||OBJ_TYPE(value)==TYPE_PACKED) {
					if(OPER_ROOM(1)) {
						PUSHOPER(ctxt,value);
					} else {
						return NULL;
					}
				} else {
					PUSHEXEC(ctxt,value);
				}
			}
			break;

		case TYPE_STRING :

			// If a string is literal it's put directly on the o_stack, else
			// we create a stream on the string and push that on the e_stack to
			// be interpreted.

			if(OBJ_ATTR(e_sp)==ATTR_LITERAL) {
				if(OPER_ROOM(1)) {
					PUSHOPER(ctxt,e_sp);
					POPEXEC(ctxt);
				} else {
					return NULL;
				}
			} else {
				POPEXEC(ctxt);
				if(MakeStringStream(&file_stream,e_sp->obj.stringval,e_sp->len,S_READABLE)) {
					file_object.type = ATTR_EXECUTE|TYPE_FILE;
					file_object.len = 0;
					file_object.tag = ctxt->space.save_level;
					file_object.obj.streamval = &file_stream;
					PUSHEXEC(ctxt,&file_object);
					AddFile(ctxt,&file_object);
				}
				
			}
			break;
		case TYPE_STOP :
			// This object is used to force the interpreter to quit immediately
			// from this level. We could have recursed !

			POPEXEC(ctxt);
			return ERR_OK;
			break;

		// A catch all to push anything not defined above onto the o_stack
		case TYPE_NULL :
		case TYPE_INT :
		case TYPE_REAL :
		case TYPE_BOOL :
		case TYPE_IMMED :
		case TYPE_DICT :
		case TYPE_MARK :
		case TYPE_FONTID :
		case TYPE_LOCK :
		case TYPE_SAVE :

			if(OPER_ROOM(1)) {
				PUSHOPER(ctxt,e_sp);
				POPEXEC(ctxt);
			} else {
				return NULL;
			}

			break;
		}
	}
}

error HandleError(DPSContext ctxt,int number) {
	ps_obj error_object,*value;

	error_object.obj.nameval = AddName(ctxt, errnames[number-1]);
	error_object.type = TYPE_NAME;
	error_object.len = 0;
	error_object.tag = 0;
	value = FindDictEntry(ctxt,&ctxt->space.errdict,&error_object);
	if (value) {
		PUSHEXEC(ctxt,value);
	}
	return ERR_OK;
}
error ExecPSobj(DPSContext ctxt,ps_obj *obj) {
	int err;
	ctxt->space.recurse++;
	PUSHEXEC(ctxt,&stop_obj);
	PUSHEXEC(ctxt,obj);
	err = Interpret(ctxt);
	ctxt->space.recurse--;
	return err;
}



//--------------------------------------------------------------------
//-------- Support Functions -----------------------------------------
//--------------------------------------------------------------------

#ifdef MAKELIBRARY
DPSContext	__saveds __asm NewContext(register __a0 char *filename) {
#else
DPSContext NewContext(char *filename) {
#endif
	vmem *m;
	DPSContext ctxt;
	ps_obj *Stdin;
	stream *ip,*input,*output;
	int loop;


	set_round_mode();				// Sets Rounding mode towards zero


	if((m = CreateVM())==NULL) {
		return NULL;
		
	}
	if((ctxt = (DPSContext)AllocVM(m,sizeof(DPSContextRec)))==NULL) {
		DestroyVM(m);
		return NULL;
	}
	ctxt->space.MemPool = m;


	ctxt->STDOUT = (int)fopen("*","w");


	ctxt->space.stacks.o.sp = ((ps_obj *)AllocVM(VM,sizeof(ps_obj)*O_SIZE))+(O_SIZE);
	ctxt->space.stacks.e.sp = ((ps_obj *)AllocVM(VM,sizeof(ps_obj)*(E_SIZE+5)))+((E_SIZE+5));
	ctxt->space.stacks.d.sp = ((ps_obj *)AllocVM(VM,sizeof(ps_obj)*D_SIZE))+(D_SIZE);

//  Obsolete !

	ctxt->space.stacks.g.sp = ((ps_obj *)AllocVM(VM,sizeof(ps_obj)*G_SIZE))+(G_SIZE);
	
	if(ctxt->space.stacks.o.sp == NULL || ctxt->space.stacks.e.sp == NULL ||
	ctxt->space.stacks.d.sp == NULL || ctxt->space.stacks.g.sp == NULL) {
		DestroyVM(m);
		return NULL;
	}
	ctxt->space.stacks.o.num = 0;
	ctxt->space.stacks.e.num = 0;
	ctxt->space.stacks.d.num = 0;
	ctxt->space.stacks.g.num = 0;
	ctxt->space.save_level = 0;
	ctxt->space.seed	= 1;
	
	ctxt->space.flist = NULL;

	if((ctxt->space.GState = (gstate *)AllocVM(VM,sizeof(gstate)))==NULL) {
		DestroyVM(m);
		return NULL;
	}


	ctxt->space.PackingOn = FALSE;

	// Setup the Standard Input and Ouput File Objects

	input = (stream *)AllocVM(VM,sizeof(stream));
	output = (stream *)AllocVM(VM,sizeof(stream));
	
	OpenFileStream(input,"*",S_READABLE);
	OpenFileStream(output,"*",S_WRITEABLE);

	ctxt->space.StdIn = (ps_obj *)AllocVM(VM,sizeof(ps_obj));
	ctxt->space.StdIn->obj.streamval = input;
	ctxt->space.StdIn->type = TYPE_FILE|ATTR_LITERAL;
	ctxt->space.StdIn->len = 0;
	ctxt->space.StdIn->tag = 0;

	ctxt->space.StdOut = (ps_obj *)AllocVM(VM,sizeof(ps_obj));
	ctxt->space.StdOut->obj.streamval = output;
	ctxt->space.StdOut->type = TYPE_FILE|ATTR_LITERAL;
	ctxt->space.StdOut->len = 0;
	ctxt->space.StdOut->tag = 0;


	if(AddFile(ctxt,ctxt->space.StdIn)!=ERR_OK) {
//		fprintf(OP,"Problem adding StdIn\n");
	}
	if(AddFile(ctxt,ctxt->space.StdOut)!=ERR_OK) {
//		fprintf(OP,"Problem adding StdOut\n");
	}


	ctxt->Debg.ExecMode = DEBUG_STEP;
	ctxt->wind = NULL;
	ctxt->rp = 0;				// No RastPort available yet...

	// Setup the initial file object on the e stack 

	ctxt->space.recurse = 0;

	if((ip = (stream *)AllocVM(VM,sizeof(stream)))==NULL) {
		return NULL;
	}

	if((OpenFileStream(ip,filename,S_READABLE)) == NULL) {
		return NULL;
	}

	// Not Needed anymore because this is inside the FileList.
	ctxt->space.ip = ip;

	Stdin = (ps_obj *)AllocVM(VM,sizeof(ps_obj));
	Stdin->type = TYPE_FILE;
	Stdin->obj.streamval = ip;
	Stdin->len = 0;
	Stdin->tag = 0;
	PUSHEXEC(ctxt,Stdin);
	

	if(AddFile(ctxt,Stdin)!=ERR_OK) {
//		fprintf(OP,"THERE HAS been and error with the second Stdin\n");
	}


	// Allocate space for the System Name Table


	if((ctxt->space.SysNameTab = (char **)AllocVM(VM,sizeof(char *)*256))==NULL){
		DestroyVM(m);
		return NULL;
	}

	for(loop=0;loop<256;loop++) {
		ctxt->space.SysNameTab[loop] = NULL;
	}

	// Initialise Larry's name space and dictionaries 
	if (! InitNameSpace(ctxt)) {
		DestroyVM(m);
		return NULL;
	}
	
	if ( InitSysDict(ctxt)) {
		DestroyVM(m);
		return NULL;
	}	
	if(filename[0] == '*') InitDebug(ctxt,1);
	return(ctxt);

}

#ifdef MAKELIBRARY
void __saveds __asm DestroyContext(register __a0 DPSContext ctxt) {
#else
void DestroyContext(DPSContext ctxt) {
#endif

	FileList *head,*current;

	head = ctxt->space.flist;
	while(current = head) {
		sclose(current->file->obj.streamval);
		head = current->next;
	}

	// Close Window if it's open 

	if(ctxt->wind) {
		CloseWindow(ctxt->wind);
	}

//	sclose(ctxt->space.ip);
//	sclose(ctxt->space.StdOut->obj.streamval);
//	sclose(ctxt->space.StdIn->obj.streamval);

	fclose((FILE *)ctxt->STDOUT);
	
	DestroyVM(ctxt->space.MemPool);
	return ;
}

