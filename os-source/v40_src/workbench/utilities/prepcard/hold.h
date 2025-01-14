#ifndef PREPCARD_TEXT_H
#define PREPCARD_TEXT_H


/****************************************************************************/


/* This file was created automatically by CatComp.
 * Do NOT edit by hand!
 */


#include <exec/types.h>


/****************************************************************************/


#ifdef PREPCARD
#define MSG_PREP_CANCEL_GAD 0
#define MSG_PREP_CANCEL_GAD_STR "Cancel"

#define MSG_PREP_OK_GAD 1
#define MSG_PREP_OK_GAD_STR "Ok"

#define MSG_PREP_QUERY_GAD 2
#define MSG_PREP_QUERY_GAD_STR "Continue|Cancel"

#define MSG_PREP_CONTINUE_GAD 3
#define MSG_PREP_CONTINUE_GAD_STR "Continue"

#define MSG_PREP_TITLE 4
#define MSG_PREP_TITLE_STR "PrepCard"

#define MSG_PREP_ADV_TITLE 5
#define MSG_PREP_ADV_TITLE_STR "PrepCard Advanced Settings"

#define MSG_PREP_FORMAT_GAD 6
#define MSG_PREP_FORMAT_GAD_STR "Prepare as DISK"

#define MSG_PREP_RAM_GAD 7
#define MSG_PREP_RAM_GAD_STR "Prepare as System RAM"

#define MSG_PREP_TYPE_GAD 8
#define MSG_PREP_TYPE_GAD_STR "Type :"

#define MSG_PREP_SPEED_GAD 9
#define MSG_PREP_SPEED_GAD_STR "Speed :"

#define MSG_PREP_UNITS_GAD 10
#define MSG_PREP_UNITS_GAD_STR "Unit Size :"

#define MSG_PREP_UNNUM_GAD 11
#define MSG_PREP_UNNUM_GAD_STR "Units :   "

#define MSG_PREP_SIZE_GAD 12
#define MSG_PREP_SIZE_GAD_STR "Total Size :"

#define MSG_PREP_EDC_GAD 13
#define MSG_PREP_EDC_GAD_STR "Error Detection :"

#define MSG_PREP_BKSZ_GAD 14
#define MSG_PREP_BKSZ_GAD_STR "Block Size :"

#define MSG_PREP_MAXB_GAD 15
#define MSG_PREP_MAXB_GAD_STR "Maximum Blocks :"

#define MSG_PREP_MAXE_GAD 16
#define MSG_PREP_MAXE_GAD_STR "Total Sectors :"

#define MSG_PREP_SECE_GAD 17
#define MSG_PREP_SECE_GAD_STR "Sectors/Track :"

#define MSG_PREP_TRKE_GAD 18
#define MSG_PREP_TRKE_GAD_STR "Tracks/Cylinder :"

#define MSG_PREP_CYLE_GAD 19
#define MSG_PREP_CYLE_GAD_STR "Cylinders :"

#define MSG_PREP_GEOE_GAD 20
#define MSG_PREP_GEOE_GAD_STR "Geometry :"

#define MSG_PREP_DINFO_GAD 21
#define MSG_PREP_DINFO_GAD_STR "Device"

#define MSG_PREP_FINFO_GAD 22
#define MSG_PREP_FINFO_GAD_STR "Format"

#define MSG_PREP_GINFO_GAD 23
#define MSG_PREP_GINFO_GAD_STR "Geometry"

#define MSG_PREP_INSERT_PROMPT 500
#define MSG_PREP_INSERT_PROMPT_STR "Press RETURN to continue, or CTRL-C to quit..."

#define MSG_PREP_WARN_PROMPT 501
#define MSG_PREP_WARN_PROMPT_STR "Warning!!!  All existing information will be erased!\n"

#define MSG_PREP_FORMAT_PROMPT 502
#define MSG_PREP_FORMAT_PROMPT_STR "Insert RAM card to prepare for use as a DISK.\n"

#define MSG_PREP_RAM_PROMPT 503
#define MSG_PREP_RAM_PROMPT_STR "Insert RAM card to prepare for use as System RAM.\n"

#define MSG_PREP_UI_PROMPT 504
#define MSG_PREP_UI_PROMPT_STR "Warning!!!\nAll existing information will be erased!"

#define MSG_PREP_BUSY_PROMPT 505
#define MSG_PREP_BUSY_PROMPT_STR "Busy...\n"

#define MSG_PREP_DONE_PROMPT 506
#define MSG_PREP_DONE_PROMPT_STR "Done\n"

#define MSG_PREP_ERROR_ISRAM 1000
#define MSG_PREP_ERROR_ISRAM_STR "Card is already in use as System RAM\n"

#define MSG_PREP_ERROR_NORES 1001
#define MSG_PREP_ERROR_NORES_STR "No card slot\n"

#define MSG_PREP_ERROR_INUSE 1002
#define MSG_PREP_ERROR_INUSE_STR "Card is in use"

#define MSG_PREP_NO_CARD_ERROR 1003
#define MSG_PREP_NO_CARD_ERROR_STR "Unable to access card\n"

#define MSG_PREP_NO_MEM_ERROR 1004
#define MSG_PREP_NO_MEM_ERROR_STR "Not enough memory\n"

#define MSG_PREP_CORRUPT_ERROR 1005
#define MSG_PREP_CORRUPT_ERROR_STR "Warning!!!\nCard information appears to be CORRUPT!\n\nRecommend: Prepare card before use."

#define MSG_PREP_PSEUDO_WARN 1006
#define MSG_PREP_PSEUDO_WARN_STR "Warning!!!\nCard may contain data from another machine\nstored in a non-standard format.\n\nRecommend: Write-protect this card to prevent\nloss of data."

#define MSG_PREP_NA 1007
#define MSG_PREP_NA_STR "N/A"

#define MSG_PREP_NOCARD_ERROR 1008
#define MSG_PREP_NOCARD_ERROR_STR "Unable to prepare card:\n Unable to access card\n"

#define MSG_PREP_WP_ERROR 1009
#define MSG_PREP_WP_ERROR_STR "Unable to prepare card:\nCard is write-protected\n"

#define MSG_PREP_SMALL_ERROR 1010
#define MSG_PREP_SMALL_ERROR_STR "Unable to prepare card:\nCard is smaller than Advanced Settings indicate\n"

#define MSG_PREP_WRITE_ERROR 1011
#define MSG_PREP_WRITE_ERROR_STR "Unable to prepare card:\nError while writing changes\n"

#define MSG_PREP_REMOVED_ERROR 1012
#define MSG_PREP_REMOVED_ERROR_STR "Unable to prepare card:\nCard removed while writing\n"

#define MSG_PREP_MINBLKS_ERROR 1013
#define MSG_PREP_MINBLKS_ERROR_STR "Unable to prepare card:\nNot enough space for disk blocks\n"

#define MSG_PREP_BATTERY_STATUS 1014
#define MSG_PREP_BATTERY_STATUS_STR "Battery"

#define MSG_PREP_PINFO_STATUS 1015
#define MSG_PREP_PINFO_STATUS_STR "Version %ld.%ld  %s"

#define MSG_PREP_PREP_BUSY 1016
#define MSG_PREP_PREP_BUSY_STR "Busy..."

#define MSG_PREP_CARD_NULL 3000
#define MSG_PREP_CARD_NULL_STR "No device"

#define MSG_PREP_CARD_ROM 3001
#define MSG_PREP_CARD_ROM_STR "ROM"

#define MSG_PREP_CARD_OTPROM 3002
#define MSG_PREP_CARD_OTPROM_STR "One Time PROM"

#define MSG_PREP_CARD_EPROM 3003
#define MSG_PREP_CARD_EPROM_STR "UV EPROM"

#define MSG_PREP_CARD_EEPROM 3004
#define MSG_PREP_CARD_EEPROM_STR "EEPROM"

#define MSG_PREP_CARD_FLASH 3005
#define MSG_PREP_CARD_FLASH_STR "Flash ROM"

#define MSG_PREP_CARD_SRAM 3006
#define MSG_PREP_CARD_SRAM_STR "Static RAM"

#define MSG_PREP_CARD_DRAM 3007
#define MSG_PREP_CARD_DRAM_STR "Dynamic RAM"

#define MSG_PREP_CARD_UNKNOWN 3008
#define MSG_PREP_CARD_UNKNOWN_STR "Unknown"

#define MSG_PREP_CARD_IO 3009
#define MSG_PREP_CARD_IO_STR "I/O Device"

#define MSG_PREP_CARD_NOCARD 3010
#define MSG_PREP_CARD_NOCARD_STR "No card inserted"

#define MSG_PREP_BATTERY_OK 3011
#define MSG_PREP_BATTERY_OK_STR "OK"

#define MSG_PREP_BATTERY_LOW 3012
#define MSG_PREP_BATTERY_LOW_STR "LOW"

#define MSG_PREP_BATTERY_FAIL 3013
#define MSG_PREP_BATTERY_FAIL_STR "FAILED"

#define MSG_PREP_FORMAT_UNKNOWN 3014
#define MSG_PREP_FORMAT_UNKNOWN_STR "Unknown format - card contains data"

#define MSG_PREP_FORMAT_DISK 3015
#define MSG_PREP_FORMAT_DISK_STR "Disk-like format"

#define MSG_PREP_FORMAT_RAM 3016
#define MSG_PREP_FORMAT_RAM_STR "Usable as System RAM"

#define MSG_PREP_FORMAT_XIP 3017
#define MSG_PREP_FORMAT_XIP_STR "Execute-in-Place software"

#define MSG_PREP_FORMAT_NOPREP 3018
#define MSG_PREP_FORMAT_NOPREP_STR "Unknown"

#define MSG_PREP_FORMAT_PSEUDO 3019
#define MSG_PREP_FORMAT_PSEUDO_STR "Might be NON-STANDARD disk-like format"

#define MSG_PREP_FORMAT_CRDMARK 3020
#define MSG_PREP_FORMAT_CRDMARK_STR "Commodore CDTV bookmark card"

#define MSG_PREP_STANDARD 3021
#define MSG_PREP_STANDARD_STR "Standard card format"

#define MSG_PREP_NONSTANDARD 3022
#define MSG_PREP_NONSTANDARD_STR "Non-Standard card format"

#define MSG_PREP_SPEED_250NS 3023
#define MSG_PREP_SPEED_250NS_STR "250ns"

#define MSG_PREP_SPEED_200NS 3024
#define MSG_PREP_SPEED_200NS_STR "200ns"

#define MSG_PREP_SPEED_150NS 3025
#define MSG_PREP_SPEED_150NS_STR "150ns"

#define MSG_PREP_SPEED_100NS 3026
#define MSG_PREP_SPEED_100NS_STR "100ns"

#define MSG_PREP_UNITS_512 3027
#define MSG_PREP_UNITS_512_STR "512"

#define MSG_PREP_UNITS_2K 3028
#define MSG_PREP_UNITS_2K_STR "2K"

#define MSG_PREP_UNITS_8K 3029
#define MSG_PREP_UNITS_8K_STR "8K"

#define MSG_PREP_UNITS_32K 3030
#define MSG_PREP_UNITS_32K_STR "32K"

#define MSG_PREP_UNITS_128K 3031
#define MSG_PREP_UNITS_128K_STR "128K"

#define MSG_PREP_UNITS_512K 3032
#define MSG_PREP_UNITS_512K_STR "512K"

#define MSG_PREP_UNITS_2M 3033
#define MSG_PREP_UNITS_2M_STR "2M"

#define MSG_PREP_UNITS_UNKNOWN 3034
#define MSG_PREP_UNITS_UNKNOWN_STR "Auto Size"

#define MSG_PREP_UNITS_RESV 3035
#define MSG_PREP_UNITS_RESV_STR "Reserved"

#define MSG_PREP_TOTAL_SIZE 3036
#define MSG_PREP_TOTAL_SIZE_STR "%ld%s"

#define MSG_PREP_EDCC_NONE 3037
#define MSG_PREP_EDCC_NONE_STR "None"

#define MSG_PREP_EDCC_CHECKSUM 3038
#define MSG_PREP_EDCC_CHECKSUM_STR "Checksum"

#define MSG_PREP_EDCC_CRC 3039
#define MSG_PREP_EDCC_CRC_STR "CRC"

#define MSG_PREP_BKSZ_128 3040
#define MSG_PREP_BKSZ_128_STR "128"

#define MSG_PREP_BKSZ_256 3041
#define MSG_PREP_BKSZ_256_STR "256"

#define MSG_PREP_BKSZ_512 3042
#define MSG_PREP_BKSZ_512_STR "512"

#define MSG_PREP_BKSZ_1024 3043
#define MSG_PREP_BKSZ_1024_STR "1024"

#define MSG_PREP_BKSZ_2048 3044
#define MSG_PREP_BKSZ_2048_STR "2048"

#define MSG_PREP_TOTBK_FMT 3045
#define MSG_PREP_TOTBK_FMT_STR "%ld"

#define MSG_PREP_TOTBK_AUTO 3046
#define MSG_PREP_TOTBK_AUTO_STR "Auto Calculate"

#define MSG_PREP_TOTBK_SMALL 3047
#define MSG_PREP_TOTBK_SMALL_STR "Too small"

#define MSG_PREP_SPEED_FMT 3048
#define MSG_PREP_SPEED_FMT_STR "%ldns"

#define MSG_PREP_SIZE_FMT 3049
#define MSG_PREP_SIZE_FMT_STR "%ld Bytes"

#define MSG_PREP_DEVICE_LABEL 3050
#define MSG_PREP_DEVICE_LABEL_STR "Device"

#define MSG_PREP_FORMAT_LABEL 3051
#define MSG_PREP_FORMAT_LABEL_STR "Format"

#define MSG_PREP_GEO_BYTES 3052
#define MSG_PREP_GEO_BYTES_STR "Bytes/Block     : %9ld"

#define MSG_PREP_GEO_BLKS 3053
#define MSG_PREP_GEO_BLKS_STR "Total Blocks    : %9ld"

#define MSG_PREP_GEO_SECT 3054
#define MSG_PREP_GEO_SECT_STR "Sectors/Track   : %9ld"

#define MSG_PREP_GEO_TCYL 3055
#define MSG_PREP_GEO_TCYL_STR "Tracks/Cylinder : %9ld"

#define MSG_PREP_GEO_CYLS 3056
#define MSG_PREP_GEO_CYLS_STR "Total Cylinders : %9ld"

#define MSG_PREP_GEO_LABEL 3057
#define MSG_PREP_GEO_LABEL_STR "Geometry"

#define MSG_PREP_PROD_LABEL 3058
#define MSG_PREP_PROD_LABEL_STR "Product"

#define MSG_PREP_INFO_LABEL 3059
#define MSG_PREP_INFO_LABEL_STR "Information"

#define MSG_PREP_EMPTY_NAME 3060
#define MSG_PREP_EMPTY_NAME_STR "Empty"

#define MSG_PREP_TITLE_MENU 5000
#define MSG_PREP_TITLE_MENU_STR "Control"

#define MSG_PREP_ADV_MENU 5001
#define MSG_PREP_ADV_MENU_STR "Advanced Settings..."

#define MSG_PREP_QUIT_MENU 5002
#define MSG_PREP_QUIT_MENU_STR "Quit"

#define MSG_PREP_QUIT_COM 5003
#define MSG_PREP_QUIT_COM_STR "Q"

#endif /* PREPCARD */


/****************************************************************************/


#ifdef STRINGARRAY

struct AppString
{
    LONG   as_ID;
    STRPTR as_Str;
};

struct AppString AppStrings[] =
{
#ifdef PREPCARD
    {MSG_PREP_CANCEL_GAD,MSG_PREP_CANCEL_GAD_STR},
    {MSG_PREP_OK_GAD,MSG_PREP_OK_GAD_STR},
    {MSG_PREP_QUERY_GAD,MSG_PREP_QUERY_GAD_STR},
    {MSG_PREP_CONTINUE_GAD,MSG_PREP_CONTINUE_GAD_STR},
    {MSG_PREP_TITLE,MSG_PREP_TITLE_STR},
    {MSG_PREP_ADV_TITLE,MSG_PREP_ADV_TITLE_STR},
    {MSG_PREP_FORMAT_GAD,MSG_PREP_FORMAT_GAD_STR},
    {MSG_PREP_RAM_GAD,MSG_PREP_RAM_GAD_STR},
    {MSG_PREP_TYPE_GAD,MSG_PREP_TYPE_GAD_STR},
    {MSG_PREP_SPEED_GAD,MSG_PREP_SPEED_GAD_STR},
    {MSG_PREP_UNITS_GAD,MSG_PREP_UNITS_GAD_STR},
    {MSG_PREP_UNNUM_GAD,MSG_PREP_UNNUM_GAD_STR},
    {MSG_PREP_SIZE_GAD,MSG_PREP_SIZE_GAD_STR},
    {MSG_PREP_EDC_GAD,MSG_PREP_EDC_GAD_STR},
    {MSG_PREP_BKSZ_GAD,MSG_PREP_BKSZ_GAD_STR},
    {MSG_PREP_MAXB_GAD,MSG_PREP_MAXB_GAD_STR},
    {MSG_PREP_MAXE_GAD,MSG_PREP_MAXE_GAD_STR},
    {MSG_PREP_SECE_GAD,MSG_PREP_SECE_GAD_STR},
    {MSG_PREP_TRKE_GAD,MSG_PREP_TRKE_GAD_STR},
    {MSG_PREP_CYLE_GAD,MSG_PREP_CYLE_GAD_STR},
    {MSG_PREP_GEOE_GAD,MSG_PREP_GEOE_GAD_STR},
    {MSG_PREP_DINFO_GAD,MSG_PREP_DINFO_GAD_STR},
    {MSG_PREP_FINFO_GAD,MSG_PREP_FINFO_GAD_STR},
    {MSG_PREP_GINFO_GAD,MSG_PREP_GINFO_GAD_STR},
    {MSG_PREP_INSERT_PROMPT,MSG_PREP_INSERT_PROMPT_STR},
    {MSG_PREP_WARN_PROMPT,MSG_PREP_WARN_PROMPT_STR},
    {MSG_PREP_FORMAT_PROMPT,MSG_PREP_FORMAT_PROMPT_STR},
    {MSG_PREP_RAM_PROMPT,MSG_PREP_RAM_PROMPT_STR},
    {MSG_PREP_UI_PROMPT,MSG_PREP_UI_PROMPT_STR},
    {MSG_PREP_BUSY_PROMPT,MSG_PREP_BUSY_PROMPT_STR},
    {MSG_PREP_DONE_PROMPT,MSG_PREP_DONE_PROMPT_STR},
    {MSG_PREP_ERROR_ISRAM,MSG_PREP_ERROR_ISRAM_STR},
    {MSG_PREP_ERROR_NORES,MSG_PREP_ERROR_NORES_STR},
    {MSG_PREP_ERROR_INUSE,MSG_PREP_ERROR_INUSE_STR},
    {MSG_PREP_NO_CARD_ERROR,MSG_PREP_NO_CARD_ERROR_STR},
    {MSG_PREP_NO_MEM_ERROR,MSG_PREP_NO_MEM_ERROR_STR},
    {MSG_PREP_CORRUPT_ERROR,MSG_PREP_CORRUPT_ERROR_STR},
    {MSG_PREP_PSEUDO_WARN,MSG_PREP_PSEUDO_WARN_STR},
    {MSG_PREP_NA,MSG_PREP_NA_STR},
    {MSG_PREP_NOCARD_ERROR,MSG_PREP_NOCARD_ERROR_STR},
    {MSG_PREP_WP_ERROR,MSG_PREP_WP_ERROR_STR},
    {MSG_PREP_SMALL_ERROR,MSG_PREP_SMALL_ERROR_STR},
    {MSG_PREP_WRITE_ERROR,MSG_PREP_WRITE_ERROR_STR},
    {MSG_PREP_REMOVED_ERROR,MSG_PREP_REMOVED_ERROR_STR},
    {MSG_PREP_MINBLKS_ERROR,MSG_PREP_MINBLKS_ERROR_STR},
    {MSG_PREP_BATTERY_STATUS,MSG_PREP_BATTERY_STATUS_STR},
    {MSG_PREP_PINFO_STATUS,MSG_PREP_PINFO_STATUS_STR},
    {MSG_PREP_PREP_BUSY,MSG_PREP_PREP_BUSY_STR},
    {MSG_PREP_CARD_NULL,MSG_PREP_CARD_NULL_STR},
    {MSG_PREP_CARD_ROM,MSG_PREP_CARD_ROM_STR},
    {MSG_PREP_CARD_OTPROM,MSG_PREP_CARD_OTPROM_STR},
    {MSG_PREP_CARD_EPROM,MSG_PREP_CARD_EPROM_STR},
    {MSG_PREP_CARD_EEPROM,MSG_PREP_CARD_EEPROM_STR},
    {MSG_PREP_CARD_FLASH,MSG_PREP_CARD_FLASH_STR},
    {MSG_PREP_CARD_SRAM,MSG_PREP_CARD_SRAM_STR},
    {MSG_PREP_CARD_DRAM,MSG_PREP_CARD_DRAM_STR},
    {MSG_PREP_CARD_UNKNOWN,MSG_PREP_CARD_UNKNOWN_STR},
    {MSG_PREP_CARD_IO,MSG_PREP_CARD_IO_STR},
    {MSG_PREP_CARD_NOCARD,MSG_PREP_CARD_NOCARD_STR},
    {MSG_PREP_BATTERY_OK,MSG_PREP_BATTERY_OK_STR},
    {MSG_PREP_BATTERY_LOW,MSG_PREP_BATTERY_LOW_STR},
    {MSG_PREP_BATTERY_FAIL,MSG_PREP_BATTERY_FAIL_STR},
    {MSG_PREP_FORMAT_UNKNOWN,MSG_PREP_FORMAT_UNKNOWN_STR},
    {MSG_PREP_FORMAT_DISK,MSG_PREP_FORMAT_DISK_STR},
    {MSG_PREP_FORMAT_RAM,MSG_PREP_FORMAT_RAM_STR},
    {MSG_PREP_FORMAT_XIP,MSG_PREP_FORMAT_XIP_STR},
    {MSG_PREP_FORMAT_NOPREP,MSG_PREP_FORMAT_NOPREP_STR},
    {MSG_PREP_FORMAT_PSEUDO,MSG_PREP_FORMAT_PSEUDO_STR},
    {MSG_PREP_FORMAT_CRDMARK,MSG_PREP_FORMAT_CRDMARK_STR},
    {MSG_PREP_STANDARD,MSG_PREP_STANDARD_STR},
    {MSG_PREP_NONSTANDARD,MSG_PREP_NONSTANDARD_STR},
    {MSG_PREP_SPEED_250NS,MSG_PREP_SPEED_250NS_STR},
    {MSG_PREP_SPEED_200NS,MSG_PREP_SPEED_200NS_STR},
    {MSG_PREP_SPEED_150NS,MSG_PREP_SPEED_150NS_STR},
    {MSG_PREP_SPEED_100NS,MSG_PREP_SPEED_100NS_STR},
    {MSG_PREP_UNITS_512,MSG_PREP_UNITS_512_STR},
    {MSG_PREP_UNITS_2K,MSG_PREP_UNITS_2K_STR},
    {MSG_PREP_UNITS_8K,MSG_PREP_UNITS_8K_STR},
    {MSG_PREP_UNITS_32K,MSG_PREP_UNITS_32K_STR},
    {MSG_PREP_UNITS_128K,MSG_PREP_UNITS_128K_STR},
    {MSG_PREP_UNITS_512K,MSG_PREP_UNITS_512K_STR},
    {MSG_PREP_UNITS_2M,MSG_PREP_UNITS_2M_STR},
    {MSG_PREP_UNITS_UNKNOWN,MSG_PREP_UNITS_UNKNOWN_STR},
    {MSG_PREP_UNITS_RESV,MSG_PREP_UNITS_RESV_STR},
    {MSG_PREP_TOTAL_SIZE,MSG_PREP_TOTAL_SIZE_STR},
    {MSG_PREP_EDCC_NONE,MSG_PREP_EDCC_NONE_STR},
    {MSG_PREP_EDCC_CHECKSUM,MSG_PREP_EDCC_CHECKSUM_STR},
    {MSG_PREP_EDCC_CRC,MSG_PREP_EDCC_CRC_STR},
    {MSG_PREP_BKSZ_128,MSG_PREP_BKSZ_128_STR},
    {MSG_PREP_BKSZ_256,MSG_PREP_BKSZ_256_STR},
    {MSG_PREP_BKSZ_512,MSG_PREP_BKSZ_512_STR},
    {MSG_PREP_BKSZ_1024,MSG_PREP_BKSZ_1024_STR},
    {MSG_PREP_BKSZ_2048,MSG_PREP_BKSZ_2048_STR},
    {MSG_PREP_TOTBK_FMT,MSG_PREP_TOTBK_FMT_STR},
    {MSG_PREP_TOTBK_AUTO,MSG_PREP_TOTBK_AUTO_STR},
    {MSG_PREP_TOTBK_SMALL,MSG_PREP_TOTBK_SMALL_STR},
    {MSG_PREP_SPEED_FMT,MSG_PREP_SPEED_FMT_STR},
    {MSG_PREP_SIZE_FMT,MSG_PREP_SIZE_FMT_STR},
    {MSG_PREP_DEVICE_LABEL,MSG_PREP_DEVICE_LABEL_STR},
    {MSG_PREP_FORMAT_LABEL,MSG_PREP_FORMAT_LABEL_STR},
    {MSG_PREP_GEO_BYTES,MSG_PREP_GEO_BYTES_STR},
    {MSG_PREP_GEO_BLKS,MSG_PREP_GEO_BLKS_STR},
    {MSG_PREP_GEO_SECT,MSG_PREP_GEO_SECT_STR},
    {MSG_PREP_GEO_TCYL,MSG_PREP_GEO_TCYL_STR},
    {MSG_PREP_GEO_CYLS,MSG_PREP_GEO_CYLS_STR},
    {MSG_PREP_GEO_LABEL,MSG_PREP_GEO_LABEL_STR},
    {MSG_PREP_PROD_LABEL,MSG_PREP_PROD_LABEL_STR},
    {MSG_PREP_INFO_LABEL,MSG_PREP_INFO_LABEL_STR},
    {MSG_PREP_EMPTY_NAME,MSG_PREP_EMPTY_NAME_STR},
    {MSG_PREP_TITLE_MENU,MSG_PREP_TITLE_MENU_STR},
    {MSG_PREP_ADV_MENU,MSG_PREP_ADV_MENU_STR},
    {MSG_PREP_QUIT_MENU,MSG_PREP_QUIT_MENU_STR},
    {MSG_PREP_QUIT_COM,MSG_PREP_QUIT_COM_STR},
#endif /* PREPCARD */
};


#endif /* STRINGARRAY */


/****************************************************************************/


#endif /* PREPCARD_TEXT_H */
