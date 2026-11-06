/*!@addtogroup sources Source files*//*@{*/
/**
@file             ula_repl.c
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
                  
@brief            Application module containing the main() entry point.
                  
                  This module contains the main() function which is the 
                  application firmware entry point.
                  
                  All not-interrupt-triggered operations are handled inside the
                  application loop in main().
*//*@}*/

/******************************************************************************/
/* Includes (#include)                                                        */
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>

/* Own header files */
#include "xtypes.h"
#include "global.h"

#include "adc.h"
#include "fan.h"
#include "init.h"
#include "temp.h"
#include "timer.h"
#include "pedal.h"
#include "speed.h"
#include "relay.h"
#include "safety.h"
#include "memory.h"
#include "speaker.h"
#include "voltage.h"
#include "current.h"



#define EXPORT
  #include "ula_repl.h"
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

/** @brief E_BOOL variable to hold pedelec mode                               */
STATIC E_BOOL mg_bIsPedelec         = TRUE;

/** @brief UINT16 variable to keep maximum speed limit (for motor enabling)   */
STATIC UINT16 mg_u16SpeedLimitMax   = SPEEDLIMIT_MAX;

/** @brief UINT16 variable to keep minimum speed limit (for motor enabling)   */
STATIC UINT16 mg_u16SpeedLimitMin   = SPEEDLIMIT_MIN;

/** @brief UINT16 variable to keep fan activation temperature */
STATIC UINT16 mg_u16FanTemp         = VALUE_TEMP_FAN;

/** @brief UINT8 variable to keep the number of tacho reed switch magnets */
STATIC UINT8  mg_u8NumMagnetsTacho  = DEFAULT_MAGNET_NUMBER_TACHO;

/** @brief UINT8 variable to keep the number of pedal reed switch magnets */
STATIC UINT8  mg_u8NumMagnetsPedal  = DEFAULT_MAGNET_NUMBER_PEDAL;

/* End: Local variables                                                 *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Global variables           *//*!@addtogroup globvar Global variables *//*@{*/
/* End: Global variables                                                *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Prototype declarations                                                     */
       int    main            (void);
STATIC E_BOOL mg_RelayControl (void);
/* End: Prototype declarations                                                */
/******************************************************************************/

/******************************************************************************/
/* Local functions             *//*!@addtogroup locfunc Local functions *//*@{*/


/* ****************************************************************************/
/**
@brief  Main entry function. Executes the application loop.
        
        This function is the program entry point.
        After setting up the hardware, initiating all software modules and 
        displaying firmware information on serial out the function will
        enter the application loop.
        What operations the application loop executes:
        
          Always:
            - updating momentary voltage, current and average temperature data
            - check for speed and pedal sensor signal timeouts
            - decide to enable or disable the motor relay
            - perform memory test
          
          Every 256ms:
            - reset watchdog
            - display information on serial out
          
          Every 512ms:
            - calculate average battery voltage and motor load
          
          Every 1024ms:
            - check motor temperature for overheating
          
        The application loop never returns.
        
@return int
@retval 1
*/
/* ****************************************************************************/
int main (void)
{
  UINT8   u8Help = 0U;
  
  E_BOOL  bMotorEnabled = FALSE;
  
  /* init IO ports */
  init_InitIO();
  
  /* init modules */
  cli();
  init_InitModules();
  sei(); 
  
  ALL_LEDS_OFF();
  
  /* get settings from EEPROM memory */
  mg_bIsPedelec         = memory_Get()->bIsPedelec;
  mg_u16SpeedLimitMax   = memory_Get()->u16SpeedLimitMax;
  mg_u16SpeedLimitMin   = memory_Get()->u16SpeedLimitMin;
  mg_u16FanTemp         = memory_Get()->u16TempFan;
  mg_u8NumMagnetsTacho  = memory_Get()->u8NumMagnetsTacho;
  mg_u8NumMagnetsPedal  = memory_Get()->u8NumMagnetsPedal;
  
  /* print firmware version on serial out */
  printf("ULA replacement V%d\n\n", pgm_read_word(MEMPOS_VER));
  printf("CRC: 0x%08lX\n\n",        safety_GetCRC());
  
  /* print settings on serial out */
  printf("PM:  %d\n",   mg_bIsPedelec);
  printf("WS:  %umm\n", memory_Get()->u16WheelSize);
  printf("LMX: %u\n",   mg_u16SpeedLimitMax);
  printf("LMN: %u\n",   mg_u16SpeedLimitMin);
  printf("FT:  %u\n",   mg_u16FanTemp);
  printf("NMT: %u\n",   mg_u8NumMagnetsTacho);
  printf("NMP: %u\n\n", mg_u8NumMagnetsPedal);
  
  /* put relay and speaker to default conditions */
  relay_Enable(FALSE);
  speaker_Off();
  
  /* reset watchdog before entering the main loop */
  wdt_reset();
  
  // zum testen: while(1) { asm("nop"); }
  
  /* application loop */
  while (1)
  {
    /* lock the timer bits during this while loop cycle */
    timer_LockTicks();
    
    /* meassure momentary voltage and current values */
    voltage_Update();
    current_Update();
    
    /* meassure the motor temperature */
    temp_Update();
    
    /* every 1024ms: check temperature for overheating */
    if(TIMER_IS_TICK1024())
    {
      temp_Check();
    }
    
    /* activate the fan if the average temperature exeeds or equals the
       configurated temperature threshold */
    if(temp_GetCelsius(temp_GetAvg()) >= mg_u16FanTemp)
    {
      fan_Set(ON);
    }
    
    /* check every 10240ms if we can deactivate the fan */
    if(TIMER_IS_TICK1024())
    {
      u8Help++;
      if(u8Help >= 10U)
      {
        u8Help = 0U;
        if(temp_GetCelsius(temp_GetAvg()) < mg_u16FanTemp)
        {
          /* fan may be deactivated only if there is no temperature 
             error condition set */
          if((safety_GetCondition() & COND_TEMP_ERR) == 0U)
          {
            fan_Set(OFF);
          }
        }
      }
    }
    
    /* every 512ms: calculate average voltage and current draw */
    if(TIMER_IS_TICK512())
    {
      voltage_UpdateAvg();
      current_UpdateSum();
    }
    
    /* every 256ms: update average motor current */
    if(TIMER_IS_TICK128())
    {
      current_UpdateAvg();
    }
    
    /* always check for speed and pedal signal timeouts */
    speed_CheckTimeout(timer_GetSystemTime());
    pedal_GetTimeout();
    
    /* after all checks and data updates above, we are ready to determine 
       wether the relay has to be enabled or disabled */
    bMotorEnabled = mg_RelayControl();
    
    /* Memory test.
       The memory range to be tested is byte address 0 to 7164 (3582 words).
       Note: This function will not return in case of a memory failure. */
    safety_MemoryTest();
    
    /* every 256ms: handle further jobs */
    if(TIMER_IS_TICK256())
    {
      /* reset watchdog */
      wdt_reset();
      
      /* print information on serial out */
      printf("#");
      printf("%lu|", speed_GetDistance());    // distance
      printf("%lu|", SPEED_GET());            // speed (10th of km/h)
      printf("%lu|", pedal_GetRpm());         // upm
      printf("%lu|", pedal_GetTimeout());     // timeout
      printf("%u|",  adc_Read(ADC_SWITCH));   // switch
      printf("%u|",  temp_GetAvg());          // vth
      printf("%u|",  voltage_GetValue());     // voltage
      printf("%u|",  voltage_GetRef());       // vref
      printf("%u|",  current_GetValue());     // current
      printf("%u|",  bMotorEnabled);          // motor enabled?
      printf("%u|",  safety_GetCondition());  // condition
      printf("%lu|", timer_GetSystemTime());  // timestamp
      printf("\n");
    }
    
  } /* end of: while(1) application loop */
  
  return (1); /* never reached */
  
} /* End: main() */


/* ****************************************************************************/
/**
@brief  Enables or disables the motor.
        
        This function checks wether the motor relay can be enabled or must be
        disabled depending on safety conditions, motor load and pedelec
        settings.
        The function returns TRUE if the motor is enabled or FALSE if not.
        
@return bRet (E_BOOL)
@retval TRUE, FALSE
*/
/* ****************************************************************************/
E_BOOL mg_RelayControl (void)
{
  E_BOOL bRet = TRUE;
  
  /* motor will be disabled if any error condition occours */
  if(safety_GetCondition() >= COND_VOLT_ERR)
  {
    bRet = FALSE;
  }
  /* motor will be disabled if momentary motor load is too high */
  else if(current_GetValue() >= MAX_CURRENT)
  {
    bRet = FALSE;
  }
  /* motor will be disabled if pedelec settings require it */
  else if (FALSE != mg_bIsPedelec)
  {
    if (SPEED_GET() < mg_u16SpeedLimitMin)
    {
      bRet = FALSE;
    }
    else
    {
      if(SPEED_GET() > mg_u16SpeedLimitMax)
      {
        bRet = FALSE;
      }
      else
      {
        if (pedal_GetTimeout() >= PEDAL_TIMEOUT)
        {
          bRet = FALSE;
        }
      }
    }
  }
  
  /* enable/disable motor relay */
  if(FALSE == bRet)
  {
    relay_Enable(FALSE);
  }
  else
  {
    relay_Enable(TRUE);
  }
  
  return (bRet);
} /* End: mg_RelayControl() */



/* End: Local functions                                                 *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Global functions          *//*!@addtogroup globfunc Global functions *//*@{*/
/* End: Global functions                                                *//*@}*/
/******************************************************************************/

/**** Last line of code                                                    ****/