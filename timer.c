/*!@addtogroup sources Source files*//*@{*/
/**
@file             timer.c
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
                  
@brief            Contains system timer ISR for exact time base.
                  
                  This module provides an exact time base for time relevant
                  calculations (revolutions per minute, distance, timeouts) and
                  some flags to execute specific operations at regular 
                  intervals.
*//*@}*/

/******************************************************************************/
/* Includes (#include)                                                        */
#include <avr/io.h>   //debug
#include <avr/interrupt.h>

/* Own headers */
#include "xtypes.h"
#include "global.h"

#include "leds.h"
#include "speed.h"
#include "safety.h"
#include "speaker.h"

#define EXPORT
  #include "timer.h"
#undef EXPORT

/******************************************************************************/
/* Constants (#define)                *//*!@addtogroup define Constants *//*@{*/

/** @brief Preload value for system timer                                     */
#define OCR_VALUE     ((F_CPU / 64U) / 1000U) - 1U

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

/** @brief Variable to hold the system time in ms                             */
STATIC VOLATILE UINT32 mg_u32SystemTime  = 0U;

/** @brief Variable to keep time flags                                        */
STATIC VOLATILE UINT8  mg_u8Ticks        = NO_TICK;

/** @brief Variable used to share time flags with other modules               */
         STATIC UINT8  mg_u8TicksOut     = NO_TICK;

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
@brief  Service routine for timer interrupt.

        Every time this function is called, the system time will be incremented.
        
        If the system time is divisible by specified numbers, flags bits in 
        mg_u8Ticks are set.
        This mechanism allows to execute actions at distinct intervals.
        
        Furthermore, this timer ISR is used to multiplex the LED displays and
        to activate the buzzer in case of an error or warning condition.
        
@see mg_u8Ticks
@see mg_u32SystemTime
@see leds_ShowVoltage()
@see leds_ShowCurrent()
@return void
*/
/* ****************************************************************************/
ISR (TIMER2_COMPA_vect)
{
  LOCAL_STATIC UINT8 u8Toggle = 0U;
  
  mg_u32SystemTime++;
  
  /* multiplexing the LEDs */
  u8Toggle++;
  if((u8Toggle & 0x01) != 0U)
  {
    leds_ShowVoltage();
  }
  else
  {
    leds_ShowCurrent();
  }
  
  if(0UL == (mg_u32SystemTime & 1023UL))
  {
    mg_u8Ticks |= (TICK64 | TICK256 | TICK512 |TICK1024);
  }
  else if(0UL == (mg_u32SystemTime & 511UL))
  {
    mg_u8Ticks |= (TICK64 | TICK256 | TICK512);
  }
  else if(0UL == (mg_u32SystemTime & 255UL))
  {
    mg_u8Ticks |= (TICK64 | TICK256);
  }
  else if(0UL == (mg_u32SystemTime & 63UL))
  {
    mg_u8Ticks |= (TICK64);
  }
  
  /* activate the speaker in case of a current or temperature condition.
     V1203: same rhythm as the fault LEDs: fast beeps for an error, slow
     beeps for a warning (battery faults stay silent, as before). */
  {
    UINT8  u8Cond = (UINT8)safety_GetCondition();
    UINT16 u16Bit = 0U;

    if((u8Cond & (COND_CURR_ERR | COND_TEMP_ERR)) != 0U)
    {
      u16Bit = LED_FLASH_ERR;
    }
    else if((u8Cond & (COND_CURR_WARN | COND_TEMP_WARN)) != 0U)
    {
      u16Bit = LED_FLASH_WARN;
    }

    if(0U == u16Bit)
    {
      speaker_Off();
    }
    else if(((UINT16)mg_u32SystemTime & u16Bit) == 0U)
    {
      speaker_Buzz();
    }
  }
}

/* End: Local functions                                                 *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Global functions          *//*!@addtogroup globfunc Global functions *//*@{*/

/* ****************************************************************************/
/**
@brief  Timer module init function.

        This function configures and activates the timer used for system time.
        
@return void
*/
/* ****************************************************************************/
void timer_Init (void)
{
  // Timer2 einstellen
  TCCR2A  = (1<<WGM21);
  TCCR2B  = (1<<CS22);
  TCNT2   = 0U;
  OCR2A   = OCR_VALUE;
  TIMSK2 |= (1<<OCIE2A);
}


/* ****************************************************************************/
/**
@brief  Get method for mg_u8TicksOut.

        Returns the value of mg_u8TicksOut. Since this is a 8bit variable, no 
        interrupt locking is required.
        
@return Value of mg_u8TicksOut
@retval UINT8
*/
/* ****************************************************************************/
UINT8 timer_GetTicks (void)
{
  return (mg_u8TicksOut);
}


/* ****************************************************************************/
/**
@brief  Function to create a copy of the time flags.

        Copies the time flags in mg_u8Ticks to mg_u8TicksOut and resets
        all flags in mg_u8Ticks.
        Sinc this process must not be interrupted, all interrupts are 
        temporarily disabled.
        
@return void
*/
/* ****************************************************************************/
void timer_LockTicks (void)
{
  cli();
  mg_u8TicksOut = mg_u8Ticks;
  mg_u8Ticks    = NO_TICK;
  sei();
}


/* ****************************************************************************/
/**
@brief  Get method for mg_u32SystemTime.

        When calling this function it could be that it is interrupted by the
        timer ISR. Hence, we need to disable interrupts for a short moment to
        buffer the mg_u32SystemTime value in another UINT32 variable.
        
@return Value of mg_u32Rpm
@retval UINT32
*/
/* ****************************************************************************/
UINT32 timer_GetSystemTime(void)
{
  UINT32 u32Ret;
  /* 2026: save/restore the interrupt flag instead of cli()/sei(). This
     function is also called from inside ISRs (pedal, tacho, LED multiplexing),
     where an unconditional sei() re-enabled interrupts and allowed nesting. */
  UINT8  u8Sreg = SREG;
  cli();
  u32Ret = mg_u32SystemTime;
  SREG = u8Sreg;
  return (u32Ret);
}

/* End: Global functions                                                *//*@}*/
/******************************************************************************/

/**** Last line of code                                                    ****/
