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
#include "print.h"
#include "pedelec.h"
#include "leds.h"
#include "switch.h"
#include "ridelog.h"



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

/** @brief 2026: motor relay decision state                                  */
STATIC S_PEDELEC_STATE mg_sPedelec;

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
STATIC void   mg_LogInput     (S_RL_INPUT* psIn, E_BOOL bMotorEnabled);
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
  PRINT_P("ULA replacement V");
  print_U32(pgm_read_word(MEMPOS_VER));
  PRINT_P("\n\nCRC: 0x");
  print_Hex32(safety_GetCRC());
  PRINT_P("\n\nBAT: " BATTERY_NAME "\n");
  
  /* print settings on serial out */
  PRINT_P("PM:  ");  print_U32(mg_bIsPedelec);
  PRINT_P("\nWS:  "); print_U32(memory_Get()->u16WheelSize);
  PRINT_P("mm\nLMX: "); print_U32(mg_u16SpeedLimitMax);
  PRINT_P("\nLMN: "); print_U32(mg_u16SpeedLimitMin);
  PRINT_P("\nFT:  "); print_U32(mg_u16FanTemp);
  PRINT_P("\nNMT: "); print_U32(mg_u8NumMagnetsTacho);
  PRINT_P("\nNMP: "); print_U32(mg_u8NumMagnetsPedal);
  PRINT_P("\nLOG: "); print_U32(memory_Get()->u8LogEnable == 1U ? 1U : 0U);
  PRINT_P("\n\n");
  
  /* put relay and speaker to default conditions */
  relay_Enable(FALSE);
  pedelec_Init(&mg_sPedelec, timer_GetSystemTime());

  /* 2026: optional ride log (reset cause, events, black box) */
  ridelog_Init();
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
      /* 2026: left LED bar shows speed while the motor button is released,
         motor current while it is pressed */
      leds_SetLeftBar((ON == switch_Get()) ? TRUE : FALSE,
                      pedelec_SpeedBarLevel(SPEED_GET()));
      {
        S_RL_INPUT sLog;
        mg_LogInput(&sLog, bMotorEnabled);
        ridelog_Check(&sLog);
        if(TIMER_IS_TICK1024())
        {
          ridelog_Sample(&sLog);   /* black box: one sample per second */
        }
      }
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
      /* same format as V1172, plus the resting voltage (2026) at the end:
         #DIST|SPEED|RPM|TIMEOUT|SWITCH|TEMP|VOLT|VREF|CURR|ENA|COND|TIME|REST| */
      UINT32 au32Out[13];
      au32Out[0]  = speed_GetDistance();    // distance
      au32Out[1]  = SPEED_GET();            // speed (10th of km/h)
      au32Out[2]  = pedal_GetRpm();         // upm
      au32Out[3]  = pedal_GetTimeout();     // timeout
      au32Out[4]  = adc_Read(ADC_SWITCH);   // switch
      au32Out[5]  = temp_GetAvg();          // vth
      au32Out[6]  = voltage_GetValue();     // voltage
      au32Out[7]  = voltage_GetRef();       // vref
      au32Out[8]  = current_GetValue();     // current
      au32Out[9]  = bMotorEnabled;          // motor enabled?
      au32Out[10] = safety_GetCondition();  // condition
      au32Out[11] = timer_GetSystemTime();  // timestamp
      au32Out[12] = voltage_GetRest();      // resting voltage (2026)
      print_Char('#');
      for(UINT8 i = 0U; i < 13U; i++)
      {
        print_U32(au32Out[i]);
        print_Char('|');
      }
      print_Char('\n');
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
  S_PEDELEC_INPUT sIn;
  E_BOOL bRet;

  sIn.u32Now       = timer_GetSystemTime();
  /* motor will be disabled immediately on any error condition or if the
     momentary motor load is too high */
  sIn.bSafetyCut   = ((safety_GetCondition() >= COND_VOLT_ERR) ||
                      (current_GetValue() >= MAX_CURRENT)) ? TRUE : FALSE;
  sIn.bPedelecMode = mg_bIsPedelec;
  sIn.bPedalling   = (pedal_GetTimeout() < PEDAL_TIMEOUT) ? TRUE : FALSE;
  sIn.u32Speed     = SPEED_GET();
  sIn.u16SpeedMin  = mg_u16SpeedLimitMin;
  sIn.u16SpeedMax  = mg_u16SpeedLimitMax;

  /* 2026: decision with hysteresis and minimum dwell time, see pedelec.c */
  bRet = pedelec_Decide(&mg_sPedelec, &sIn);

  relay_Enable(bRet);

  return (bRet);
} /* End: mg_RelayControl() */



/** @brief 2026: collects the current measurements for the ride log.        */
void mg_LogInput (S_RL_INPUT* psIn, E_BOOL bMotorEnabled)
{
  psIn->u32Speed   = SPEED_GET();
  psIn->u16Volt    = voltage_GetValue();
  psIn->u16Curr    = current_GetValue();
  psIn->u16TempAdc = temp_GetAvg();
  psIn->u8TempC    = (UINT8)temp_GetCelsius(psIn->u16TempAdc);
  psIn->u8Cond     = (UINT8)safety_GetCondition();
  psIn->u8Flags    = 0U;
  if (FALSE != bMotorEnabled)                    { psIn->u8Flags |= RL_F_RELAY; }
  if (ON == switch_Get())                        { psIn->u8Flags |= RL_F_BUTTON; }
  if (pedal_GetTimeout() < PEDAL_TIMEOUT)        { psIn->u8Flags |= RL_F_PEDALLING; }
}

/* End: Local functions                                                 *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Global functions          *//*!@addtogroup globfunc Global functions *//*@{*/
/* End: Global functions                                                *//*@}*/
/******************************************************************************/

/**** Last line of code                                                    ****/