/* Prototypes for functions defined in
requester.c
 */

int drawRequester(struct Requester * req,
                  struct Window * w);

static BOOL newDMRequest(struct Window * , struct Requester * );

BOOL ClearDMRequest(struct Window * w);

int SetDMRequest(struct Window * w,
                 struct Requester * req);

BOOL Request(struct Requester * req,
             struct Window * window);

BOOL IRequest(struct Requester * req,
              struct Window * window,
              int isdmr);

BOOL IEndRequest(struct Window * window,
                 struct Requester * req);

int EndRequest(struct Requester * req,
               struct Window * window);

static int pointRel(struct Requester * , struct Window * , int , int );

