/* Prototypes for functions defined in
mouse.c
 */

int acceloMouse(struct InputEvent * ie);

static int accelOne(WORD * );

int freeMouse(void);

int screenMouse(struct Screen * s);

int windowMouse(struct Window * w);

int SetMouseQueue(struct Window * w,
                  int qlen);

