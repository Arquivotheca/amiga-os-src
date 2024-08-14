/****************************************************************************
 Prototypes for all functions internal to the PostScript library.  PostScript
 operators are always preceded by ps_ while other routines have no standard
 prefix.  PostScript functions are prototyped first followed by non-PS
 functions that need to be called from other modules.
****************************************************************************/

/**** Functions defined in newbin_enc.c ************************************/
int		HandleBinaryTokens	(DPSContext,stream *,int);
int		HandleBinaryObjects	(stream *,int);


/**** Functions defined in char_array.c ************************************/


/**** Functions defined in debug.c *****************************************/
int		InitDebug			(DPSContext,int);
void	ClearDisplay		(DPSContext );
void	RefreshScreen		(DPSContext);
void	UpdateStacks		(DPSContext,int);
void	Monitor				(DPSContext);
void	Box					(struct RastPort *,int,int,int,int,int);


/**** Functions defined in dict.c ******************************************/
error	InitSysDict			(DPSContext);
pso		*InitDict			(DPSContext, int);
pso		*NameLookUp			(DPSContext, pso*);
pso		*FindDictEntry		(DPSContext, pso*, pso*);
BOOL	Define				(DPSContext, pso*, pso *key, pso *val);
int		AddOperator			(DPSContext, char*, int (*funcptr)() );
pso		*FindOperator		(DPSContext, pso*);


/**** Functions defined in memory.c ****************************************/
vmem	*CreateVM			(void);
void	DestroyVM			(vmem *);
void	*AllocVM			(vmem *,ulong);
void	FreeVM				(vmem *, void *, ulong);
void	*ReAllocVM			(vmem *,void *,ulong,ulong);
void	*AllocTrackedVM		(vmem *,ulong);
void	FreeTrackedVM		(vmem *, void *);
void	ReUseTrackedVM		(vmem *,void *);
void	*ConvertVM			(vmem *, void *, ulong);
int		GetTotalVM			(vmem *);
int		GetFreeVM			(vmem *);


/**** Functions defined in names.c *****************************************/
BOOL	InitNameSpace		(DPSContext);
char	*AddName			(DPSContext, char *);


/**** Functions defined in op_array.c **************************************/
error	ps_array			(DPSContext);
error	ps_closesquare		(DPSContext);
error	ps_length			(DPSContext);
error	ps_getinterval		(DPSContext);
error	ps_putinterval		(DPSContext);
error	ps_aload			(DPSContext);
error	ps_astore			(DPSContext);
error	ps_get				(DPSContext);
error	ps_put				(DPSContext);

void	AddOpsArray			(DPSContext);


/**** Functions defined in op_attr.c ***************************************/
error	dps_type			(DPSContext);	// extended for DPS
error	ps_cvlit			(DPSContext);
error	ps_cvx				(DPSContext);
error	ps_xcheck			(DPSContext);
error	ps_executeonly		(DPSContext);
error	ps_noaccess			(DPSContext);
error	ps_readonly			(DPSContext);
error	ps_rcheck			(DPSContext);
error	ps_wcheck			(DPSContext);
error	ps_cvi				(DPSContext);
error	ps_cvn				(DPSContext);
error	ps_cvr				(DPSContext);
error	ps_cvrs				(DPSContext);
error	ps_cvs				(DPSContext);

void	AddOpsAttr			(DPSContext);
int		GetNumber			(char *,double *);


/**** Functions defined in op_bool.c ***************************************/
error	ps_eq				(DPSContext);
error	ps_ne				(DPSContext);
error	ps_ge				(DPSContext);
error	ps_gt				(DPSContext);
error	ps_le				(DPSContext);
error	ps_lt				(DPSContext);
error	ps_and				(DPSContext);
error	ps_not				(DPSContext);
error	ps_or				(DPSContext);
error	ps_xor				(DPSContext);
error	ps_bitshift			(DPSContext);

void	AddOpsBool			(DPSContext);


/**** Functions defined in op_control.c ************************************/
error	ps_exec				(DPSContext);
error	ps_if				(DPSContext);
error	ps_ifelse			(DPSContext);
error	ps_for				(DPSContext);
error	ps_repeat			(DPSContext);
error	ps_loop				(DPSContext);
error	ps_exit				(DPSContext);
error	ps_stop				(DPSContext);
error	ps_stopped			(DPSContext);
error	ps_countexecstack	(DPSContext);
error	ps_forall			(DPSContext);
error	ps_execstack		(DPSContext);
error	ps_quit				(DPSContext);
error	ps_start			(DPSContext);

void	AddOpsControl		(DPSContext);


/**** Functions defined in op_device.c *************************************/
error	ps_showpage			(DPSContext);
error	ps_copypage			(DPSContext);
error	ps_banddevice		(DPSContext);
error	ps_framedevice		(DPSContext);
error	ps_nulldevice		(DPSContext);
error	ps_renderbands		(DPSContext);

void	 AddOpsDevice		(DPSContext);


/**** Functions defined in op_dict.c ***************************************/
error	ps_dict				(DPSContext);
error	ps_maxlength		(DPSContext);
error	ps_begin			(DPSContext);
error	ps_end				(DPSContext);
error	ps_def				(DPSContext);
error	ps_load				(DPSContext);
error	ps_store			(DPSContext);
error	ps_known			(DPSContext);
error	ps_where			(DPSContext);
error	ps_currentdict		(DPSContext);
error	ps_countdictstack	(DPSContext);
error	ps_dictstack		(DPSContext);
error	dps_cleardictstack	(DPSContext);

void	AddOpsDict			(DPSContext);


/**** Functions defined in op_file.c ***************************************/
error	ps_currentfile		(DPSContext);
error	ps_file				(DPSContext);
error	ps_token			(DPSContext);
error	ps_closefile		(DPSContext);
error	ps_print			(DPSContext);
error	ps_flush			(DPSContext);
error	ps_read				(DPSContext);
error	ps_write			(DPSContext);
error	ps_run				(DPSContext);
error	dps_status			(DPSContext);
error	ps_bytesavailable	(DPSContext);
error	ps_readhexstring	(DPSContext);
error	ps_writehexstring	(DPSContext);
error	ps_readstring		(DPSContext);
error	ps_writestring		(DPSContext);
error	ps_readline			(DPSContext);
error	ps_flushfile		(DPSContext);
error	ps_resetfile		(DPSContext);
error	ps_echo				(DPSContext);
error	dps_deletefile		(DPSContext);
error	dps_renamefile		(DPSContext);
error	dps_filenameforall	(DPSContext);
error	dps_setfileposition	(DPSContext);
error	dps_fileposition	(DPSContext);

void	AddOpsFile			(DPSContext);
int		CompString			(int,char *,char *);
error	GetStatement		(DPSContext);
error	dummy				(DPSContext);


/**** Functions defined in op_font.c ***************************************/
error	ps_definefont		(DPSContext);
error	ps_scalefont		(DPSContext);
error	ps_makefont			(DPSContext);
error	ps_setfont			(DPSContext);
error	ps_currentfont		(DPSContext);
error	ps_show				(DPSContext);
error	ps_ashow			(DPSContext);
error	ps_widthshow		(DPSContext);
error	ps_awidthshow		(DPSContext);
error	ps_kshow			(DPSContext);
error	ps_stringwidth		(DPSContext);
error	ps_charpath			(DPSContext);
error	ps_cachestatus		(DPSContext);
error	ps_setcachedevice	(DPSContext);
error	ps_setcharwidth		(DPSContext);
error	ps_setcachelimit	(DPSContext);
error	ps_setcacheparams	(DPSContext);
error	ps_currentcacheparams (DPSContext);

void	fixer				(float, float);
void	AddOpsFont			(DPSContext);


/**** Functions defined in op_math.c ****************************************/
error	ps_div				(DPSContext);
error	ps_idiv				(DPSContext);
error	ps_mod				(DPSContext);
error	ps_abs				(DPSContext);
error	ps_neg				(DPSContext);
error	ps_ceiling			(DPSContext);
error	ps_floor			(DPSContext);	// **!! WARNING (see code)
error	ps_round			(DPSContext);	// **!! WARNING
error	ps_truncate			(DPSContext);
error	ps_sqrt				(DPSContext);
error	ps_atan				(DPSContext);
error	ps_cos				(DPSContext);
error	ps_sin				(DPSContext);
error	ps_exp				(DPSContext);
error	ps_ln				(DPSContext);
error	ps_log				(DPSContext);
error	ps_rand				(DPSContext);
error	ps_srand			(DPSContext);
error	ps_rrand			(DPSContext);

void	AddOpsMath			(DPSContext);


/**** Functions defined in op_matrix.c **************************************/
error	ps_matrix			(DPSContext);
error	ps_initmatrix		(DPSContext);
error	ps_identmatrix		(DPSContext);
error	ps_defaultmatrix	(DPSContext);
error	ps_currentmatrix	(DPSContext);
error	ps_setmatrix		(DPSContext);
error	ps_translate		(DPSContext);
error	ps_scale			(DPSContext);
error	ps_rotate			(DPSContext);
error	ps_concat			(DPSContext);
error	ps_concatmatrix		(DPSContext);
error	ps_transform		(DPSContext);
error	ps_dtransform		(DPSContext);
error	ps_itransform		(DPSContext);
error	ps_idtransform		(DPSContext);
error	ps_invertmatrix		(DPSContext);

void	MultMatrix			(float *, float *, float *);
error	InvertMatrix		(float *, float *);
void	TransformPoint		(float *, float, float, float *, float *);
void	DTransformPoint		(float *, float, float, float *, float *);
error	IsMatrix			(ps_obj *);
error	IsMatrixSized		(ps_obj *);
void	FetchMatrix			(ps_obj *, float *);
void	PutMatrix			(ps_obj *, float *);
error	CheckMatrixArgs		(DPSContext, ps_obj *,int);
error	CheckICTM			(DPSContext);
void	AddOpsMatrix		(DPSContext);


/**** Functions defined in op_misc.c ****************************************/
error	ps_bind				(DPSContext);
error	ps_usertime			(DPSContext);
error	dps_currentstrokeadjust	(DPSContext);
error	dps_setstrokeadjust	(DPSContext);

void	AddOpsMisc			(DPSContext);


/**** Functions defined in op_opstack.c *************************************/
error	ps_pop				(DPSContext);
error	ps_exch				(DPSContext);
error	ps_dup				(DPSContext);
error	ps_copy				(DPSContext);
error	ps_index			(DPSContext);
error	ps_roll				(DPSContext);
error	ps_clear			(DPSContext);
error	ps_count			(DPSContext);
error	ps_mark				(DPSContext);
error	ps_cleartomark		(DPSContext);
error	ps_counttomark		(DPSContext);

void	AddOpsOpStack		(DPSContext);


/**** Functions defined in op_string.c **************************************/
error	ps_string			(DPSContext);
error	ps_anchorsearch		(DPSContext);
error	ps_search			(DPSContext);

void	AddOpsString		(DPSContext);

/**** Functions defined in op_vm.c ******************************************/
error	ps_vmstatus			(DPSContext);
error	ps_save				(DPSContext);
error	ps_restore			(DPSContext);

void	AddOpsVM			(DPSContext);


/**** Functions defined in op_gstate.c **************************************/
error	ps_gsave			(DPSContext);
error	ps_grestore			(DPSContext);
error	ps_grestoreall		(DPSContext);
error	ps_initgraphics		(DPSContext);
error	ps_setlinewidth		(DPSContext);
error	ps_setlinecap		(DPSContext);
error	ps_setlinejoin		(DPSContext);
error	ps_setmiterlimit	(DPSContext);
error	ps_setdash			(DPSContext);
error	ps_setflat			(DPSContext);
error	ps_setgray			(DPSContext);
error	ps_sethsbcolor		(DPSContext);
error	ps_setrgbcolor		(DPSContext);
error	ps_setscreen		(DPSContext);
error	ps_settransfer		(DPSContext);
error	ps_currentlinewidth	(DPSContext);
error	ps_currentlinecap	(DPSContext);
error	ps_currentlinejoin	(DPSContext);
error	ps_currentmiterlimit (DPSContext);
error	ps_currentdash		(DPSContext);
error	ps_currentflat		(DPSContext);
error	ps_currentgray		(DPSContext);
error	ps_currenthsbcolor	(DPSContext);
error	ps_currentrgbcolor	(DPSContext);
error	ps_currentscreen	(DPSContext);
error	ps_currenttransfer	(DPSContext);

void	AddOpsGState		(DPSContext);
void	HSL_to_RGB			(float,float,float,float*,float*,float*);
void	RGB_to_HSL			(float,float,float,float*,float*,float*);


/**** Functions defined in op_path.c ****************************************/
error	ps_newpath			(DPSContext);
error	ps_currentpoint		(DPSContext);
error	ps_moveto			(DPSContext);
error	ps_rmoveto			(DPSContext);
error	ps_lineto			(DPSContext);
error	ps_rlineto			(DPSContext);
error	ps_arc				(DPSContext);
error	ps_arcn				(DPSContext);
error	ps_arcto			(DPSContext);
error	ps_curveto			(DPSContext);
error	ps_rcurveto			(DPSContext);
error	ps_closepath		(DPSContext);
error	ps_pathforall		(DPSContext);
error	ps_flattenpath		(DPSContext);
error	ps_reversepath		(DPSContext);
error	ps_strokepath		(DPSContext);
error	ps_pathbbox			(DPSContext);
error	ps_initclip			(DPSContext);
error	ps_clip				(DPSContext);
error	ps_eoclip			(DPSContext);

void	AddOpsPath			(DPSContext);
float	*NextPathSeg		(DPSContext, int);
error	movlin				(DPSContext, int, int);
error	push2reals			(DPSContext, float, float);
int		arc					(double,double,double,double,double,int,float *);


/**** Functions defined in op_userob.c **************************************/
error	dps_defineuserobject(DPSContext);
error	dps_execuserobject	(DPSContext);
error	dps_undefineuserobject	(DPSContext);

void	AddOpsUserOb		(DPSContext);


/**** Functions defined in op_struct.c **************************************/
error	dps_currentobjectformat	(DPSContext);
error	dps_printobject		(DPSContext);
error	dps_setobjectformat	(DPSContext);
error	dps_writeobject		(DPSContext);

void	AddOpsStruct		(DPSContext);


/**** Functions defined in op_mec.c *****************************************/
error	dps_condition		(DPSContext);
error	dps_currentcontext	(DPSContext);
error	dps_fork			(DPSContext);
error	dps_join			(DPSContext);
error	dps_lock			(DPSContext);
error	dps_monitor			(DPSContext);
error	dps_notify			(DPSContext);
error	dps_wait			(DPSContext);
error	dps_yield			(DPSContext);

void	AddOpsMEC			(DPSContext);


/**** Functions defined in op_debug.c ***************************************/
error	ps_debugon			(DPSContext);
error	ps_debugoff			(DPSContext);
error	ps_showhash			(DPSContext);
error	ps_flash			(DPSContext);

void	AddOpsDebug			(DPSContext);


/**** Functions defined in newscan.c ****************************************/
int		Scan				(DPSContext,stream *,int);


/**** Functions defined in stream.c *****************************************/
int		StringRead			(stream *);
int		StringReadS			(stream *, uchar *, int);
int		StringWrite			(stream *,uchar);
int		StringWriteS		(stream *, uchar *, int);
int		StringSeek			(stream *, int);
int		StringFlush			(stream *);
int		StringClose			(stream *);
int		FileRead			(stream *);
int		FileReadS			(stream *,uchar *, int);
int		FileWrite			(stream *,uchar);
int		FileWriteS			(stream *,uchar *,int);
int		FileSeek			(stream *, int);
int		FileFlush			(stream *);
int		FileClose			(stream *);


/**** Functions defined in newsupport.c *************************************/
int		GetEntireString		(DPSContext,stream *,int,ps_obj *);
int		GetEntireHexString	(DPSContext,stream *,ps_obj *);
int		Get_Esc_Char		(stream *);


/**** Functions defined in op_paint.c ***************************************/
error	ps_eofill			(DPSContext);
error	ps_fill				(DPSContext);
error	ps_stroke			(DPSContext);
// these are here to give us a display for writing to, just testing
error	ps_klugeon			(DPSContext);
error	ps_klugeoff			(DPSContext);


/**** Functions defined in pathpaint.c **************************************/
error	Bezier				(DPSContext, double, double, float *);
error	AddPrivateSeg		(DPSContext, int, double, double);
error	CopyPrivateToPath	(DPSContext);


/**** Functions defined in lib_public.c *************************************/
DPSContext	__saveds __asm NewContext		(register __a0 char *);
void		__saveds __asm DestroyContext	(register __a0 DPSContext);
int			__saveds __asm Interpret		(register __a0 DPSContext);

