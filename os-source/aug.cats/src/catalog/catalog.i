	IFND	CATALOG_I
CATALOG_I	SET	1

;---------------------------------------------------------------------------

	INCLUDE "exec/types.i"
	INCLUDE "exec/lists.i"
	INCLUDE "libraries/locale.i"

;---------------------------------------------------------------------------

   STRUCTURE XCatalog,LN_SIZE           ; for internal linkage
        UWORD    ec_Pad                 ; to longword align
        APTR     ec_Language            ; language of the catalog, may be NULL
        ULONG    ec_CodeSet             ; currently always 0
        UWORD    ec_Version             ; version of the catalog
        UWORD    ec_Revision            ; revision of the catalog
	STRUCT   ec_LanguageStr,32
	APTR	 ec_Strings
   LABEL XCatalog_SIZEOF

;---------------------------------------------------------------------------

	ENDC	; CATALOG_I
