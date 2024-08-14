extern struct IntuiText IText32;	// Partitioning
extern struct IntuiText IText92;
extern struct IntuiText IText97;
extern struct IntuiText IText102;
extern struct IntuiText IText110;	// Bad Blocks
extern struct IntuiText IText113;
extern struct IntuiText IText136;

#define IntuiTextList1  (NULL)		// Preparations...
#define IntuiTextList2  IText32		// Partitioning
#define IntuiTextList3  (NULL)		// File System Characteristics
#define IntuiTextList4  (NULL)		// File System Maintanance
#define IntuiTextList5  (NULL)		// Define a New drive
#define IntuiTextList6  IText92
#define IntuiTextList7  IText97
#define IntuiTextList9  IText102
#define IntuiTextList10 (NULL)		// Bad Blocks
#define IntuiTextList11 IText113
#define IntuiTextList12 (NULL)		// Set drive Type
#define IntuiTextList13 IText136

extern struct IntuiText IText32;	/* Address    , LUN in handlepart.c */
extern struct IntuiText IText141;	/* Verify stuff 1234567 */
extern struct IntuiText IText142;	/* Verify stuff 1234567 */
extern struct IntuiText IText143;	/* Verify stuff 1234567 */
extern struct IntuiText IText144;	/* Verify stuff 1234567890 */
extern struct IntuiText NotifyIText;	// Notify requester stuff

#define PartAddrIText	IText32
#define VerifyCyl	IText141
#define VerifyHead	IText142
#define VerifySec	IText143
#define VerifyBlock	IText144

extern struct NewWindow NewWindowStructure1;
extern struct NewWindow NewWindowStructure2;
extern struct NewWindow NewWindowStructure3;
extern struct NewWindow NewWindowStructure4;
extern struct NewWindow NewWindowStructure5;
extern struct Requester RequesterStructure6;
extern struct Requester RequesterStructure7;
extern struct Requester RequesterStructure77;	// 39.9
extern struct Requester RequesterStructure8;
extern struct Requester RequesterStructure9;
extern struct NewWindow NewWindowStructure10;
extern struct Requester RequesterStructure11;
extern struct NewWindow NewWindowStructure12;
extern struct Requester RequesterStructure13;
extern struct Requester NotifyRequester;

#define PrepWindow		NewWindowStructure1
#define PartWindow		NewWindowStructure2
#define FileWindow		NewWindowStructure3
#define FlSMWindow		NewWindowStructure4
#define NewDWindow		NewWindowStructure5
#define LLFormatRequester	RequesterStructure6
#define FileSysTypeRequester	RequesterStructure7
#define FileSysNameRequester	RequesterStructure77	// 39.9
#define AskSure_Requester	RequesterStructure8
#define PleaseWait_Requester	RequesterStructure9
#define BadWindow		NewWindowStructure10
#define BadRequester		RequesterStructure11
#define TypeWindow		NewWindowStructure12
#define MapOutRequester		RequesterStructure13
