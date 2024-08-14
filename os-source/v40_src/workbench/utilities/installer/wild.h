/* return values from wild.asm */

#define PATTERN_GOOD	-1					/* Success Return Code */
#define PATTERN_BAD		0					/* Failure Return Code */
#define MATCH_FOUND		0					/* Success Return Code */
#define MATCH_FAILED	1					/* Normal Failure RetCode */
#define MATCH_NO_MEM	2					/* Abnormal Failure RetCode */

#ifdef AZTEC_C
LONG PatternValid(char *Pattern);
LONG MatchADOS(char *String, char *Pattern);
#endif

#define MATCH_FILES_ONLY		0
#define MATCH_DIRS_ONLY			1
#define MATCH_FILES_DIRS		2
#define MATCH_FREE_INFOS		0x0040
#define UNMATCH_INFO_FILES		0x0080
#define UNMATCH_FONT_FILES		0x0100

#define IS_DEVICE				0
#define IS_DRAWER				1
#define IS_FILE					2
#define IS_ASSIGN				3
