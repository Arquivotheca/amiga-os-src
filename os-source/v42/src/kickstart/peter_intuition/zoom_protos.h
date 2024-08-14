/* Prototypes for functions defined in
zoom.c
 */

int ZipWindow(struct Window * window);

int IZoomWindow(struct Window * window,
                BOOL interactive);

int IChangeWindow(struct Window * w,
                  struct IBox * newbox,
                  int subcommand);

int IWindowDepth(struct Window * w,
                 int whichway,
                 struct Window * behindwindow);

int establishReqLayer3(struct Requester * req,
                       struct IBox * windowbox,
                       struct Point * gzzdims);

