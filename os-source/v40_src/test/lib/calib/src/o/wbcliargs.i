/* Prototypes for functions defined in c/WBCliArgs.c */
struct WBCli *WBCliArgs(struct TagItem *array);
char *FilterTemplateVec(char *template);
BOOL OpenAllWBCliLibraries(struct WBCli *wbcli);
void CloseAllWBCliLibraries(struct WBCli *wbcli);
struct WBCli *WBCliArgsTags(unsigned long Tag, ...);
struct WBCGui *AddGuiEntry(struct WBCli *wbcli);
long ParseWBCliTemplate(struct WBCli *wbcli,
                        char *template,
                        ULONG *args);
void FreeWBCli(struct WBCli *wbcli);
