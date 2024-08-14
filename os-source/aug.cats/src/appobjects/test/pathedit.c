enum
{
    Dummy = APSH_USER_ID,

    OpenTraceID,
    TraceWinOpenID,
    TraceWinCloseID,
    SelectPathID,
    AddPathID,
    DeletePathID,
    ClearPathID,
    ClonePathID,
    OKPathID,
    CancelPathID,

    LAST_ID
};

STRPTR Trace_deftext[] =
{
    "",
    "RipStruct Trace",
    "_Delete",
    "_Add",
    "C_lone...",
    "_OK",
    "_Cancel",
    "Cl_ear",
    NULL
};

/* Objects may use TagItems to describe additional attributes.  The attributes
 * in this case are defined by GadTools.  */
struct TagItem select_tags[] =
{
    {GTST_MaxChars, 512L},
    {TAG_DONE,},
};

/* Because we don't no the gadget address for the text gadget to assign to
 * the list, we provide the name assigned to the object instead.  We also
 * provide a function ID to call when the gadget is selected when either
 * ALT key is pressed. */
struct TagItem list_tags[] =
{
    {APSH_ShowSelected, (ULONG) "PathEntry"},
    {TAG_DONE,},
};

struct Object Trace[] =
{
    {&Trace[1], 0, 0, OBJ_Window, NULL, APSH_OBJF_NOADJUST, NULL, "Trace", 1L,
     {0, 0, 0, 0}, },

    {&Trace[2], 0, 0, OBJ_String, SelectID, NULL, 'p', "PathEntry", NULL,
     {  3,   0,186, 14}, select_tags,},
    {&Trace[3], 0, 0, OBJ_Listview, SelectPathID, NULL, NULL, "PathList", NULL,
     {  3,   3,186,115}, list_tags,},

    {&Trace[4], 0, 0, OBJ_Button, AddPathID, NULL, 'A', "Add", 3L,
     {194,   3, 80, 15}, },
    {&Trace[5], 0, 0, OBJ_Button, DeletePathID, NULL, 'D', "Delete", 2L,
     {194,  19, 80, 15}, },
    {&Trace[6], 0, 0, OBJ_Button, ClearPathID, NULL, 'E', "Clear", 7L,
     {194,  35, 80, 15}, },
    {&Trace[7], 0, 0, OBJ_Button, ClonePathID, NULL, 'L', "Clone", 4L,
     {194,  51, 80, 15}, },

    {&Trace[8], 0, 0, OBJ_Button, OKPathID, APSH_OBJF_CLOSEWINDOW, 'O', "OK", 5L,
     {  3, 121, 80, 15}, },
    {NULL,      0, 0, OBJ_Button, CancelPathID, APSH_OBJF_CLOSEWINDOW, 'C', "Cancel", 6L,
     {194, 121, 80, 15}, },
};
