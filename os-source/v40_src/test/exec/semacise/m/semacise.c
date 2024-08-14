/****** checkpoint.test/Semacise **********************************************

    NAME
        Semacise -- execercise semaphore / task priority handling

    AUTHOR
        Fred Mitchell, Product Assurance, Commodore-Amiga, Inc.
        John J. Szucs, Systems Assurance, Commodore International Services Company

    VERSION
        1.00    06 Mar 1991 - Inception
        2.00    24 Jun 1992 - Procure/Vacate support added; documentation
                              changes (jjszucs)

    SYNOPSIS
        Semacise

    FUNCTION
        Semacise tests the handling of semaphore/task scheduling
        arbitration. It creates 7 tasks of priorities 0-6, while
        boosting the priority of the parent process to 7. The
        parent (master) process will then issue orders to his
        children to perform certain operations. The operations are:

            0) No-operation                               (.)
            1) Wait for next command                      (W)
            2) BusyWait                                   (U)
            3) Obtain a specified semaphore Shared        (Sx)
            4) Obtain a specified semaphore Exclusive     (Ex)
            5) Attempt to obtain a specified semaphore    (Ax)
            6) Release a specified semaphore              (Rx)
            7) Procure a specified semaphore Exclusive    (Pxy)
               on bid message y
            8) Procure a specified semaphore Shared       (pxy)
               on bid message y
            9) Hold on specified semaphore (after P or p) (Hxy)
               on bid message y
            10) Vacate a specified semaphore              (Vxy)
                on bid message y
            11) Return to beginning of string             (<)

            (There are 26 semaphores, labeled a thru z)

        The master process will continually send these commands
        to his children as instructed by the user.

        When entering new commands, you must first [RESET], then
        type in your commands. If you do not place the
        return-to-beginning command (<) at the end of your string,
        then the messages will stop there for that child task.

        It is not a good idea to modify commands with everything
        running, espcially in the case of semaphore operations.
        Issue a [HALT] first before modifying anything.

        Due to the nature of this test, it is REAL EASY to crash
        your machine, or get it hung up in continuous recoverable
        alerts. For this reason, have NOTHING ELSE running that
        you can't afford to lose. You have been warned.

    CONTROL
        [START] - Start-up the command stream
        [HALT]  - Pause the command stream
        [STEP]  - Single-step all of the tasks.
        [RESET] - Do a [HALT] and reset string pointers back to beginning
        [CLEAR] - Do a [RESET] and clear all strings
        [HELP]  - Open up the help file (Semacise.doc) and send it to
                  a console window.
        [EXIT]  - Close down system, kill all children, and exit.
                  WARN - if there are outstanding semaphores when this
                  is executed, you might crash the system or get an
                  endless stream of recoverable alerts. Be sure all
                  semaphores are resolved BEFORE doing an exit.
        [TASKn] - Single-step this task through the string. Normally,
                  you would only do this during a [HALT] or [RESET].
                  The message indicator may or may not update depending
                  on whether or not the task is already waiting for
                  a semaphore. In that case, the command will simply
                  pile up in the child's port.

    STATUS INDICATORS
        Status indication is performed by the child tasks. There are
        two types - the Message Indicator (the large recessed box)
        that children display what message they've just received, and
        the Busy Box that the child renders a continually changing
        pattern to indicate that the child is executing.

    NOTES
        Children do not wait for a reply. If one comes back, they will
        quietly deallocate it.

    EXAMPLE
        A typical case that has been discussed in amiga.software:

            TASK1   SaSbRaRb<
            TASK2   ..U<
            TASK3   EaEbRaRb<

    EXAMPLE
        A Deadlock situation (warning - this might kill your machine):
            TASK1   EaEbRaRb<
            TASK2   EbEaRbRa<

******************************************************************************/

#include "Semacise.h"
#include <stdio.h>
#include "mo/Semacise.i"
#include "rev.h"

static char *verstag = VERSTAG;

struct Library *IntuitionBase;
struct Library *GfxBase;
struct Library *GadToolsBase;

// The Main of all

void main()
{
    struct GlobalEnvironment *ge;

    if (IntuitionBase = OpenLibrary("intuition.library", 36))
    {
        if (GfxBase = OpenLibrary("graphics.library", 36))
        {
            if (GadToolsBase = OpenLibrary("gadtools.library", 36))
            {
                if (ge = calloc(1, sizeof(*ge)))
                {
                    if (ge->win = OpenWindowTags(NULL,  WA_Left, 0,
                                                        WA_Top, 0,
                                                        WA_InnerWidth, 664,
                                                        WA_InnerHeight, 280,
                                                        WA_SmartRefresh, TRUE,
                                                        WA_DragBar, TRUE,
                                                        WA_DepthGadget, TRUE,
                                                        WA_CloseGadget, TRUE,
                                                        WA_Activate, TRUE,
                                                        WA_RMBTrap, TRUE,
                                                        WA_NoCareRefresh, TRUE,
                                                        WA_GimmeZeroZero, TRUE,
                                                        WA_ScreenTitle, "Semacise Semaphore Execerciser by Fred Mitchell and John J. Szucs",
                                                        WA_Title, "Semacise.",
                                                        WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_GADGETUP | IDCMP_GADGETDOWN | IDCMP_INTUITICKS,
                                                        TAG_DONE))
                    {
                        // Setup the environment
                        UnlockPubScreen(NULL, ge->scr = LockPubScreen(NULL));
                        ge->visual_info = GetVisualInfoA(ge->scr, NULL);

                        ge->ta.ta_Name = SEMA_FONT;
                        ge->ta.ta_YSize = SEMA_YSIZE;
                        ge->ta.ta_Style = FS_NORMAL;
                        ge->ta.ta_Flags = NULL;

                        if (CreateInterface(ge))
                        {
                            DoInterface(ge);

                            DestroyInterface(ge);
                        }
                        else
                            printf("CreateInterface() failed\n");

                        FreeVisualInfo(ge->visual_info);
                        CloseWindow(ge->win);
                    }
                    free(ge);
                }
                else
                    printf("Out of Memory\n");

                CloseLibrary(GadToolsBase);
            }
            else
                printf("gadtools.library failed to open.\n");

            CloseLibrary(GfxBase);
        }
        else
            printf("graphics.library failed to open.\n");

        CloseLibrary(IntuitionBase);
    }
    else
        printf("intuition.library failed to open.\n");
}

// Messages

struct SemaMessage *CreateSemaMessage(struct MsgPort *ReplyToMe)
{
    struct SemaMessage *m;

    if (m = AllocVec(sizeof(*m), MEMF_CLEAR | MEMF_PUBLIC))
    {
        m->m.mn_Node.ln_Type = NT_MESSAGE;
        m->m.mn_Node.ln_Pri = 0;
        m->m.mn_Node.ln_Name = "SemaMessage";
        m->m.mn_ReplyPort = ReplyToMe;
        m->m.mn_Length = sizeof(*m);
        m->task = FindTask(NULL);
    }
    return m;
}

void DoSemaMessage(struct GlobalEnvironment *ge, ULONG tasknum, enum Command cmd, void *arg, long index)
{
    struct SemaMessage *sm;

    if (sm = CreateSemaMessage(ge->master_port))
    {
        sm->cmd = cmd;
        sm->arg = arg;
        sm->index = index;
        PutMsg(ge->child_ports[tasknum], sm);
    }
}

void DestroySemaMessage(struct SemaMessage *m) // allows for NULLs
{
    if (m)
        FreeVec(m);
}

// Child task

__saveds void Child()
{
    struct Task *task;

    ULONG tasknum = -1; // our task number (and hence our priority)
    struct RastPort *rp = NULL;
    ULONG cnt = 0;      // counter for our rendering
    long sem_cnt = 0;   // semaphore count

    struct MsgPort *port = NULL, *parent_port = NULL;
    struct SemaMessage *mess, *sm;
    enum Command cmd = sc_wait, wait_cmd = sc_wait;
    void *arg;
    long index;
    BOOL done = FALSE;
    BOOL holding; // Hold state (TRUE for hold, FALSE for release)

    struct SemaphoreMessage semaphoreMessage[NUMBER_OF_BIDS];
    struct SemaphoreMessage *thisSemaphoreMessage, *nextSemaphoreMessage;
    struct MsgPort *semaphorePort;

    // Get thy name, so we can use it as a port name
    task = FindTask(NULL);
    parent_port = FindPort(MASTER_PORT);

    if (semaphorePort = CreatePort(NULL,0)) {

        if (port = CreatePort(task->tc_Node.ln_Name, 0))
        {
            //// A chat with my parent
            // Register myself with my parent
            if (sm = CreateSemaMessage(port))
            {
                sm->cmd = sm_my_port;
                sm->arg = port;
                PutMsg(parent_port, sm);
            }

            // Get a RastPort to render in!
            if (sm = CreateSemaMessage(port))
            {
                sm->cmd = sm_what_rport;
                PutMsg(parent_port, sm);
            }

            // Get my task number!
            if (sm = CreateSemaMessage(port))
            {
                sm->cmd = sm_what_mytasknum;
                PutMsg(parent_port, sm);
            }

            while (!done)
            {
                if (rp && tasknum != -1)
                    ChildRender(rp, tasknum, ++cnt);

#ifdef DEBUG
                kprintf("Task:%lx, Port = %lx, rp=%lx, cnt=%ld, sem_cnt=%ld\n", task, port, rp, cnt, sem_cnt);
#endif // DEBUG

                switch(cmd) // State Machine
                {
                case sc_noop:
                    cmd = wait_cmd;
                    break;

                case sc_get_message:
                    if (mess = GetMsg(port))
                    {
                        if (mess->cmd < sc_count) // command for us?
                        {
                            cmd = mess->cmd;
                            arg = mess->arg;
                            index = mess->index;
                            ReplyMsg(mess);
                            DisplayMessage(rp, tasknum, cmd, index);
                        }
                        else
                            switch(mess->cmd) // Reply to one of our messages
                            {
                            case sm_my_port:         // child register of port
                                DestroySemaMessage(mess);
                                break;

                            case sm_what_mytasknum:  // child requesting task number
                                tasknum = mess->arg;
                                DestroySemaMessage(mess);
                                break;

                            case sm_what_rport:      // child requesting rastport
                                rp = mess->arg;
                                DestroySemaMessage(mess);
                                break;

                            default: // no useful information
                                DestroySemaMessage(mess);
                                break;
                            }
                    }
                    else
                        cmd = wait_cmd;
                    break;

                case sc_wait:
                    WaitPort(port);
                    wait_cmd = sc_wait;
                    cmd = sc_get_message;
                    break;

                case sc_busy:
                    cmd = wait_cmd = sc_get_message;
                    break;

                case sc_grab_exclusive: // (arg should point to semaphore)
                    sem_cnt++;
                    ObtainSemaphore(arg);
                    cmd = wait_cmd;
                    break;

                case sc_grab_shared: // (arg should point to semaphore)
                    sem_cnt++;
                    ObtainSemaphoreShared(arg);
                    cmd = wait_cmd;
                    break;

                case sc_release: // (arg should point to semaphore)
                    sem_cnt--;
                    ReleaseSemaphore(arg);
                    cmd = wait_cmd;
                    break;

                case sc_procure_exclusive: // (arg = semaphore, index = bid)

                    // Debugging
#ifdef DEBUG
                    kprintf("Child: Exclusive procure of semaphore $%08lx on bid message %ld\n",arg,index);
#endif // DEBUG

                    // Initialize bid message
                    semaphoreMessage[index].ssm_Message.mn_ReplyPort=semaphorePort;
                    semaphoreMessage[index].ssm_Message.mn_Length=sizeof(struct SemaphoreMessage);

                    // Exclusive bid
                    semaphoreMessage[index].ssm_Message.mn_Node.ln_Name=(UBYTE *) 0;

                    // Bid for semaphore
                    Procure(arg,&(semaphoreMessage[index]));

                    // Wait for command
                    cmd = wait_cmd;

                    break;

                case sc_procure_shared: // (arg = semaphore, index = bid)

                    // Debugging
#ifdef DEBUG
                    kprintf("Child: Shared procure of semaphore $%08lx on bid message %ld\n",arg,index);
#endif // DEBUG

                    // Initialize bid message
                    semaphoreMessage[index].ssm_Message.mn_ReplyPort=semaphorePort;
                    semaphoreMessage[index].ssm_Message.mn_Length=sizeof(struct SemaphoreMessage);

                    // Shared bid
                    semaphoreMessage[index].ssm_Message.mn_Node.ln_Name=(UBYTE *) 1;

                    // Bid for semaphore
                    Procure(arg,&(semaphoreMessage[index]));

                    // Wait for command
                    cmd = wait_cmd;

                    break;

                case sc_hold: // (arg = semaphore, index = bid)

                    // Debugging
#ifdef DEBUG
                    kprintf("Child: Hold on semaphore $%08lx on bid message %ld\n",arg,index);
#endif // DEBUG

                    // Set hold state
                    holding=TRUE;

                    // While holding ...
                    while (holding) {

                        // Debugging
#ifdef DEBUG
                        kprintf("Child: Waiting for message on semaphore port\n");
#endif // DEBUG

                        // Wait for message at semaphore port
                        WaitPort(semaphorePort);

                        // Debugging
#ifdef DEBUG
                        kprintf("Child: Message received at semaphore port\n");
#endif // DEBUG

                        // Forbid multi-tasking
                        Forbid();

                        // Begin with first message
                        thisSemaphoreMessage=(struct SemaphoreMessage *)
                            semaphorePort->mp_MsgList.lh_Head;

                        // Loop through all messages
                        while (nextSemaphoreMessage=(struct SemaphoreMessage *)
                            thisSemaphoreMessage->ssm_Message.mn_Node.ln_Succ) {

                            // If this message is bid message ...
                            if (thisSemaphoreMessage==&(semaphoreMessage[index])) {

                                // Debugging
#ifdef DEBUG
                                kprintf("Child: Received reply to bid message\n");
#endif // DEBUG

                                // Remove message from message port
                                Remove(thisSemaphoreMessage);

                                // Clear hold state
                                holding=FALSE;

                            } else {

                                // Debugging
#ifdef DEBUG
                                kprintf("Child: Irrelevant message received at semaphore port\n");
#endif // DEBUG


                            }

                            // Process next semaphore message
                            thisSemaphoreMessage=nextSemaphoreMessage;

                        }

                        // Debugging
#ifdef DEBUG
                        kprintf("Child: Out of semaphore message port loop\n");
#endif // DEBUG

                        // Permit multi-tasking
                        Permit();

                    }

                    // Debugging
#ifdef DEBUG
                    kprintf("Child: Out of hold\n");
#endif // DEBUG

                    // Wait for command
                    cmd = wait_cmd;

                    break;

                case sc_vacate: // (arg = semaphore, index = bid)

                    // Debugging
#ifdef DEBUG
                    kprintf("Child: Vacate of semaphore $%08lx on bid message %d\n",arg,index);
#endif // DEBUG

                    // Vacate semaphore
                    Vacate(arg,&(semaphoreMessage[index]));

                    // Wait for command
                    cmd = wait_cmd;

                    break;

                case sc_die:
                    done = TRUE;
                    Forbid();  // we will stay forbidden until we die
                    parent_port = FindPort(MASTER_PORT);
                    if (mess = CreateSemaMessage(NULL))
                    {
                        mess->cmd = sm_dead;
                        PutMsg(parent_port, mess);
                    }
                    break;

                default:
                    cmd = sc_get_message;
                    break;
                }
            }

            DeletePort(port);
        }

        DeletePort(semaphorePort);

    }
}

void DisplayMessage(struct RastPort *rp, ULONG tasknum, enum Command cmd, long index)
{
    long top;
    char str[80], *s;

    top = STATUS_BOX_TOP + (BUTTON_HEIGHT / 2 - rp->Font->tf_YSize / 2 + rp->Font->tf_Baseline);

    strcpy(str, cmd_string[cmd]);
    if (s = stpchr(str, '*'))
        *s = 'a' + index;

    SetAPen(rp, 1);
    SetBPen(rp, 2);
    SetDrMd(rp, JAM2);
    Move(rp, STATUS_BOX_LEFT + X_SPACE, top + Y_SPACE * tasknum);
    Text(rp, str, strlen(str));
}

void ChildRender(struct RastPort *rp, ULONG tasknum, ULONG rend)
{
    SetAPen(rp, 1);
    // SetBPen(rp, 0);
    SetDrMd(rp, COMPLEMENT);
    WritePixel(rp, ACTIVITY_AREA_LEFT + (rend % ACTIVITY_AREA_WIDTH),
                   ACTIVITY_AREA_TOP + Y_SPACE * tasknum
                    + ((rend / ACTIVITY_AREA_WIDTH) % ACTIVITY_AREA_HEIGHT));
}

// Master Code - Create and handle the nessacary gadgets for the interface

BOOL CreateInterface(struct GlobalEnvironment *ge) // Returns TRUE if succeeds
{
    BOOL ret;
    int i;

    if (ret = !!CreateContext(&ge->gadlist))
    {
        ge->error = err_ok;

        // Command Selection (Start, Stop, Reset, and Exit gadgets)
        FirstGadget(ge, "_Start", BUTTON_KIND, START_LEFT, START_TOP, BUTTON_WIDTH, BUTTON_HEIGHT, PLACETEXT_IN, sg_Start, 0, GT_Underscore, '_', TAG_DONE);
        NextGadget(ge, "_Halt",  BUTTON_KIND, X_SPACE, 0, PLACETEXT_IN, sg_Halt, 0, GT_Underscore, '_', TAG_DONE);
        NextGadget(ge, "S_tep",  BUTTON_KIND, X_SPACE, 0, PLACETEXT_IN, sg_Step, 0, GT_Underscore, '_', TAG_DONE);
        NextGadget(ge, "_Reset", BUTTON_KIND, X_SPACE, 0, PLACETEXT_IN, sg_Reset, 0, GT_Underscore, '_', TAG_DONE);
        NextGadget(ge, "_Clear", BUTTON_KIND, X_SPACE, 0, PLACETEXT_IN, sg_Clear, 0, GT_Underscore, '_', TAG_DONE);
        NextGadget(ge, "Hel_p", BUTTON_KIND, X_SPACE, 0, PLACETEXT_IN, sg_Help, 0, GT_Underscore, '_', TAG_DONE);
        NextGadget(ge, "_Exit",  BUTTON_KIND, X_SPACE, 0, PLACETEXT_IN, sg_Exit, 0, GT_Underscore, '_', TAG_DONE);

#ifdef FUTURE
        // State selection (Busy, Wait, etc.)
        FirstGadget(ge, "B_usy", BUTTON_KIND, BUSY_LEFT, BUSY_TOP, BUTTON_WIDTH, BUTTON_HEIGHT, PLACETEXT_IN, sg_Busy, 0, GA_Disabled, FALSE, GT_Underscore, '_', TAG_DONE);
        NextGadget(ge, "_Wait",  BUTTON_KIND, X_SPACE, 0, PLACETEXT_IN, sg_Wait, 0, GT_Underscore, '_', TAG_DONE);
        NextGadget(ge, "Sa _A",  BUTTON_KIND, X_SPACE, 0, PLACETEXT_IN, sg_SsW, 0, GT_Underscore, '_', TAG_DONE);
        NextGadget(ge, "Sb _B",  BUTTON_KIND, X_SPACE, 0, PLACETEXT_IN, sg_SsX, 0, GT_Underscore, '_', TAG_DONE);
        NextGadget(ge, "Ea _C",  BUTTON_KIND, X_SPACE, 0, PLACETEXT_IN, sg_EsY, 0, GT_Underscore, '_', TAG_DONE);
        NextGadget(ge, "Eb _D",  BUTTON_KIND, X_SPACE, 0, PLACETEXT_IN, sg_EsZ, 0, GT_Underscore, '_', TAG_DONE);
#endif
        // Task buttons
        FirstGadget(ge, "Task_1", BUTTON_KIND, TASK_LEFT, TASK_TOP, BUTTON_WIDTH, BUTTON_HEIGHT, PLACETEXT_IN, sg_Task1, 0, GA_Disabled, FALSE, GT_Underscore, '_', TAG_DONE);
        NextGadget(ge, "Task_2",  BUTTON_KIND, 0, Y_SPACE, PLACETEXT_IN, sg_Task2, 0, GT_Underscore, '_', TAG_DONE);
        NextGadget(ge, "Task_3",  BUTTON_KIND, 0, Y_SPACE, PLACETEXT_IN, sg_Task3, 0, GT_Underscore, '_', TAG_DONE);
        NextGadget(ge, "Task_4",  BUTTON_KIND, 0, Y_SPACE, PLACETEXT_IN, sg_Task4, 0, GT_Underscore, '_', TAG_DONE);
        NextGadget(ge, "Task_5",  BUTTON_KIND, 0, Y_SPACE, PLACETEXT_IN, sg_Task5, 0, GT_Underscore, '_', TAG_DONE);
        NextGadget(ge, "Task_6",  BUTTON_KIND, 0, Y_SPACE, PLACETEXT_IN, sg_Task6, 0, GT_Underscore, '_', TAG_DONE);
        NextGadget(ge, "Task_7",  BUTTON_KIND, 0, Y_SPACE, PLACETEXT_IN, sg_Task7, 0, GT_Underscore, '_', TAG_DONE);

        // String Gadgets
        i = 0;
        ge->ch_strgads[i++] = FirstGadget(ge, "", STRING_KIND, STRING_LEFT, STRING_TOP, STRING_WIDTH, STRING_HEIGHT, PLACETEXT_LEFT, sg_Str1, 0, GTST_MaxChars, MAX_CHARS, GA_TabCycle, TRUE, GT_Underscore, '_', TAG_DONE);
        ge->ch_strgads[i++] = NextGadget(ge, "", STRING_KIND, 0, Y_SPACE, PLACETEXT_LEFT, sg_Str2, 0, GTST_MaxChars, MAX_CHARS, GA_TabCycle, TRUE, GT_Underscore, '_', TAG_DONE);
        ge->ch_strgads[i++] = NextGadget(ge, "", STRING_KIND, 0, Y_SPACE, PLACETEXT_LEFT, sg_Str3, 0, GTST_MaxChars, MAX_CHARS, GA_TabCycle, TRUE, GT_Underscore, '_', TAG_DONE);
        ge->ch_strgads[i++] = NextGadget(ge, "", STRING_KIND, 0, Y_SPACE, PLACETEXT_LEFT, sg_Str4, 0, GTST_MaxChars, MAX_CHARS, GA_TabCycle, TRUE, GT_Underscore, '_', TAG_DONE);
        ge->ch_strgads[i++] = NextGadget(ge, "", STRING_KIND, 0, Y_SPACE, PLACETEXT_LEFT, sg_Str5, 0, GTST_MaxChars, MAX_CHARS, GA_TabCycle, TRUE, GT_Underscore, '_', TAG_DONE);
        ge->ch_strgads[i++] = NextGadget(ge, "", STRING_KIND, 0, Y_SPACE, PLACETEXT_LEFT, sg_Str6, 0, GTST_MaxChars, MAX_CHARS, GA_TabCycle, TRUE, GT_Underscore, '_', TAG_DONE);
        ge->ch_strgads[i++] = NextGadget(ge, "", STRING_KIND, 0, Y_SPACE, PLACETEXT_LEFT, sg_Str7, 0, GTST_MaxChars, MAX_CHARS, GA_TabCycle, TRUE, GT_Underscore, '_', TAG_DONE);

        for (i = 0; i < NUMBER_OF_TASKS; ++i)
            ge->ch_str[i] = ((struct StringInfo *) ge->ch_strgads[i]->SpecialInfo)->Buffer;

        // Status Boxes (just drawn)

        DrawBevelBox(ge->win->RPort, STATUS_BOX_LEFT, STATUS_BOX_TOP, STATUS_BOX_WIDTH, STATUS_BOX_HEIGHT, GT_VisualInfo, ge->visual_info, GTBB_Recessed, TRUE, TAG_DONE);
        DrawBevelBox(ge->win->RPort, ACTIVITY_BOX_LEFT, ACTIVITY_BOX_TOP, ACTIVITY_BOX_WIDTH, ACTIVITY_BOX_HEIGHT, GT_VisualInfo, ge->visual_info, GTBB_Recessed, TRUE, TAG_DONE);

        // Attach gadgets to window!
        if (ge->error == err_ok)
        {
            AddGList(ge->win, ge->gadlist, 0, -1, NULL);
            RefreshGList(ge->gadlist, ge->win, NULL, -1);
            GT_RefreshWindow(ge->win, NULL);
        }
        else
            ret = FALSE;
    }
    return ret;
}

void ClearStrings(struct GlobalEnvironment *ge)
{
    int i;

    RemoveGList(ge->win, ge->gadlist, -1);
    for (i = 0; i < NUMBER_OF_TASKS; ++i)
    {
        ((struct StringInfo *) ge->ch_strgads[i]->SpecialInfo)->BufferPos = 0;
        ((struct StringInfo *) ge->ch_strgads[i]->SpecialInfo)->Buffer[0] = '\0';
    }

    AddGList(ge->win, ge->gadlist, 0, -1, NULL);
    RefreshGList(ge->gadlist, ge->win, NULL, -1);
    GT_RefreshWindow(ge->win, NULL);
}

void DestroyInterface(struct GlobalEnvironment *ge)
{
    RemoveGList(ge->win, ge->gadlist, -1);
    FreeGadgets(ge->gadlist), ge->gadlist = NULL;
}

void DoInterface(struct GlobalEnvironment *ge)
{
    BOOL loop = TRUE, finish_up = FALSE, working = TRUE;
    struct IntuiMessage m, *mess;
    struct SemaMessage *sm;
    struct Gadget *g;
    int i;
    ULONG bits; // Wait bits
    ULONG window_bit, port_bit;
    char buf[80];

    // Init environment
    for (i = 0; i < NUMBER_OF_SEMAPHORES; ++i)
        InitSemaphore(&ge->ss[i]);

    if (ge->master_port = CreatePort(MASTER_PORT, 0))
    {
        port_bit = 1 << ge->master_port->mp_SigBit;
        window_bit = 1 << ge->win->UserPort->mp_SigBit;

        // Start up tasks
        SetTaskPri(FindTask(NULL), NUMBER_OF_TASKS); // raise our priority to be higher than our comming children
        for (i = 0; !ge->error && i < NUMBER_OF_TASKS; ++i)
        {
            sprintf(buf, "%s%ld", CHILD_PORT, i+1);
            if (!(ge->child_tasks[i] = CreateTask(strdup(buf), i, &Child, 6000)))
                ge->error = err_child_stillborn;
        }

        while (loop && !ge->error)
        {
            bits = Wait(window_bit | port_bit);

            if (bits & window_bit)
            {
                while (mess = GT_GetIMsg(ge->win->UserPort))
                {
                    FILE *fi, *fo;
                    char buf[256];

                    m = *mess;
                    GT_ReplyIMsg(mess);

                    switch (m.Class)
                    {
                    case IDCMP_CLOSEWINDOW:
                        // We must tell all children to die here.
                        for (i = 0; i < NUMBER_OF_TASKS; ++i)
                            if (ge->child_tasks[i] && ge->child_ports[i])
                                DoSemaMessage(ge, i, sc_die, NULL, NULL);

                        finish_up = TRUE;
                        break;

                    case IDCMP_INTUITICKS:
                        // We advance the commands as indicated by the strings
                        if (!finish_up && working)
                            for (i = 0; i < NUMBER_OF_TASKS; ++i)
                                if (ge->child_ports[i])
                                    StepChild(ge, i);
                        break;

                    case IDCMP_GADGETDOWN:
                        break;

                    case IDCMP_GADGETUP:
                        g = m.IAddress;
                        switch(g->GadgetID)
                        {
                        case sg_Start:
                            working = TRUE;
                            break;

                        case sg_Halt:
                            working = FALSE;
                            break;

                        case sg_Step:
                            for (i = 0; i < NUMBER_OF_TASKS; ++i)
                                if (ge->child_ports[i])
                                    StepChild(ge, i);
                            break;

                        case sg_Reset:
                            ResetChildren(ge);
                            working = FALSE;
                            break;

                        case sg_Clear:  // reset, and clear out strings
                            ResetChildren(ge);
                            ClearStrings(ge);
                            working = FALSE;
                            break;

                        case sg_Help:
                            if (fi = fopen(HELP_FILE, "r"))
                            {
                                if (fo = fopen(HELP_OUTPUT, "w"))
                                {
                                    while (fgets(buf, sizeof(buf), fi))
                                        fputs(buf, fo);

                                    fclose(fo);
                                }

                                fclose(fi);
                            }
                            break;

                        case sg_Exit:
                            // We must tell all children to die here.
                            for (i = 0; i < NUMBER_OF_TASKS; ++i)
                                if (ge->child_tasks[i] && ge->child_ports[i])
                                    DoSemaMessage(ge, i, sc_die, NULL, NULL);
                            finish_up = TRUE;
                            break;

                        case sg_Task1:  // Single-step (done during a HALT condition)
                        case sg_Task2:
                        case sg_Task3:
                        case sg_Task4:
                        case sg_Task5:
                        case sg_Task6:
                        case sg_Task7:
                            StepChild(ge, g->GadgetID - sg_Task1); // compute the child number and step him
                            break;
                        }
                        break;

                    default:
                        break;
                    }
                }
            }

            if (bits & port_bit) // message or reply from one of our children
            {
                while (sm = GetMsg(ge->master_port))
                {
                    struct RastPort *rp;

                    switch (sm->cmd)
                    {
                    case sm_dead_abort:
                        ge->error = err_child_stillborn;
                        DestroySemaMessage(sm);
                        break;

                    case sm_dead: // died in response to sc_die
                        ge->child_tasks[FindChildEntry(ge, sm->task)] = NULL;
                        DestroySemaMessage(sm);
                        loop = AnyRemainingChildren(ge);
                        break;

                    case sm_my_port: // child registery of port
                        ge->child_ports[FindChildEntry(ge, sm->task)] = sm->arg;
                        break;

                    case sm_what_mytasknum: // child requesting task number
                        sm->arg = FindChildEntry(ge, sm->task);
                        ReplyMsg(sm);
                        break;

                    case sm_what_rport: // child requesting rastport
                        // We will make a copy (!) of the RastPort for
                        //  our children (spence). This should
                        //  eliminate the pathologies encountered
                        //  before. Let's pray that our children
                        //  not ask for RastPorts always! :-)

                        if (rp = malloc(sizeof(*rp)))
                            *rp = *ge->win->RPort;
                        else // out of memory - fallback procedure
                            rp = ge->win->RPort;

                        sm->arg = rp;
                        ReplyMsg(sm);
                        break;

                    default: // Reply to one of our own messages
                        DestroySemaMessage(sm);
                        break;
                    }
                }
            }
        }

        // We will kill any left-over children by force, at this point!
        for (i = 0; i < NUMBER_OF_TASKS; ++i)
            if (ge->child_tasks[i])
                DeleteTask(ge->child_tasks[i]), ge->child_tasks[i] = NULL;

        DeletePort(ge->master_port), ge->master_port = NULL;
    }
    else
        ge->error = err_port_failed;
}

void StepChild(struct GlobalEnvironment *ge, int child)
{
    char c;
    char b; // Bid number

    switch (ge->ch_str[child][ge->ch_index[child]++])
    {
    case 'U':   // Busy-wait command
        // Debugging
#ifdef DEBUG
        kprintf("StepChild: busy wait\n");
#endif // DEBUG

        DoSemaMessage(ge, child, sc_busy, NULL, NULL);
        break;

    case 'W':   // Wait for next message
        // Debugging
#ifdef DEBUG
        kprintf("StepChild: wait\n");
#endif // DEBUG

        DoSemaMessage(ge, child, sc_wait, NULL, NULL);
        break;

    case 'S':   // Grab shared (Sx)
        // Debugging
#ifdef DEBUG
        kprintf("StepChild: Obtain shared\n");
#endif // DEBUG

        c = ge->ch_str[child][ge->ch_index[child]++];
        if ((c = tolower(c) - 'a') < 0 || c >= NUMBER_OF_SEMAPHORES)
            c = 0;
        DoSemaMessage(ge, child, sc_grab_shared, &ge->ss[c], c);
        break;

    case 'E':   // Grab exclusive (Ex)
        // Debugging
#ifdef DEBUG
        kprintf("StepChild: Obtain exclusive\n");
#endif // DEBUG

        c = ge->ch_str[child][ge->ch_index[child]++];
        if ((c = tolower(c) - 'a') < 0 || c >= NUMBER_OF_SEMAPHORES)
            c = 0;
        DoSemaMessage(ge, child, sc_grab_exclusive, &ge->ss[c], c);
        break;

    case 'R':   // Release semaphore  (Rx)
        // Debugging
#ifdef DEBUG
        kprintf("StepChild: Release\n");
#endif // DEBUG

        c = ge->ch_str[child][ge->ch_index[child]++];
        if ((c = tolower(c) - 'a') < 0 || c >= NUMBER_OF_SEMAPHORES)
            c = 0;
        DoSemaMessage(ge, child, sc_release, &ge->ss[c], c);
        break;

    case 'P':   // Procure semaphore exclusive (Pxy)
        // Debugging
#ifdef DEBUG
        kprintf("StepChild: Procure exclusive\n");
#endif // DEBUG

        c = ge->ch_str[child][ge->ch_index[child]++];
        if ((c = tolower(c) - 'a') < 0 || c >= NUMBER_OF_SEMAPHORES)
            c = 0;
        b = ge->ch_str[child][ge->ch_index[child]++];
        if ((b = b - '0' ) < 0 || b >= NUMBER_OF_BIDS)
            b=0;
        DoSemaMessage(ge, child, sc_procure_exclusive, &ge->ss[c], b);
        break;

    case 'p':   // Procure semaphore shared on bid (pxy)
        // Debugging
#ifdef DEBUG
        kprintf("StepChild: Procure shared\n");
#endif // DEBUG

        c = ge->ch_str[child][ge->ch_index[child]++];
        if ((c = tolower(c) - 'a') < 0 || c >= NUMBER_OF_SEMAPHORES)
            c = 0;
        b = ge->ch_str[child][ge->ch_index[child]++];
        if ((b = b - '0' ) < 0 || b >= NUMBER_OF_BIDS)
            b=0;
        DoSemaMessage(ge, child, sc_procure_shared, &ge->ss[c], b);
        break;

    case 'H':   // Hold for semaphore on bid (after P or p) (Hxy)
        // Debugging
#ifdef DEBUG
        kprintf("StepChild: Hold\n");
#endif // DEBUG

        c = ge->ch_str[child][ge->ch_index[child]++];
        if ((c = tolower(c) - 'a') < 0 || c >= NUMBER_OF_SEMAPHORES)
            c = 0;
        b = ge->ch_str[child][ge->ch_index[child]++];
        if ((b = b - '0' ) < 0 || b >= NUMBER_OF_BIDS)
            b=0;
        DoSemaMessage(ge, child, sc_hold, &ge->ss[c], b);
        break;

    case 'V':   // Vacate semaphore on bid (Vxy)
#ifdef DEBUG
        kprintf("StepChild: Vacate\n");
#endif // DEBUG

        c = ge->ch_str[child][ge->ch_index[child]++];
        if ((c = tolower(c) - 'a') < 0 || c >= NUMBER_OF_SEMAPHORES)
            c = 0;
        b = ge->ch_str[child][ge->ch_index[child]++];
        if ((b = b - '0' ) < 0 || b >= NUMBER_OF_BIDS)
            b=0;
        DoSemaMessage(ge, child, sc_vacate, &ge->ss[c], b);
        break;

    case '<':   // Jump back to beginning
        ge->ch_index[child] = 0;
        DoSemaMessage(ge, child, sc_noop, NULL, NULL);
        break;

    case '[':   // Place to loop back to
        ge->ch_loopback[child] = ge->ch_index[child];
        DoSemaMessage(ge, child, sc_noop, NULL, NULL);
        break;

    case ']':   // loop back
        ge->ch_index[child] = ge->ch_loopback[child];
        DoSemaMessage(ge, child, sc_noop, NULL, NULL);
        break;

    case '\0':  // keep index at end
        --ge->ch_index[child];
        DoSemaMessage(ge, child, sc_noop, NULL, NULL);
        break;

    default: // send a no-operation command
        DoSemaMessage(ge, child, sc_noop, NULL, NULL);
        break;
    }
}

void ResetChildren(struct GlobalEnvironment *ge)
{
    int i;

    for (i = 0; i < NUMBER_OF_TASKS; ++i)
    {
        ge->ch_index[i] = 0;
        DoSemaMessage(ge, i, sc_wait, NULL, NULL);
        Delay(2);  // give'em a chance to process
    }

    // reset semaphores
    for (i = 0; i < NUMBER_OF_SEMAPHORES; ++i)
        while (ge->ss[i].ss_NestCount)
        {
            ReleaseSemaphore(&ge->ss[i]);
            Delay(2);
        }

    for (i = 0; i < NUMBER_OF_SEMAPHORES; ++i)
        InitSemaphore(&ge->ss[i]);
}

int FindChildEntry(struct GlobalEnvironment *ge, struct Task *task)
{
    int i;

    for (i = 0; i < NUMBER_OF_TASKS; ++i)
        if (ge->child_tasks[i] == task)
            break;

    return i;
}

BOOL AnyRemainingChildren(struct GlobalEnvironment *ge)
{
    int i;
    BOOL ret = FALSE;

    for (i = 0; i < NUMBER_OF_TASKS; ++i)
        if (ge->child_tasks[i])
            ret = TRUE;

    return ret;
}

struct Gadget *FirstGadget(struct GlobalEnvironment *ge, UBYTE *text, ULONG kind,
    WORD left, WORD top, WORD width, WORD height,
    ULONG flags, UWORD gid, APTR uid,
    ULONG taglist, ...)
{
    struct Gadget *g = NULL;

    if (ge->error == err_ok)
    {
        ge->ng.ng_LeftEdge = left;
        ge->ng.ng_TopEdge = top;
        ge->ng.ng_Width = width;
        ge->ng.ng_Height = height;
        ge->ng.ng_Flags = flags;
        ge->ng.ng_GadgetText = text;
        ge->ng.ng_GadgetID = gid;
        ge->ng.ng_UserData = uid;
        ge->ng.ng_VisualInfo = ge->visual_info;
        ge->ng.ng_TextAttr = &ge->ta;

        if (!ge->prev_gadget)
            ge->prev_gadget = ge->gadlist;

        if (!(g = ge->prev_gadget = CreateGadgetA(kind, ge->prev_gadget, &ge->ng, (void *) &taglist)))
            ge->error = err_failed;
    }
    return g;
}

struct Gadget *NextGadget(struct GlobalEnvironment *ge, UBYTE *text, ULONG kind,
                            WORD dleft, WORD dtop,
                            ULONG flags,
                            ULONG gid, APTR uid, ULONG taglist, ...) // Set up NewGadget structure
{
    struct Gadget *g = NULL;

    if (ge->error == err_ok)
    {
        ge->ng.ng_LeftEdge += dleft;
        ge->ng.ng_TopEdge += dtop;
        ge->ng.ng_Flags = flags;
        ge->ng.ng_GadgetID = gid;
        ge->ng.ng_UserData = uid;
        ge->ng.ng_GadgetText = text;

        if (!(g = ge->prev_gadget = CreateGadgetA(kind, ge->prev_gadget, &ge->ng, (void *) &taglist)))
            ge->error = err_failed;
    }
    return g;
}

