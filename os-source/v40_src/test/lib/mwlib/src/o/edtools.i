/* Prototypes for functions defined in c/EdTools.c */
struct RPLineEd *LEInit(struct Window *win,
                        struct TextFont *font,
                        void (*exfun)(void),
                        void (*helpfun)(void));
void LEDInit(struct RPLineEd *r);
void _disp_cur(struct RPLineEd *r);
BOOL _is_cur_visible(struct RPLineEd *r);
char *_finish_up(struct RPLineEd *r);
void _raw_process(struct RPLineEd *r);
void _vanilla_process(struct RPLineEd *r);
char *LineEd(struct RPLineEd *r,
             short x,
             short y,
             char *str,
             short chpos);
