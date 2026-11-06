/*!@addtogroup sources Source files*//*@{*/
/**
@file             leds.c
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
                  
@brief            Methods to control voltage and current display.
                  
                  This module provides functions to control the voltage and 
                  current LED displays.
                  
                  Since you cannot light up both displays at the same time, 
                  another function (see timer.c) will be required to call 
                  leds_ShowVoltage() and leds_ShowCurrent() alternately.
                  
                  Multiplexing doesn't require exernal means but is done in 
                  this functions.
*//*@}*/

/******************************************************************************/
/* Includes (#include)                                                        */
#include <avr/io.h>

/* Own header files */
#include "xtypes.h"
#include "global.h"


#include "timer.h"
#include "safety.h"
#include "voltage.h"
#include "current.h"
#include "pedelec.h"

#define EXPORT
  #include "leds.h"
#undef EXPORT

/******************************************************************************/
/* Constants (#define)                *//*!@addtogroup define Constants *//*@{*/

/* 2026: voltage LED thresholds VOLT_LED1..5 now come from the battery
   profile in global.h */

/* Current definitions - don't change this unless you know what you're doing! */

/** @brief ADC value for 20 Ampere                                            */
#define CURR_20A        75U

/** @brief ADC value for 15 Ampere                                            */
#define CURR_15A        56U

/** @brief ADC value for 10 Ampere                                            */
#define CURR_10A        37U

/** @brief ADC value for 5 Ampere                                             */
#define CURR_5A         17U

/** @brief ADC value for 2 Ampere                                             */
#define CURR_2A         8U

/** @brief ADC value for 0 Ampere                                             */
#define CURR_0A         0U

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
@brief  Init function to initiate related hardware.

        This function is used to initiate LED related I/O pins and to start the
        display multiplexing.
        
        This function should be called before any other functions of this 
        module.
        
@return void
*/
/** @brief 2026: TRUE while the motor button is pressed (set from main loop) */
STATIC VOLATILE E_BOOL mg_bButton    = FALSE;

/** @brief 2026: speed bar level 0..5 or SPEEDBAR_LEVEL_FLASH (from main loop)*/
STATIC VOLATILE UINT8  mg_u8SpeedLvl = 0U;

/**
@brief  2026: tells the display what the left (current) bar shall show.
        Called from the main loop; the LED multiplexing itself runs in the
        timer ISR and must not read the ADC or the speed sensor directly.
@param  bButtonPressed  TRUE: motor button pressed -> show motor current
@param  u8SpeedLevel    speed bar level from pedelec_SpeedBarLevel()
*/
void leds_SetLeftBar (E_BOOL bButtonPressed, UINT8 u8SpeedLevel)
{
  mg_u8SpeedLvl = u8SpeedLevel;
  mg_bButton    = bButtonPressed;
}

/* ****************************************************************************/
void leds_Init (void)
{
  LED1_DDR |= (1U << LED1_BIT);
  LED2_DDR |= (1U << LED2_BIT);
  LED3_DDR |= (1U << LED3_BIT);
  LED4_DDR |= (1U << LED4_BIT);
  LED5_DDR |= (1U << LED5_BIT);
  
  MUX_VOLT_DDR |= (1U << MUX_VOLT_BIT);
  MUX_CURR_DDR |= (1U << MUX_CURR_BIT);
  
  LED1_OFF();
  LED2_OFF();
  LED3_OFF();
  LED4_OFF();
  LED5_OFF();
  
  MUX_VOLT();
}


/* ***************************************************************************/
/**
@brief  Method to display the battery voltage.

        This function switches the multiplexer to the voltage display, 
        retrieves the average battery voltage and shows the value with the LED
        bar graph.
        
        In case of a voltage error condition all (voltage) LEDs will be 
        flashing.
        
@return void
*/
/* ****************************************************************************/
void leds_ShowVoltage(void)
{
  UINT16 u16Help;
  
  u16Help = voltage_GetDisplay();
  
  MUX_VOLT();

  if((safety_GetCondition() & COND_VOLT_ERR) != 0U)
  {
    /* all LED flashing in case of a voltage error condidtion */
    if((timer_GetSystemTime() & 256UL) == 0UL)
    {
      ALL_LEDS_ON();
    }
    else
    {
      ALL_LEDS_OFF();
    }
  }
  else 
  {
    if(u16Help >= VOLT_LED1)
    {
      LED1_ON();
    }
    else
    {
      LED1_OFF();
    }
    
    if(u16Help >= VOLT_LED2)
    {
      LED2_ON();
    }
    else
    {
      LED2_OFF();
    }
    
    if(u16Help >= VOLT_LED3)
    {
      LED3_ON();
    }
    else
    {
      LED3_OFF();
    }
    
    if(u16Help >= VOLT_LED4)
    {
      LED4_ON();
    }
    else
    {
      LED4_OFF();
    }
    
    if(u16Help >= VOLT_LED5)
    {
      LED5_ON();
    }
    else
    {
      LED5_OFF();
    }
  }
}


/* ***************************************************************************/
/**
@brief  Method to display the motor current.

        This function switches the multiplexer to the current display, 
        retrieves the recent current sum and shows the value with the LED
        bar graph.
        
        In case of a current warning or error condition some LEDs will be
        flashing.
        
@return void
*/
/* ****************************************************************************/
void leds_ShowCurrent(void)
{
  UINT16 u16Help;
  
  u16Help = current_GetAvg();
  
  MUX_CURR();
  
  if(safety_GetCondition() != COND_OK)
  {
    if((timer_GetSystemTime() & 256UL) == 0UL)
    {
      if ((safety_GetCondition() & COND_VOLT_WARN) != 0U)
      {
        /* LED5 indicates a voltage warning */
        LED5_ON();
      }
      
      if ((safety_GetCondition() & COND_VOLT_ERR) != 0U)
      {
        /* LED5 will also be flashing on voltage error */
        LED5_ON();
      }
      
      if ((safety_GetCondition() & COND_CURR_WARN) != 0U)
      {
        /* LED1 indicates a current warning */
        LED5_ON();
      }
      
      if ((safety_GetCondition() & COND_CURR_ERR) != 0U)
      {
        /* LED2 indicates a current error */
        LED2_ON();
      }
      
      if ((safety_GetCondition() & COND_TEMP_WARN) != 0U)
      {
        /* LED3 indicates a temperature warning */
        LED3_ON();
      }
      
      if ((safety_GetCondition() & COND_TEMP_ERR) != 0U)
      {
        /* LED4 indicates a temperature error */
        LED4_ON();
      }
    }
    else
    {
      ALL_LEDS_OFF();
    }
  }
  else if(FALSE == mg_bButton)
  {
    /* 2026: motor button released -> show the speed, one LED per 5 km/h,
       top LED flashing above SPEEDBAR_FLASH */
    UINT8 u8Lvl = mg_u8SpeedLvl;
    if(u8Lvl >= 1U) { LED1_ON(); } else { LED1_OFF(); }
    if(u8Lvl >= 2U) { LED2_ON(); } else { LED2_OFF(); }
    if(u8Lvl >= 3U) { LED3_ON(); } else { LED3_OFF(); }
    if(u8Lvl >= 4U) { LED4_ON(); } else { LED4_OFF(); }
    if((u8Lvl == 5U) ||
       ((u8Lvl == SPEEDBAR_LEVEL_FLASH) &&
        ((timer_GetSystemTime() & 256UL) == 0UL)))
    {
      LED5_ON();
    }
    else
    {
      LED5_OFF();
    }
  }
  else
  {
    /* motor button pressed -> show the motor current (original behaviour) */
    if(u16Help >= CURR_20A)
    {
      LED5_ON();
    }
    else
    {
      LED5_OFF();
    }
    
    if(u16Help >= CURR_15A)
    {
      LED4_ON();
    }
    else
    {
      LED4_OFF();
    }
    
    if(u16Help >= CURR_10A)
    {
      LED3_ON();
    }
    else
    {
      LED3_OFF();
    }
    
    if(u16Help >= CURR_5A)
    {
      LED2_ON();
    }
    else
    {
      LED2_OFF();
    }
    
    if(u16Help > CURR_0A)
    {
      LED1_ON();
    }
    else
    {
      LED1_OFF();
    }
  }
}

/* End: Global functions                                                *//*@}*/
/******************************************************************************/

/**** Last line of code                                                    ****/


