/*
 * $Id: quotes.h,v 38.1 91/06/24 11:38:06 mks Exp $
 *
 * $Log:	quotes.h,v $
 * Revision 38.1  91/06/24  11:38:06  mks
 * Initial V38 tree checkin - Log file stripped
 * 
 */

/* ErrorTitle() strings */
/*  1234567890123456789012345 */
enum {
    Q_DRW_IS_NOT_A_DIR,
    Q_CANT_RENAME_DISK,
    Q_DRW_HAS_NO_PARENT,
    Q_ICON_CANT_B_IN_WIN,
    Q_CANT_MOV_OUT_OF_WIN,
    Q_CANT_COPY_TO_ITSELF,
    Q_CANT_DISKCOPY_SRC,
    Q_INFO_FAILED,
    Q_DO_NOT_USE_IN_NAMES,
    Q_OUT_OF_MEM,
    Q_ICON_HAS_NO_DEF_TOOL,
    Q_CANT_LAUNCH_PROGRAM,
    Q_NO_MEM_TO_RUN,
    Q_CANT_OPEN_TOOL,
    Q_DRW_CANT_BE_OPENED,

/* format strings for error while WB was doing some DOS thing */
    Q_ERROR_WHILE,
    Q_EXAMINING_DIR,
    Q_ACCESSING_STRING,
    Q_OPENING_STRING,
    Q_READING,
    Q_WRITING,
    Q_MOVING_STRING,
    Q_REMOVING_STRING,
    Q_EXAMINING_STRING,
    Q_WRITING_STRING,
    Q_CREATING_STRING,

/* format string for workbench screen title */
    Q_SCREEN_TITLE,

/* format string for menu item 'Version' */
    Q_VERSION_TITLE,

/* misc. requester(s) strings */
    Q_RETRY_TEXT,
    Q_CANCEL_TEXT,
    Q_OK_TEXT,
    Q_SAVE_TEXT,
    Q_OK_CANCEL_TEXT,
    Q_RETRY_CANCEL_TEXT,
    Q_REMOVE_CANCEL_TEXT,
    Q_REPLACE_CANCEL_TEXT,

    Q_INFO_TITLE,

/* protection bit descriptions (used by Info) */
    Q_SCRIPT_BIT,
    Q_PURE_BIT,
    Q_ARCHIVED_BIT,
    Q_DELETE_BIT,
    Q_READ_BIT,
    Q_WRITE_BIT,
    Q_EXECUTE_BIT,

/* various strings used by Info */
    Q_STACK,
    Q_VOLUME,
    Q_BLOCKS,
    Q_USED,
    Q_FREE,
    Q_BLOCK_SIZE,
    Q_BYTES,
    Q_LAST_CHANGED,
    Q_CREATED,
    Q_COMMENT,
    Q_DEFAULT_TOOL,
    Q_TOOL_TYPES,

/* volume string and volume status (in Info) */
    Q_VOLUME_TEXT,
    Q_VOL_IS_READ_ONLY,
    Q_VOL_IS_VALIDATING,
    Q_VOL_IS_READ_WRITE,
    Q_VOL_IS_UNKNOWN,

/* ToolTypes gadget strings (used in Info) */
    Q_NEW,
    Q_DELETE,

/* icon types (used in Info) */
    Q_IDISK,
    Q_IDRAWER,
    Q_ITOOL,
    Q_IPROJECT,
    Q_ITRASHCAN,

/* file type when in View By Name/Date/Size */
    Q_WBDRAWER,

/* Start of workbench menu strings */
    Q_END_MENU_MENU,
    Q_END_MENU_MENU_KEY,

    Q_WORKBENCH_MENU,
    Q_WORKBENCH_MENU_KEY,
    Q_FLIP_WB_MENU,
    Q_FLIP_WB_MENU_KEY,
    Q_EXECUTE_MENU,
    Q_EXECUTE_MENU_KEY,
    Q_REDRAW_ALL_MENU,
    Q_REDRAW_ALL_MENU_KEY,
    Q_RESCAN_ALL_MENU,
    Q_RESCAN_ALL_MENU_KEY,
    Q_LAST_ERROR_MENU,
    Q_LAST_ERROR_MENU_KEY,
    Q_VERSION_MENU,
    Q_VERSION_MENU_KEY,
    Q_QUIT_MENU,
    Q_QUIT_MENU_KEY,

    Q_WINDOW_MENU,
    Q_WINDOW_MENU_KEY,
    Q_NEW_DRAWER_MENU,
    Q_NEW_DRAWER_MENU_KEY,
    Q_PARENT_MENU,
    Q_PARENT_MENU_KEY,
    Q_CLOSE_MENU,
    Q_CLOSE_MENU_KEY,
    Q_RESCAN_MENU,
    Q_RESCAN_MENU_KEY,
    Q_SELECT_ALL_MENU,
    Q_SELECT_ALL_MENU_KEY,
    Q_CLEAN_UP_MENU,
    Q_CLEAN_UP_MENU_KEY,
    Q_SNAPSHOT_MENU,
    Q_SNAPSHOT_MENU_KEY,
    Q_SHOW_MENU,
    Q_SHOW_MENU_KEY,
    Q_VIEW_BY_MENU,
    Q_VIEW_BY_MENU_KEY,

    Q_ICON_MENU,
    Q_ICON_MENU_KEY,
    Q_OPEN_MENU,
    Q_OPEN_MENU_KEY,
    Q_COPY_MENU,
    Q_COPY_MENU_KEY,
    Q_RENAME_MENU,
    Q_RENAME_MENU_KEY,
    Q_INFO_MENU,
    Q_INFO_MENU_KEY,
    Q_SNAPSHOTI_MENU,
    Q_SNAPSHOTI_MENU_KEY,
    Q_UNSNAPSHOT_MENU,
    Q_UNSNAPSHOT_MENU_KEY,
    Q_LEAVE_OUT_MENU,
    Q_LEAVE_OUT_MENU_KEY,
    Q_PUT_AWAY_MENU,
    Q_PUT_AWAY_MENU_KEY,
    Q_SEPARATOR_MENU,
    Q_SEPARATOR_MENU_KEY,
    Q_DISCARD_MENU,
    Q_DISCARD_MENU_KEY,
    Q_FORMAT_DISK_MENU,
    Q_FORMAT_DISK_MENU_KEY,
    Q_EMPTY_TRASH_MENU,
    Q_EMPTY_TRASH_MENU_KEY,

    Q_TOOLS_MENU,
    Q_TOOLS_MENU_KEY,
    Q_RESETWB_MENU,
    Q_RESETWB_MENU_KEY,

    Q_SNAPSHOT_WINDOW_MENU,
    Q_SNAPSHOT_WINDOW_MENU_KEY,
    Q_SNAPSHOT_ALL_MENU,
    Q_SNAPSHOT_ALL_MENU_KEY,

    Q_VIEW_BY_ICON_MENU,
    Q_VIEW_BY_ICON_MENU_KEY,
    Q_VIEW_BY_NAME_MENU,
    Q_VIEW_BY_NAME_MENU_KEY,
    Q_VIEW_BY_DATE_MENU,
    Q_VIEW_BY_DATE_MENU_KEY,
    Q_VIEW_BY_SIZE_MENU,
    Q_VIEW_BY_SIZE_MENU_KEY,

    Q_SHOW_ONLY_ICONS_MENU,
    Q_SHOW_ONLY_ICONS_MENU_KEY,
    Q_SHOW_ALL_FILES_MENU,
    Q_SHOW_ALL_FILES_MENU_KEY,

/* Discard requester strings */
    Q_DISCARD_REQ_TEXT,

/* Quit wb requester strings */
    Q_QUIT_REQ_TEXT,

/* this is the name 'New Drawer' uses for the new drawer */
    Q_NEW_DRAWER_NAME,

/* format string for window title of disks (volumes) */
    Q_VOLUME_TITLE_FORMAT,

/* Rename requester strings */
    Q_RENAME_GREETING,
    Q_RENAME_REQ_TITLE,
    Q_RENAME_LABEL,
    Q_RENAME_FAILED,

/* misc. entries */
    Q_EXECUTE_CMD,
    Q_EXECUTE_CMD_AND_ARGS,
    Q_EXECUTE_FAILED,
    Q_WB_OUTPUT_TITLE,
    Q_COMMAND_TEXT,
    Q_EXECUTE_REQ_TITLE,
    Q_TOOL_EXIT,
    Q_LIBRARY_OPEN,

    Q_CANNOT_LEAVE_OUT,
    Q_CANNOT_PUT_AWAY,

    Q_CANNOT_FORMAT,

    Q_ALREADY_LEFT_OUT,

    Q_RENAME_LEFTOUT,

    Q_CANNOT_DELETE,

    Q_LOADING,

    Q_STARTUP_WAIT,

/* Copy error texts... */
    Q_COPY_ERROR_TEXT,
    Q_COPY_DELETE_TEXT,
    Q_CANT_COPY,

/*
 * Moved the debug stuff to the end such that it can be removed
 * via a simple change...
 */

    Q_DEBUG_MENU,
    Q_DEBUG_MENU_KEY,
    Q_ROMWACK_MENU,
    Q_ROMWACK_MENU_KEY,
    Q_FLUSHLIBS_MENU,
    Q_FLUSHLIBS_MENU_KEY,

    Q_ENDOFQUOTES
};

#define Q_ERROR_WHILE_BASE	Q_EXAMINING_DIR
#define Q_I_ICONTYPE_BASE	Q_IDISK
