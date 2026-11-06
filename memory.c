/*!@addtogroup sources Source files*//*@{*/
/**
@file             memory.c
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
                  
@brief            Methods to read and write the EEPROM memory.
*//*@}*/


/******************************************************************************/
/* Includes (#include)                                                        */
#include <avr/eeprom.h>

/* Own header files */
#include "xtypes.h"
#include "global.h"

#define EXPORT
  #include "memory.h"
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
/* Local constants (const)     *//*!@addtogroup loccon  Local constants *//*@{*/
/* End: Local constants (const)                                         *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Local variables              *//*!@addtogroup locvar Local variables *//*@{*/

/** @brief Variable to keep application settings (wheel size, etc.)           */
STATIC S_MEM_STRUCTRUE  mg_sMemory;

/* End: Local variables                                                 *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Global variables           *//*!@addtogroup globvar Global variables *//*@{*/
/* End: Global variables                                                *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Prototype declarations                                                     */
STATIC void mg_Load      (void);
/* End: Prototype declarations                                                */
/******************************************************************************/

/******************************************************************************/
/* Local functions             *//*!@addtogroup locfunc Local functions *//*@{*/

/* ****************************************************************************/
/**
@brief  Module function to load settings from EEPROM.

        This function loads some settings (wheel size, pedelec mode,...) from
        EEPROM memory to the mg_sMemory structure.
        
@see mg_sMemory
@return void
*/
/* ****************************************************************************/
void mg_Load (void)
{
  eeprom_read_block(&mg_sMemory,
                    &(sMemoryEEPROM),
                    sizeof(S_MEM_STRUCTRUE));
}

/* End: Local functions                                                 *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Global functions          *//*!@addtogroup globfunc Global functions *//*@{*/

/* ***************************************************************************/
/**
@brief  Init function to load settings from EEPROM.

        This function calls mg_Load() to retieve settings (wheel size, 
        pedelec mode,...) from EEPROM memory.
        
@see mg_sMemory
@return void
*/
/* ****************************************************************************/
void memory_Init (void)
{
  mg_Load();
}


/* ***************************************************************************/
/**
@brief  Get function for mg_sMemory.
        
        The application can only read the values in mg_sMemory while the 
        bootloader will be able to edit it.
        
@see mg_sMemory
@return S_MEM_STRUCTRUE*
@retval Pointer to mg_sMemory
*/
/* ****************************************************************************/
CONST S_MEM_STRUCTRUE* memory_Get (void)
{
  return (&mg_sMemory);
}

/* End: Global functions                                                *//*@}*/
/******************************************************************************/

/**** Last line of code                                                    ****/


