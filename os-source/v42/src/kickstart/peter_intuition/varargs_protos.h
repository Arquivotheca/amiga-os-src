/* Prototypes for functions defined in
varargs.c
 */

int SetGadgetAttrs(struct Gadget * g,
                   struct Window * w,
                   struct Requester * req,
                   ULONG tag1);

int SetAttrs(Object * o,
             ULONG tag1);

Object * NewObject(Class * cl,
                   ClassID classid,
                   ULONG tag1,
                   int data1);

int NotifyAttrChanges(Object * o,
                      void * ginfo,
                      ULONG flags,
                      ULONG tag1);

int mySetSuperAttrs(Class * cl,
                    Object * o,
                    ULONG t1);

