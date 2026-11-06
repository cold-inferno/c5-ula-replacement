/*!@addtogroup sources Source files*//*@{*/
/**
@file             safety.c
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
                  
@brief            Safety condition handling and (flash) memory tests.
                  
                  This module provides a common interface to handle safety 
                  conditions and functionality to perform memory tests.
*//*@}*/

/******************************************************************************/
/* Includes (#include)                                                        */
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

/* Own headers */
#include "xtypes.h"
#include "global.h"

#include "fan.h"
#include "relay.h"

#define EXPORT
  #include "safety.h"
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

/** @brief Variable to hold the current safety status.                        */
STATIC E_SAFETY_CONDITION mg_eCondition = COND_OK;

/** @brief Index variable for flash memory testing. Equates a byte address.   */
STATIC UINT16             mg_u16MemIdx  = 0U;

/** @brief Variable to hold the calculated memory CRC value.                  */
STATIC UINT32             mg_u32CalcCRC = 0xFFFFFFFFUL;

/** @brief Variable to hold the stored memory CRC value. Will be set only once
    in the init function of this module.                                      */
STATIC UINT32             mg_u32ApplCRC = 0x00000000UL;

/* End: Local variables                                                 *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Global variables           *//*!@addtogroup globvar Global variables *//*@{*/
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

/* ****************************************************************************/
/**
@brief  Init function of safety module.

        This function reads the application CRC value from flash memory for 
        later comparison and activates the watchdog.
        
@return void
*/
/* ****************************************************************************/
void safety_Init (void)
{
  fan_Set(OFF);
  mg_u16MemIdx  = 0U;
  mg_u32CalcCRC = 0xFFFFFFFFUL;
  
  /* read CRC */
  mg_u32ApplCRC = pgm_read_word(MEMPOS_CRC+2U);
  mg_u32ApplCRC <<= 16U;
  mg_u32ApplCRC += pgm_read_word(MEMPOS_CRC);
  
  /* activate watchdog */
  wdt_enable(WDTO_500MS);
  wdt_reset();
}


/* ****************************************************************************/
/**
@brief  Set method for mg_eCondition.

        If a temperature error condition is set, the fan will be activated.
        
@param  eCond E_SAFETY_CONDITION condition to be set.
@return void
*/
/* ****************************************************************************/
void safety_SetCondition (E_SAFETY_CONDITION eCond)
{
  mg_eCondition |= eCond;
  
  if((mg_eCondition & COND_TEMP_ERR) != 0U)
  {
    fan_Set(ON);
  }
}


/* ****************************************************************************/
/**
@brief  Clear method for mg_eCondition.

        Will be called to clear a distinct condition flag.
        
@param  eCond E_SAFETY_CONDITION condition to be cleared.
@return void
*/
/* ****************************************************************************/
void safety_ClearCondition (E_SAFETY_CONDITION eCond)
{
  mg_eCondition &= ~eCond;
}


/* ****************************************************************************/
/**
@brief  Get method for mg_u32ApplCRC.
        
@return mg_u32ApplCRC
@retval UINT32
*/
/* ****************************************************************************/
UINT32 safety_GetCRC(void)
{
  return (mg_u32ApplCRC);
}


/* ****************************************************************************/
/**
@brief  Get method for mg_eCondition.
        
@return mg_eCondition
@retval E_SAFETY_CONDITION
*/
/* ****************************************************************************/
E_SAFETY_CONDITION safety_GetCondition(void)
{
  return (mg_eCondition);
}


/* ****************************************************************************/
/**
@brief  Continue flash memory test.
        
        This function reads the next byte from the flash memory at index in 
        mg_u16MemIdx.
        
        The byte value is used to recalculate the flash memory CRC.
        When reaching the end of monitored memory area, the index variable is
        reset to 0 to start again and the calculated CRC will be compared with
        the stored CRC value.
        
        In case of a CRC divergency, safety_SafeMode() will be called.
        
@see    safety_SafeMode()
@return void
*/
/* ****************************************************************************/
void safety_MemoryTest (void)
{
  UINT8   u8Help;
  UINT8   u8Idx;
  
  u8Help = pgm_read_byte(mg_u16MemIdx);
  
  /* Calculate CRC */
  for(u8Idx = 8U; u8Idx; u8Idx--)
  {
    if(((mg_u32CalcCRC & 0x80000000) ? 1U : 0U) != (u8Help & 1U))
    {
      mg_u32CalcCRC = (mg_u32CalcCRC << 1U) ^ CRC32POLY;
    }
    else
    {
      mg_u32CalcCRC <<= 1U;
    }
    u8Help >>= 1U;
  }
  
  mg_u16MemIdx++;
  
  if(mg_u16MemIdx >= MEMPOS_CRC)
  {
    mg_u32CalcCRC ^= 0xFFFFFFFFUL;
    
    /* compare CRC values */
    if(mg_u32ApplCRC != mg_u32CalcCRC)
    {
      safety_SafeMode(); /* this function never returns */
    }
    
    mg_u16MemIdx  = 0U;
    mg_u32CalcCRC = 0xFFFFFFFFUL;
  }
}


/* ****************************************************************************/
/**
@brief  Safe mode of entire system.
        
        This function represents a safe mode for the entire software system.
        
        If this function is called, a fatal error has occured.
        In this case normal software operation cannot be restored.
        
        Tis function deactivates all interrupts, activates the fan and 
        continuously disables the motor relay.
        
@return void
*/
/* ****************************************************************************/
void safety_SafeMode (void)
{
  mg_eCondition |= COND_VOLT_WARN;
  mg_eCondition |= COND_CURR_WARN;
  mg_eCondition |= COND_TEMP_WARN;
  mg_eCondition |= COND_VOLT_ERR;
  mg_eCondition |= COND_CURR_ERR;
  mg_eCondition |= COND_TEMP_ERR;
  cli();
  while(1U) /* loop forever */
  {
    relay_Enable(FALSE);
    fan_Set(ON);
    wdt_reset(); /* watchdog must be triggered to prevent from restarting */
  }
}


/* End: Global functions                                                *//*@}*/
/******************************************************************************/

/**** Last line of code                                                    ****/
