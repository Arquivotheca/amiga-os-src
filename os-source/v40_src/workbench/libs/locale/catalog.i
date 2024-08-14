	IFND	CATALOG_I
CATALOG_I	SET	1

;---------------------------------------------------------------------------

	INCLUDE "exec/types.i"
	INCLUDE "exec/lists.i"
	INCLUDE "locale.i"

;---------------------------------------------------------------------------

   STRUCTURE ExtCatalog,Catalog_SIZEOF
	APTR	ec_Strings
	STRUCT  ec_Language,32
	UWORD  	ec_UsageCnt
   LABEL ExtCatalog_SIZEOF

;---------------------------------------------------------------------------

	ENDC	; CATALOG_I
