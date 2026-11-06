/*!@addtogroup sources Source files*//*@{*/
/**
@file             pedal.c
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
                  
@brief            Pedal ISR and methods to get pedaling frequency.
                  
                  This module contains an interrupt service routine for the 
                  pedal sensor signal and methods to calculate the pedaling 
                  frequency.
*//*@}*/

/******************************************************************************/
/* Includes (#include)                                                        */
#include <avr/io.h>
#include <avr/interrupt.h>

/* Own header files */
#include "xtypes.h"
#include "global.h"

#include "timer.h"
#include "memory.h"

#define EXPORT
  #include "pedal.h"
#undef EXPORT

/******************************************************************************/
/* Constants (#define)                *//*!@addtogroup define Constants *//*@{*/
/* End: Constants (#define)                                             *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Macro definitions (#define)           *//*!@addtogroup macros Macros *//*@{*/

/** @brief This macro enables the pedal interrupt                             */
#define PEDAL_INT_ENABLE()  EIMSK |=  (1<<INT1);

/** @brief This macro enables the pedal interrupt                             */
#define PEDAL_INT_DISABLE() EIMSK &= ~(1<<INT1);

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

/** @brief Variable to keep a timestamp of the last pedal interrupt           */
STATIC VOLATILE UINT32 mg_u32TickTime;

/** @brief This variable holds the value of pedal revolutions per minute      */
STATIC VOLATILE UINT32 mg_u32Rpm;

/** @brief UINT8 variable to keep the number of pedal reed switch magnets */
STATIC UINT8  mg_u8NumMagnetsPedal  = DEFAULT_MAGNET_NUMBER_PEDAL;
/** @brief 2026: shortest accepted pulse distance (debounce) in ms           */
STATIC UINT16 mg_u16MinPeriod       = PEDAL_MIN_PERIOD_MS;
/** @brief 2026: TRUE after the first valid pedal pulse since power-on       */
STATIC VOLATILE E_BOOL mg_bSeenPulse = FALSE;

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

/* ****************************************************************************/
/**
@brief  Service routine for pedal sensor interrupt.

        Every time this function is called, a timestamp will be stored and 
        compared with the last timestamp. The difference between now and the 
        last timestamp is used to calculate the pedal revolutions per minute.
        
@see mg_u32TickTime
@see mg_u32Rpm
@return void
*/
/* ****************************************************************************/
ISR (INT1_vect)
{
  LOCAL_STATIC  UINT32  u32CurrentTime;
                UINT32  u32DiffTime;
  
  PEDAL_INT_DISABLE();
  
  u32CurrentTime = timer_GetSystemTime();
  u32DiffTime = u32CurrentTime - mg_u32TickTime;
  /* 2026: ignore contact bounce of the reed switch */
  if(u32DiffTime >= mg_u16MinPeriod)
  {
    mg_u32TickTime = u32CurrentTime;
    if(FALSE != mg_bSeenPulse)
    {
      mg_u32Rpm = ONE_MINUTE_MS / u32DiffTime / mg_u8NumMagnetsPedal;
    }
    mg_bSeenPulse = TRUE;
  }
  
  PEDAL_INT_ENABLE();
}

/* End: Local functions                                                 *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Global functions          *//*!@addtogroup globfunc Global functions *//*@{*/

/* ****************************************************************************/
/**
@brief  Init function of the pedal module.
        
@see mg_u32TickTime
@see mg_u32Rpm
@return void
*/
/* ****************************************************************************/
void pedal_Init (void)
{
  mg_u8NumMagnetsPedal  = memory_Get()->u8NumMagnetsPedal;
  if((0U == mg_u8NumMagnetsPedal) || (0xFFU == mg_u8NumMagnetsPedal))
  {
    mg_u8NumMagnetsPedal = 1U;  // 2026: avoid division by 0 / erased EEPROM
  }
  mg_u16MinPeriod = PEDAL_MIN_PERIOD_MS / mg_u8NumMagnetsPedal;
  mg_bSeenPulse   = FALSE;
  
  INT1_DDR  &= ~(1U << INT1_BIT);   /* set ext. interrupt pin as input pin */
  
  /* setup for falling edge */
  EICRA |=  (1<<ISC11);
  EICRA &= ~(1<<ISC10);
  
  mg_u32TickTime  = 0UL;
  mg_u32Rpm       = 0UL;
  
  /* enable interrupt */
  PEDAL_INT_ENABLE();
}


/* ****************************************************************************/
/**
@brief  Check and get method for pedal timeout.

        When no more pedal revolutions occur, the value mg_u32Rpm would stay as
        it is.
        To prevent from this behaviour, this function should be called at 
        regular intervals.
        It will read the current system time (see timer_GetSystemTime()) and
        compare it with the last timestamp in mg_u32TickTime.
        If the difference between both values exeeds PEDAL_TIMEOUT, the 
        value for mg_u32Rpm will be reset to 0.
        
        The time difference between now and mg_u32TickTime will be returned.
        
@see timer_GetSystemTime
@return Time difference between now and mg_u32TickTime (in ms)
@retval UINT32
*/
/* ****************************************************************************/
UINT32 pedal_GetTimeout (void)
{
  UINT32 u32SysTime;
  UINT32 u32TickTime;
  UINT32 u32Ret;
  E_BOOL bSeen;

  u32SysTime = timer_GetSystemTime();
  cli();
  u32TickTime = mg_u32TickTime;
  bSeen       = mg_bSeenPulse;
  sei();

  u32Ret = u32SysTime - u32TickTime;
  /* 2026: before the first pulse after power-on the driver is not pedalling
     (previously the first 2 seconds after power-on counted as pedalling) */
  if((u32Ret > PEDAL_TIMEOUT) || (FALSE == bSeen))
  {
    cli();
    mg_u32Rpm = 0UL;
    sei();
    u32Ret = PEDAL_TIMEOUT;
  }
  
  return(u32Ret);
}


/* ****************************************************************************/
/**
@brief  Get method for mg_u32Rpm.

        When calling this function it could be interrupted by the pedal ISR. 
        Hence, we need to disable interrupts for a short moment to
        buffer the mg_u32Rpm value in another UINT32 variable.
        
@return Value of mg_u32Rpm
@retval UINT32
*/
/* ****************************************************************************/
UINT32 pedal_GetRpm (void)
{
  UINT32 u32Ret;
  cli();
  u32Ret = mg_u32Rpm;
  sei();
  return (u32Ret);
}


/* End: Global functions                                                *//*@}*/
/******************************************************************************/

/**** Last line of code                                                    ****/
