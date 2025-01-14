/* action.c */
void write_log_file_icon(char *path);
short create_drawer_icon(char *path);
BPTR LockDir(char *name, long mode);
short create_dir(char *path, short icon);
short modify_icon(char *name, struct ToolType *ntt, char *deftool, int stack, short nopos,short swap);
short CopyFileWithStatus(BPTR sfh, BPTR dfh, char *sname, BPTR dlock, long size);
short CopyFile(BPTR sfh, BPTR dfh);
short copy_file(char *filename, char *dirname, char *newname, short nopos);
int copy_tree_file(BPTR src, BPTR dest, struct FileInfoBlock *fib);
int copy_tree_dir(BPTR src, BPTR dest);
int copy_tree(BPTR src, char *dest);
int copy_object(char *src, char *dir, char *name, short nopos);
int execute_command(char *command);
int send_rexx(char *str);
short GetAssign(char *name, char *buffer, long opts);
short add_assign(struct DosInfo *dinfo, char *name, BPTR lock);
short SetAssign(char *name, BPTR flock);
BPTR lock_object(struct DeviceNode *dnode);
BOOL FindAssign(char *old, long opts, BPTR *lock);
long MatchAssign(char *new, char *old, long opts);
long VolumeMounted(char *name,long types);
long ChangeFileDate (char *name,struct DateStamp *ds);
int RelabelDisk(char *old,char *new);

/* colors.c */
short ColorDifference(unsigned short rgb0, unsigned short rgb1);
short ColorLevel(unsigned short rgb);
void assign_colors(void);

/* compile.c */
struct String *MakeString(char *text, short length);
void FreeString(struct String *s);
void sym_delete_hook(struct GSymbolScope *scope, struct Value *value);
int init_symbols(void);
void cleanup_symbols(void);
struct TreeNode *compile(char *filename);
void parse_list(struct TreeNode *parent);
void de_compile(struct TreeNode *node);
void fatal(char *message);
void prefatal(char *message);
void offending_line(short n);

/* derr.c */
char *DosErrorText(long num);

/* files.c */
short get_entry_type(struct String *str);
long get_dos_type(struct String *str);
long get_entry_size(struct String *str);
LONG astcsma(char *str, char *pat);
struct String **match_files(BPTR lock, char *pat, short mode, long *total);
void free_matches(struct String **str);
short is_disk(struct DeviceNode *dnode);
struct DosList *LockDevList(void);
struct DosList *FindDevEntry(struct DosList *dnode, char *name, LONG type);
struct DosList *NextDevEntry(struct DosList *dnode, LONG type);
void UnLockDevList(void);
short check_node(struct MinList *list, struct DeviceNode *dnode, long *count);
struct String **get_drives(long *total);
long GetDiskSpace(char *name);
struct String *find_largest_partition(void);
struct DeviceNode *find_device_for_task(struct MsgPort *mp);
BPTR boot_volume(void);

/* interp.c */
struct String *TempString(char *text, short length, struct Value *v);
void doroot(struct TreeNode *head);
void interpret(struct TreeNode *head);
void eval(struct TreeNode *head);
int test(struct TreeNode *head);
void vfree(struct Value *v);
void pop(void);
int integer_to_string(struct Value *v);
void dupstring(struct Value *v);
void doseq(struct TreeNode *head);
void doseqn(struct TreeNode *head);
void doif(struct TreeNode *head);
void doselect(struct TreeNode *head);
void dowhile(struct TreeNode *head);
void dountil(struct TreeNode *head);
struct Value *each_string(struct TreeNode *head);
void do_foreach(struct TreeNode *head);
void dotrap(struct TreeNode *head);
void doset(struct TreeNode *head);
void doproc(struct TreeNode *head);
void do_onerror(struct TreeNode *head);
int verify_string(struct Value *v, char *func);
int scmp(struct Value *v1, struct Value *v2);
void do_cat_sub(struct Value *vbase, short args, char *between, char *caller);
void do_cat(struct Value *vbase, short args);
void do_tackon(struct Value *vbase, short args);
void do_fileonly(struct Value *vbase, short args);
void do_pathonly(struct Value *vbase, short args);
void do_exists(struct Value *vbase, short args);
short compare_dates(struct DateStamp *date1, struct DateStamp *date2);
void do_earlier(struct Value *vbase, short args);
void do_getsize(struct Value *vbase, short args);
void do_getdiskspace(struct Value *vbase, short args);
short GetChecksum(char *name, long *sum);
void do_getsum(struct Value *vbase, short args);
long GetFileVersion(char *name);
void do_getversion(struct Value *vbase, short args);
void do_expandpath(struct Value *vbase, short args);
void do_database(struct Value *vbase, short args);
void do_getassign(struct Value *vbase, short args);
void do_makeassign(struct Value *vbase, short args);
void do_getdevice(struct Value *vbase, short args);
void do_protect(struct Value *vbase, short args);
void do_user(struct Value *vbase, short args);
void do_getenv(struct Value *vbase, short args);
void do_select(struct Value *vbase, short args);
void do_compare(struct Value *vbase, short args, short type);
void do_equal(struct Value *vbase, short args);
void do_unequal(struct Value *vbase, short args);
void do_greater(struct Value *vbase, short args);
void do_greater_equal(struct Value *vbase, short args);
void do_lesser(struct Value *vbase, short args);
void do_lesser_equal(struct Value *vbase, short args);
void do_minus(struct Value *vbase, short args);
void do_mathfunc(struct Value *vbase, short args, long result, short functype);
void do_in(struct Value *vbase, short args);
void do_plus(struct Value *vbase, short args);
void do_mul(struct Value *vbase, short args);
void do_div(struct Value *vbase, short args);
void do_and(struct Value *vbase, short args);
void do_or(struct Value *vbase, short args);
void do_xor(struct Value *vbase, short args);
void do_not(struct Value *vbase, short args);
void do_bitand(struct Value *vbase, WORD args);
void do_bitor(struct Value *vbase, WORD args);
void do_bitxor(struct Value *vbase, WORD args);
void do_bitnot(struct Value *vbase, WORD args);
void do_shiftleft(struct Value *vbase, WORD args);
void do_shiftright(struct Value *vbase, WORD args);
void do_substr(struct Value *vbase, WORD args);
void do_strlen(struct Value *vbase, WORD args);
void do_patmatch(struct Value *vbase, WORD args);
void do_debug(struct Value *vbase, short args);
void memerr(void);
int aborterr(char *name);
void err(struct TreeNode *node, short code, char *msg, ...);
void err_msg(struct TreeNode *node, short code, char *sub, char *msg, ...);
int argcheck(short argcount, short min, short max, char *name);
void transcript(char *string, ...);
void transcript_wrap(char *string, ...);

#ifdef DEBUG
void debug_action(char *act);
#endif

/* layout.c */
void make_images(void);
void restart_layout(void);
void start_layout(void);
void center_layout(short a);
void DrawBevel(struct RastPort *rp, short x, short y, short w, short h, short in);
void DrawBevelRect(struct RastPort *rp, struct Rectangle *box, short in);
void write_text(struct RastPort *rp,struct Gadget *gad, char *text);
void new_border_button(struct Button *gad, unsigned short id, char *text, struct Gadget **list);
void arrow_gadget(struct Gadget **list, unsigned short id, short x, short y, short type);
void slider_gadget(struct Gadget **list, unsigned short id, short x, short y, short width, short height);
void list_gadget(struct Gadget **list, unsigned short id);
void layout_button(struct Gadget **list, char *text, unsigned short id, short lower_flag, short width_type);
void layout_2_buttons(struct Gadget **list, char *text1, unsigned short id1, char *text2, unsigned short id2, short lower_flag);
void layout_3_buttons(struct Gadget **list, char *text1, unsigned short id1, char *text2, unsigned short id2, char *text3, unsigned short id3, short lower_flag);
void layout_side_buttons(struct Gadget **list, char *text1, unsigned short id1, char *text2, unsigned short id2, short lower_flag);
void finish_textzone(struct TextZone *gad, char *text, unsigned short id);
void std_text_gad(struct TextZone *gad, short lower_flag);
void set_integer_gadget(struct Gadget *gad, LONG val);
void layout_gauge(struct Gadget **list, unsigned short id);
void layout_strgad(struct Gadget **list, char *text, unsigned short id, short lower_flag);
void layout_intgad(struct Gadget **list, long val, unsigned short id, short lower_flag);
void layout_text_length(char *text, short length, short lower_flag, short style_type, unsigned long special);
void layout_text(char *text, short lower_flag, short style_type, unsigned short special);
void init_wrap(void);
short find_wrap(char **textptr, short pixels);
void layout_wrap_text(char *text, short lower_flag, short style_type);
void erase_page(void);
void ghost_gadget(unsigned short id);
void enable_gadget(unsigned short id);
short TextPixels(char *text);
void build_checkmark(struct Gadget *gad, unsigned short id);
void build_radio(struct Gadget *gad, unsigned short id);
short layout_box_gads(struct Gadget **list, struct Rectangle *box, struct String **text, short count, short id, short type);
void disable_radio(struct Gadget *gad, short sel, short total);
void default_radio(struct Gadget *gad, short sel, short total);
short final_radio(struct Gadget *gad, short total);
void init_checkboxes(struct Gadget *gad, long mask, short total);
long final_checkboxes(struct Gadget *gad, short total);
void radio_space_check(short count);
struct Gadget *find_key_gadget(UWORD code);
long make_gadgets(struct Gadget **gad, struct GadgetDef *def, struct InstallationRecord *ir, unsigned short mode);
void clip_off(struct RastPort *rp);
void clip_on(struct RastPort *rp, short x1, short y1, short x2, short y2);

/* scan.c */
int scan_scripts(char *start_file, int nestable);

/* start.c */
void edit_startup(char *appname);

/* statement.c */
void cleanup_irecord(void);
void popall(struct Value *vbase);
void do_string_param(struct Value *v, short args, struct Value *d, char *funcname, long flags);
int help_missing(char *me, short confirmed);
void do_askfile(struct Value *vbase, short args);
void do_askdir(struct Value *vbase, short args);
void do_askstring(struct Value *vbase, short args);
void do_asknumber(struct Value *vbase, short args);
void do_askchoice(struct Value *vbase, short args);
void do_askoptions(struct Value *vbase, short args);
void do_askbool(struct Value *vbase, short args);
void do_askdisk(struct Value *vbase, short args);
void do_newdir(struct Value *vbase, short args);
void do_files(struct Value *vbase, short args);
void do_libs(struct Value *vbase, short args);
void do_startup(struct Value *vbase, short args);
void do_tooltype(struct Value *vbase, short args);
void do_textfile(struct Value *vbase, short args);
void do_execute(struct Value *vbase, short args);
void do_run(struct Value *vbase, short args);
void do_rexx(struct Value *vbase, short args);
void do_rename(struct Value *vbase, short args);
void do_delete(struct Value *vbase, short args);
void do_abort(struct Value *vbase, short args);
void do_exit(struct Value *vbase,short args);
void do_complete(struct Value *vbase, short args);
void do_message(struct Value *vbase, short args);
void do_working(struct Value *vbase, short args);
void do_welcome(struct Value *vbase, short args);
void do_merged_params(struct Value *vbase, short args, struct Value *d, char *text);
void do_help(struct Value *vbase, short args);
void do_prompt(struct Value *vbase, short args);
void do_trans(struct Value *vbase, short args);
void do_choices(struct Value *vbase, short args);
void do_defaulttool(struct Value *vbase, short args);
void do_pattern(struct Value *vbase, short args);
void do_source(struct Value *vbase, short args);
void do_dest(struct Value *vbase, short args);
void do_newname(struct Value *vbase, short args);
void do_filesonly(struct Value *vbase, short args);
void do_default(struct Value *vbase, short args);
void do_all(struct Value *vbase, short args);
void do_infos(struct Value *vbase, short args);
void do_fonts(struct Value *vbase, short args);
void do_confirm(struct Value *vbase, short args);
void do_safe(struct Value *vbase, short args);
void do_nogauge(struct Value *vbase, short args);
void do_optional(struct Value *vbase, short args);
void do_delopts(struct Value *vbase, short args);
void do_resident(struct Value *vbase, short args);
void do_range(struct Value *vbase, short args);
void do_setstack(struct Value *vbase, short args);
void do_noposition(struct Value *vbase, short args);
void do_settooltype(struct Value *vbase, short args);
void do_disk(struct Value *vbase, short args);
struct TextList *new_text(char *text, long length, short flag);
void free_text(struct TextList *tl);
void do_append(struct Value *vbase, short args);
void do_include(struct Value *vbase, short args);

/* support.c */
struct Gadget *FindGadgetB(struct Gadget *gad, short num);
struct Gadget *FindGadget(struct Window *win, short num);
void EmptyPort(struct MsgPort *port);
void SelectGadget(struct Gadget *gad, struct Window *win, unsigned short state);
struct String *new_string(char *text);
struct String *empty_string(LONG size);
short CheckToolValue(struct DiskObject *dobj, char *type, char *value);
char *LocateToolType(struct DiskObject *dobj, char *type);
struct Library *OpenConsoleLib(void);
void CloseConsoleLib(void);
UWORD DoKeyConvert(struct IntuiMessage *imsg);

/* symtab.c */
struct GSymbol *FindGSymbol(struct GSymbolScope *scope, char *name, short namelength, struct GSymbol ***headptr);
struct GSymbol *AddGSymbol(struct GSymbolScope *scope, char *name, short namelength, struct GSymbol **headptr);
void DeleteGScope(struct GSymbolScope *scope);

/* window.c */
void calc_window_size(struct TextFont *tf);
void assign_text(void);
void main(int argc, char **argv);
short welcome_page(struct InstallationRecord *ir);
short copylogfile(char *src, char *dest);
char *close_transcript(void);
void set_gadgets(struct Gadget *gad);
void disable_gadgets(struct Gadget *gad);
void remove_gadgets(struct Gadget *gad);
void clear_gadgets(struct Gadget *gad);
struct Gadget *HandleEvents(struct ListControl *ctrl);
short check_escape(void);
int save_page(struct InstallationRecord *ir,char *title,
	int (*func)(struct InstallationRecord *,char *));
int do_insert_disk(struct InstallationRecord *ir, char *title);
int do_nothing(struct InstallationRecord *ir, char *title, long mask);
int do_textlist(struct InstallationRecord *ir, char *title);
short abortcopy_page(short *counts);
struct Gadget *abortcopy_display(char *src, char *dest);
void abortcopy_clear(short *counts);
int makedir_page(struct InstallationRecord *ir, char *newdir);
int askdelete_page(struct InstallationRecord *ir,char *file);
short setup_list(struct ListControl *ctrl, char *path, short repl);
int do_drawers(struct InstallationRecord *ir, char *title);
int show_filelist(struct InstallationRecord *ir, char *title);
struct String *copy_page(struct InstallationRecord *ir);
int radio_page(struct InstallationRecord *ir);
int checkbox_page(struct InstallationRecord *ir);
char *askstring_page(struct InstallationRecord *ir);
long asknumber_page(struct InstallationRecord *ir);
long yesno_page(struct InstallationRecord *ir);
long askdisk_page(struct InstallationRecord *ir);
void display_help_page(char *text, short lines);
void erase_help_page(void);
int help_page(struct InstallationRecord *ir, char *title);
int about_page(struct InstallationRecord *ir, char *title);
int error_page(struct InstallationRecord *ir, char *title);
char *select_dir(struct InstallationRecord *ir);
char *select_file(struct InstallationRecord *ir);
void message_page(struct InstallationRecord *ir);
void final_page(void);
int working_page(struct InstallationRecord *ir);
int quick_working_page(void);
int confirm_page(struct InstallationRecord *ir);
int version_page(struct InstallationRecord *ir, long old, long new);
int get_user(struct InstallationRecord *ir);
int get_options(struct InstallationRecord *ir);
short change_startup(char **file, int *offset);
void set_slider(struct Gadget *gad, struct ListControl *ctrl);
void hilite_file(struct Gadget *gad, struct ListControl *ctrl);
void show_files(struct Gadget *gad, struct ListControl *ctrl);
long which_file(struct Gadget *gad, struct ListControl *list, short my);
void set_zone(struct TextZone *gad, char *text);
void expand_zone(struct TextZone *gad, BPTR lock);
void init_text(struct Gadget *gad, struct ListControl *ctrl);
void show_text(struct Gadget *gad, struct ListControl *ctrl, short mode);
void erase_text(struct Gadget *gad, struct ListControl *ctrl);
long which_line(struct Gadget *gad, struct ListControl *list, short my);
long final_text(struct Gadget *gad, struct ListControl *ctrl);
int report_message(short mode, char *text, ...);
void simple_message(char *text, ...);
char *has_special(void);
int fatal_page(struct InstallationRecord *ir, char *title);
void show_error(short mode, char *text);
int ask_abort(void);
void set_busy(void);
void clear_busy(void);
void completion(long percent);
void auto_err(char *msg);

/* text.c */
#ifdef AZTEC_C
char *GetLocalText(WORD num);
#pragma regcall( GetLocalText(d0) )
#else
/* char __asm *GetLocalText(register __d0 WORD num); */
char *GetLocalText(WORD num);
#endif
void localize_text(void);
