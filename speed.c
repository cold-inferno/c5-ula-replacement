/*!@addtogroup sources Source files*//*@{*/
/**
@file             speed.c
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
                  
@brief            Speed sensor ISR and methods to get tacho information.
                  
                  This module contains an interrupt service routine for the 
                  tacho sensor signal and methods to calculate speed and 
                  trip distance.
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
  #include "speed.h"
#undef EXPORT

/******************************************************************************/
/* Constants (#define)                *//*!@addtogroup define Constants *//*@{*/

/** @brief This value defines the maximum reliable speed. 
           Used to detect false tacho ticks.                                  */
#define MAX_RELIABLE_SPEED  70U

/* End: Constants (#define)                                             *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Macro definitions (#define)           *//*!@addtogroup macros Macros *//*@{*/

/** @brief This macro enables the tacho interrupt                             */
#define SPEED_INT_ENABLE()  EIMSK |=  (1<<INT0);

/** @brief This macro disables the tacho interrupt                            */
#define SPEED_INT_DISABLE() EIMSK &= ~(1<<INT0);
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

/** @brief Variable to hold a timestamp of the last tacho interrupt           */
STATIC VOLATILE UINT32 mg_u32TickTime;

/** @brief This variable holds the value of wheel revolutions per minute      */
STATIC VOLATILE UINT32 mg_u32Rpm;

/** @brief This variable holds the value of trip distance im mm               */
STATIC VOLATILE UINT32 mg_u32Distance;

/** @brief This variable holds a copy value of wheel circumference in mm      */
STATIC          UINT16 mg_u16WheelSize;

/** @brief UINT8 variable to keep the number of tacho reed switch magnets */
STATIC UINT8  mg_u8NumMagnetsTacho  = DEFAULT_MAGNET_NUMBER_TACHO;
/** @brief Full wheel circumference in mm (as configured in EEPROM)          */
STATIC          UINT16 mg_u16WheelSizeFull;

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
@brief  Service routine for tacho sensor interrupt.

        Every time this function is called, a timestamp will be stored and 
        compared with the last timestamp. The difference between now and the 
        last timestamp is used to calculate the wheel revolutions per minute.
        
        Also the trip distance will be incremented by calling this ISR.
        
@see mg_u32TickTime
@see mg_u32Rpm
@see mg_u32Distance
@return void
*/
/* ****************************************************************************/
ISR (INT0_vect)
{
  LOCAL_STATIC  UINT32  u32CurrentTime;
                UINT32  u32DiffTime;
  
  SPEED_INT_DISABLE();
  
  u32CurrentTime = timer_GetSystemTime();
  u32DiffTime = u32CurrentTime - mg_u32TickTime;
  
  /* Maybe there will be false sensor signals from keybouncing.
    To filter these out we use a maximum speed definition that gives a
    minimum diff time between two sensor signals:
    
                wheel_size[mm] * 3.6
    --------------------------------------------
    max_reliable_speed[km/h] * number_of_magnets
    
    And to avoid floating point numbers we sustitute 3.6 with 36/10.
  */
  if(u32DiffTime >= ( (mg_u16WheelSize * 36U) / 
                      (MAX_RELIABLE_SPEED * 10U * mg_u8NumMagnetsTacho) )
                    )
  {
    mg_u32TickTime = u32CurrentTime;
    mg_u32Rpm      = ONE_MINUTE_MS / u32DiffTime / mg_u8NumMagnetsTacho;
    mg_u32Distance += mg_u16WheelSize;
  }
  
  SPEED_INT_ENABLE();
}

/* End: Local functions                                                 *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Global functions          *//*!@addtogroup globfunc Global functions *//*@{*/

/* ****************************************************************************/
/**
@brief  Init function for this module.

        This function initiates the variables for timestamp, tacho RMP and trip
        distance and configures the tacho interrupt.
        
@see mg_u32TickTime
@see mg_u32Rpm
@see mg_u32Distance
@return void
*/
/* ****************************************************************************/
void speed_Init (void)
{
  mg_u8NumMagnetsTacho  = memory_Get()->u8NumMagnetsTacho;
  if(0U == mg_u8NumMagnetsTacho)
  {
    mg_u8NumMagnetsTacho = 1U;  // to avoid division by 0
  }
  
  INT0_DDR  &= ~(1U << INT0_BIT);   /* set ext. interrupt pin as input pin */
  
  /* the effective wheel size is the actual wheel size divided by the number of
     magnets */
  mg_u16WheelSizeFull = memory_Get()->u16WheelSize;
  if((0U == mg_u16WheelSizeFull) || (0xFFFFU == mg_u16WheelSizeFull))
  {
    mg_u16WheelSizeFull = WHEEL_SIZE;  // erased/invalid EEPROM: use default
  }
  mg_u16WheelSize = ( mg_u16WheelSizeFull / mg_u8NumMagnetsTacho );
  
  /* setup for falling edge */
  EICRA |=  (1<<ISC01);
  EICRA &= ~(1<<ISC00);
  
  mg_u32TickTime  = 0U;
  mg_u32Rpm       = 0U;
  mg_u32Distance  = 0U;
  
  /* enable interrupt */
  SPEED_INT_ENABLE();
}

/* ****************************************************************************/
/**
@brief  Get method for mg_u32Rpm.

        When calling this function it could be interrupted by the tacho ISR. 
        Hence, we need to disable interrupts for a short moment to
        buffer the mg_u32Rpm value in another UINT32 variable.
        
@return Value of mg_u32Rpm
@retval UINT32
*/
/* ****************************************************************************/
UINT32 speed_GetRpm (void)
{
  UINT32 u32Ret;
  cli();
  u32Ret = mg_u32Rpm;
  sei();
  return (u32Ret);
}


/* ****************************************************************************/
/**
@brief  Get method for mg_u32Distance.

        When calling this function it could be, that it is interrupted by the
        tacho ISR. hence, we need to disable interrupts for a short moment to
        buffer the mg_u32Distance value in another UINT32 variable.
        
@return Value of mg_u32Distance
@retval UINT32
*/
/* ****************************************************************************/
UINT32 speed_GetDistance (void)
{
  UINT32 u32Ret;
  cli();
  u32Ret = mg_u32Distance;
  sei();
  return (u32Ret);
}


/* ****************************************************************************/
/**
@brief  Check and get method for tacho timeout.

        When no more wheel revolutions occur, the value mg_u32Rpm would stay as
        it is.
        To prevent from this behaviour, this function should be called at 
        regular intervals.
        It will compare the given timestamp with the last timestamp in 
        mg_u32TickTime.
        If the difference between both values exeeds SPEED_TIMEOUT, the 
        value for mg_u32Rpm will be reset to 0.
        
@return void
*/
/* ****************************************************************************/
UINT32 speed_GetSpeed (void)
{
  /* wheel rpm * circumference [mm] * 60 min/h / 1e6 mm/km * 10 = 10th km/h */
  return (speed_GetRpm() * mg_u16WheelSizeFull * 6UL / 10000UL);
}

void speed_CheckTimeout (UINT32 u32SystemTime)
{
  UINT32 u32TickTime;
  cli();
  u32TickTime = mg_u32TickTime;
  sei();
  if(u32SystemTime >= (u32TickTime + SPEED_TIMEOUT))
  {
    cli();
    mg_u32Rpm = 0U;
    sei();
  }
}

/* End: Global functions                                                *//*@}*/
/******************************************************************************/

/**** Last line of code                                                    ****/
