
******* jcc.library/AllocConversionHandleA ***********************************
*
*   NAME
*	AllocConversionHandleA -- allocate JConversionCodeSet structure
*	AllocConversionHandle -- varargs stub for AllocConversionHandleA().
*
*   SYNOPSYS
*	jcc = AllocConversionHandleA(taglist);
*	D0                           A0
*
*	struct JConversionCodeSet *AllocConversionHandleA(struct TagItem *);
*
*	jcc = AllocConversionHandle(firstTag, ...);
*
*	struct JConversionCodeSet *AllocConversionHandle(Tag, ...);
*
*   FUNCTION
*	Allocate JConversionCodeSet structure.
*
*   INPUTS
*	taglist - pointer to TagItem list.
*
*   TAGS
*	JCC_WhatInputCode (LONG) - input code type
*	JCC_WhatOutputCode (LONG) - output code type
*	JCC_FirstByte (BYTE) - first byte of double-byte
*	JCC_ESC (LONG) - escape sequece for JIS (New-JIS, Old-JIS and
*	                 NEC-JIS) code
*	JCC_HanKata (LONG) - single-byte KATAKANA
*	JCC_ShiftedIn (BOOL) - TRUE: in double-byte JIS mode
*	                       FALSE: in single-byte ASCII mode
*
*   CODESETS
*	CTYPE_EUC - EUC is Japanese code for Amiga and UNIX.
*	CTYPE_SJIS - Shift-JIS is most common Japanese code for personal
*	             computer including MS-DOS machine.
*	CTYPE_NEWJIS - New Japanese Industrial Standard code.
*	CTYPE_OLDJIS - Old Japanese Industrial Standard code.
*	CTYPE_NECJIS - NEC Japanese Industrial Standard code.
*                    
*   RESULT
*	jcc - an allocated JConversionCodeSet data structure, or NULL on
*	      failure.
*
*   SEE ALSO
*	FreeConversionHandle()
*
******************************************************************************


******* jcc.library/FreeConversionHandle *************************************
*
*   NAME
*	FreeConversionHandle -- free JConversionCodeSet structure
*
*   SYNOPSYS
*	FreeConversionHandleA(jcc);
*	                      A0
*
*	VOID FreeConversionHandleA(struct JConversionCodeSet *);
*
*   FUNCTION
*	Free JConversionCodeSet structure that was allocated by
*	AllocConversionHandle().
*
*   INPUTS
*	jcc - pointer to JConversionCodeSet.
*	      if it is NULL, this function does nothing.
*
*   SEE ALSO
*	AllocConversionHandle()
*
******************************************************************************


******* jcc.library/GetJConversionAttrsA *************************************
*
*   NAME
*	GetJConversionAttrsA -- get the attributes of a JConversionCodeSet
*	                        data.
*	GetJConversionAttrs -- varargs stub for GetJConversionAttrsA().
*
*   SYNOPSYS
*	numProcessed = GetJConversionAttrsA(jcc, taglist);
*	D0                                 A0   A1
*
*	LONG GetJConversionAttrsA(struct JConversionCodeSet *,
*	                         struct TagItem *);
*
*	numProcessed = GetJConversionAttrs(jcc, firstTag, ...);
*
*	LONG GetJConversionAttrs(struct JConversionCodeSet *, Tag, ...);
*
*   FUNCTION
*	Retrieve the attributes of the JConversionCodeSet data, according to
*	the attributes chosen in the tag list.  For each entry in the tag
*	list, ti_Tag identifies the attribute, and ti_Data is a pointer to
*	the long variable where you wish the result to be stored.
*
*   INPUTS
*	jcc - pointer to JConversionCodeSet.
*	      if it is NULL, this function returns 0.
*	taglist - pointer to TagItem list.
*
*   TAGS
*	See AllocConversionHandle().
*
*   RESULT
*	numProcessed - the number of attributes successfully filled in.
*
*   SEE ALSO
*	SetJConversionAttrs()
*
******************************************************************************


******* jcc.library/SetJConversionAttrsA *************************************
*
*   NAME
*	SetJConversionAttrsA -- set the attributes of a JConversionCodeSet
*	                        data.
*	SetJConversionAttrs -- varargs stub for SetJConversionAttrsA().
*
*   SYNOPSYS
*	SetJConversionAttrsA(jcc, taglist);
*	                    A0   A1
*
*	VOID SetJConversionAttrsA(struct JConversionCodeSet *,
*	                         struct TagItem *);
*
*	SetJConversionAttrs(jcc, firstTag, ...);
*
*	VOID SetJConversionAttrs(struct JConversionCodeSet *, Tag, ...);
*
*   FUNCTION
*	Change the attributes of the JapaneseConversionCodeSet data, according
*	to the attributes chosen in the tag list. If an attribute is not
*	provided in the tag list, its value remains unchanged.
*
*   INPUTS
*	jcc - pointer to JConversionCodeSet.
*	      if it is NULL, this function does nothing.
*	taglist - pointer to TagItem list.
*
*   TAGS
*	See AllocConversionHandle().
*
*   SEE ALSO
*	GetJConversionAttrs()
*
******************************************************************************


******* jcc.library/IdentifyCodeSet ******************************************
*
*   NAME
*	IdentifyCodeSet -- Detect code set
*
*   SYNOPSYS
*	codeset = IdentifyCodeSet(jcc, inbuf, length);
*	D0                        A0   A1     D0
*
*	LONG IdentifyCodeSet(struct JConversionCodeSet *jcc, UBYTE *inbuf,
*	                     ULONG length);
*
*   FUNCTION
*	Determine code set in input buffer string.
*
*	The length parameter specifies how many characters to use for
*	detection, or if the length is specified as -1 then the string
*	are parsed until NULL is encountered and the function supposes
*	the string is continuous.
*
*	- User's responsibility to determine how many bytes enough to feed.
*	- User's responsibility to determine what kinds of string to feed.
*	- Assume never combine Japanese charcater and Europian character
*	  for input string. Especially, it's diffecult to determine Shift-JIS
*	  and ASCII including Europian character.
*	- When you get "CTYPE_BINARY", you don't need to feed more strings.
*	  In the other way, as long as returns the other codes than
*	  "CTYPE_BINARY", you might doubt the answer is wrong.
*	- New-JIS, Old-JIS and NEC-JIS can't determine untill get escape code
*	  in string. If end of string has part of escape code, return
*	  "CTYPE_CONTINUE". When you get "CTYPE_NEW" or "CTYPE_OLD" or
*	  "CTYPE_NEC", if you think string does not have binary data, you don't
*	  need to feed more strings.
*	- Shift-JIS and EUC are needed to check first byte and second byte.
*	  But still can't determine with only few bytes, because 25 % of EUC
*	  code has same code with Shift-JIS like "e0c0". And also all of EUC
*	  single-byte KATAKANA is located in Shift-JIS. This function returns
*	  "CTYPE_EUC" in these cases. While returnig "CTYPE_ASCII"
*	  and "CTYPE_EUC", you had better feed more strings.
*	  When you get "CTYPE_SJIS", if you think string does not have binary
*	  data, you don't need to feed more strings.
*	  The function returns "CTYPE_SJIS", only when get the character which
*	  is not "EUC". If user does not give reasonal string, this routine
*	  might give wrong answer. If end of string has first byte Shift-JIS,
*	  returns "CTYPE_CONTINUE".
*	- In any cases, while returning "CTYPE_ASCII" and "CTYPE_CONTINUE",
*	  please feed more strings.
*
*   INPUTS
*	jcc - pointer to JConversionCodeSet.
*	      if it is NULL, this function returns -1.
*	inbuf - input string to be determined.
*	length - how many characters to use for detection, or if the length
*	         is specified as -1 then the string are parsed until NULL
*	         is encountered and the function supposes the string is
*	         continuous.
*
*   RESULTS
*	-1:            / jcc is NULL
*	CTYPE_UNKNOWN  / Can't determine
*	CTYPE_ASCII    / ASCII code only (include Europian)
*	CTYPE_BINARY   / Include binary data
*	CTYPE_EUC      / EUC code
*	CTYPE_SJIS     / Shift JIS code
*	CTYPE_NEWJIS   / New JIS code
*	CTYPE_OLDJIS   / Old JIS code
*	CTYPE_NECJIS   / NEC JIS code
*	CTYPE_CONTINUE / Give continue string
*
*   SEE ALSO
*	See sample "jisconv.c"
*	More information for code set is in "jiscode.doc"
*
******************************************************************************


******* jcc.library/JStringConvertA ******************************************
*
*   NAME
*	JStringConvertA -- convert input string to output string according
*	                   code set.
*	JStringConvert -- varargs stub for JStringConvertA().
*
*   SYNOPSYS
*	number = JStringConvertA(jcc, inbuf, outbuf, incode, outcode,
*	D0                       A0   A1     A2      D0      D1
*	                         length, taglist);
*	                         D2      A3
*
*	LONG JStringConvertA(struct JConversionCodeSet *jcc,
*	                     UBYTE *inbuf, UBYTE *outbuf,
*	                     LONG incode, LONG outcode,
*	                     LONG length,
*	                     struct TagItem *);
*
*	number = JStringConvert(jcc, inbuf, outbuf, incode, outcode,
*	                        length, firstTag, ...);
*
*	LONG JStringConvert(struct JConversionCodeSet *jcc,
*	                    UBYTE *in, UBYTE *out,
*	                    LONG incode, LONG outcode,
*	                    LONG length,
*	                    Tag, ...);
*
*   FUNCTION
*	Converts the passed string and places the resulting into the supplied
*	buffer according code set. No more than length bytes are copied
*	into the buffer. if length is -1, function supposes string is continuous.
*
*	case (length == -1)
*	  Function supposes the string is continuous.
*	  If the legal first byte is set in JConversionCodeSet structure,
*	  recognize that byte for first byte in input string.
*	  If the legal escape code is set in JConversionCodeSet structure,
*	  recognize the byte (or bytes) for escape code in input
*	  string.
*	  If the legal single-byte KATAKANA is set in JConversionCodeSet
*	  structure, recognize the byte (or bytes) for single-byte
*	  KATAKANA in input string.
*	  Input string must be NULL terminated.
*
*	case (length != -1)
*	  Function supposes the string is not continuous.
*	  Count with length and check end of string has legal byte.
*	  If the end of string has only the first byte of pair,
*	  function will truncate this first byte.
*
*   EXAMPLE
*	input code    : Shift-JIS
*	input string  : 0x8c 0x4b 0x93 0x63 0x8d 0x81
*	length        : 5
*
*	output code   : EUC
*	output string : 0xb7 0xac 0xc5 0xc4
*	return value  : 4
*
*   INPUTS
*	jcc - pointer to JConversionCodeSet.
*	      if it is NULL, this function does nothing.
*	inbuf - NULL-terminated string to convert
*	outbuf - buffer where to put the converted string
*	incode - input code set you want to convert from
*	outcode - output code set you want to convert to
*	length - maximum number of bytes to deposit in the buffer
*	taglist - pointer to TagItem list.
*
*   TAGS
*	JCT_EUCHanKata -
*		TRUE: when single-byte KATAKANA is found out in inbuf and outocde
*		      is "CTYPE_EUC", convert EUC single-byte KATAKANA
*		FALSE (default): convert to double-byte
*	JCT_SJISHanKata -
*		TRUE: when single-byte KATAKANA is found out in inbuf and outcode
*		      is "CTYPE_SJS", convert Shift-JIS single-byte KATAKANA
*		FALSE (default): convert to double-byte
*	JCT_UDHanKata -
*		character code: 1 byte character you want to convert to
*	        0 (default): convert to double-byte
*	JCT_TextFilter -
*		TRUE: remove 0x0d in case of converting Shift-JIS to EUC
*		      add 0x0d in case of converting EUC to Shift-JIS
*	        FALSE (default): no filtering
*	JCT_SkipESCSeq -
*		TRUE: skip ESC Sequence in JIS conversion
*	        FALSE (default): no skip
*
*   RESULTS
*	number - the number of bytes converted.
*	-1: incode or outcode is not supported.
*
*   SEE ALSO
*
******************************************************************************


******* jcc.library/HanToZen *************************************************
*
*   NAME
*	HanToZen -- convert input (single-byte) string to output (double-byte)
*	            string according code set.
*
*   SYNOPSYS
*	number = HanToZen(jcc, inbuf, outbuf, incode, outcode, length);
*
*	LONG HanToZen(struct JConversionCodeSet *jcc,
*	              UBYTE *in, UBYTE *out,
*	              LONG incode, LONG outcode,
*	              LONG length);
*
*   FUNCTION
*	Converts the passed string and places the resulting into the supplied
*	buffer according code set. if single-byte KATAKANA is found out in
*	input buffer, converts to double-byte KATAKANA. No more than length
*	bytes are copied into the buffer. if length is -1, function supposes
*	string is continuous. Actually, this function is equivalent of
*	JStringConvert with no tags.
*
*   INPUTS
*	jcc - pointer to JConversionCodeSet.
*	      if it is NULL, this function does nothing.
*	inbuf - NULL-terminated string to convert
*	outbuf - buffer where to put the converted string
*	incode - input code set you want to convert from
*	outcode - output code set you want to convert to
*	length - maximum number of bytes to deposit in the buffer
*
*   RESULTS
*	number - the number of bytes converted.
*	-1: incode or outcode is not supported.
*
*   SEE ALSO
*	ZenToHan(), JConvertString()
*
******************************************************************************


******* jcc.library/ZenToHan *************************************************
*
*   NAME
*	ZenToHan -- convert input (double-byte) string to output (single-byte)
*	            string according code set.
*
*   SYNOPSYS
*	number = ZenToHan(jcc, inbuf, outbuf, incode, outcode, length);
*
*	LONG ZenToHan(struct JConversionCodeSet *jcc,
*	              UBYTE *in, UBYTE *out,
*	              LONG incode, LONG outcode,
*	              LONG length);
*
*   FUNCTION
*	Converts the passed string and places the resulting into the supplied
*	buffer according code set. if double-byte KATAKANA is found out in
*	input buffer, converts to single-byte KATAKANA. No more than length
*	bytes are copied into the buffer. if length is -1, function supposes
*	string is continuous.
*
*   INPUTS
*	jcc - pointer to JConversionCodeSet.
*	      if it is NULL, this function does nothing.
*	inbuf - NULL-terminated string to convert
*	outbuf - buffer where to put the converted string
*	incode - input code set you want to convert from
*	outcode - output code set you want to convert to
*	length - maximum number of bytes to deposit in the buffer
*
*   RESULTS
*	number - the number of bytes converted.
*	-1: incode or outcode is not supported.
*
*   SEE ALSO
*	HanToZen(), JConvertString()
*
******************************************************************************


******* jcc.library/EightToSeven *********************************************
*
*   NAME
*	EightToSeven -- convert from 1 chracter (double-byte) Shift-JIS to
*	                1 character (double-byte) JIS.
*
*   SYNOPSYS
*	EightToSeven(p1, p2);
*	             A0  A1
*
*	VOID EightToSeven(ULONG *p1, ULONG *p2);
*
*   FUNCTION
*	Converts from 1 character (double-byte) Shift-JIS specified p1
*	to 1 character (double-byte) JIS specified p2.
*
*   INPUTS
*	p1 - a Shift-JIS input character (double-byte).
*	p2 - a JIS input character (double-byte).
*
******************************************************************************


******* jcc.library/SevenToEight *********************************************
*
*   NAME
*	SevenToEight -- convert from 1 chracter (double-byte) JIS to
*	                1 character (double-byte) Shift-JIS.
*
*   SYNOPSYS
*	SevenToEight(p1, p2);
*	             A0  A1
*
*	VOID SevenToEight(ULONG *p1, ULONG *p2);
*
*   FUNCTION
*	Converts from 1 character (double-byte) JIS specified p1
*	to 1 character (double-byte) Shift-JIS specified p2.
*
*   INPUTS
*	p1 - a JIS input character (double-byte).
*	p2 - a Shift-JIS input character (double-byte).
*
******************************************************************************

