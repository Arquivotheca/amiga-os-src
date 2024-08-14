/* Prototypes for functions defined in
scale.c
 */

int screenToMouse(struct Point * pt,
                  struct Screen * screen);

int screenToLongMouse(struct LongPoint * pt,
                      struct Screen * screen);

int longmouseToScreen(struct LongPoint * pt,
                      struct Screen * screen);

int scaleConversion(struct Point * pt,
                    struct Point res1,
                    struct Point res2);

int scaleScreenMouse(struct Rectangle * rect,
                     struct Screen * screen);

