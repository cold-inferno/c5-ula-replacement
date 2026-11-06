/*!@addtogroup sources Source files*//*@{*/
/**
@file             global.c
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
                  
@brief            Contains firmware version and CRC32 checksum sections.
*//*@}*/

/******************************************************************************/
/* Includes (#include)                                                        */

/* Own header files */
#include "xtypes.h"

#define EXPORT
  #include "global.h"
#undef EXPORT

/******************************************************************************/
/* Constants (#define)                *//*!@addtogroup define Constants *//*@{*/
/* End: Constants (#define)                                             *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Macro definitions (#define)           *//*!@addtogroup macros Macros *//*@{*/
/* End: Macro definitions (#define)                                     *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Basic types (typedef)                 *//*!@addtogroup typedef Types *//*@{*/
/* End: Basic types (typedef)                                           *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Local constants (const)      *//*!@addtogroup loccon Local constants *//*@{*/
/* End: Local constants (const)                                         *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Local variables              *//*!@addtogroup locvar Local variables *//*@{*/
/* End: Local variables                                                 *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Global variables           *//*!@addtogroup globvar Global variables *//*@{*/
#ifndef BOOTLOADER

/** @brief UINT16 variable to store the application firmware version at a 
    dedicated memory position.                                                */
CONST UINT16 g_u16ApplicationVersion SECTION_VERSION  = FW_VERSION;

/** @brief UINT32 variable to store the firmware checksum at a dedicated memory 
    position.                                                                 */
CONST UINT32 g_u32ApplicationCRC     SECTION_CRC      = APPLICATION_CRC;
#endif

/** @brief Variable to reserve memory for structure in EEPROM                 */
CONST S_MEM_STRUCTRUE sMemoryEEPROM SECTION_EEMEM = 
{
  TRUE, 
  WHEEL_SIZE, 
  SPEEDLIMIT_MAX,
  SPEEDLIMIT_MIN,
  VALUE_TEMP_FAN,
  DEFAULT_MAGNET_NUMBER_TACHO,
  DEFAULT_MAGNET_NUMBER_PEDAL
};

/* End: Global variables                                                *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Prototype declarations                                                     */
/* End: Prototype declarations                                                */
/******************************************************************************/

/******************************************************************************/
/* Local functions             *//*!@addtogroup locfunc Local functions *//*@{*/
/* End: Local functions                                                 *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Global functions          *//*!@addtogroup globfunc Global functions *//*@{*/
/* End: Global functions                                                *//*@}*/
/******************************************************************************/

/**** Last line of code                                                    ****/