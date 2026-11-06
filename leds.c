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

#define EXPORT
  #include "leds.h"
#undef EXPORT

/******************************************************************************/
/* Constants (#define)                *//*!@addtogroup define Constants *//*@{*/

/* Voltage definitions - don't change this unless you know what you're doing! */

/** @brief ADC value for 10.0 Volt                                            */
#define VOLT_10V        448U

/** @brief ADC value for 10.5 Volt                                            */
#define VOLT_10V5       472U

/** @brief ADC value for 11.0 Volt                                            */
#define VOLT_11V        496U

/** @brief ADC value for 11.5 Volt                                            */
#define VOLT_11V5       520U

/** @brief ADC value for 12.0 Volt                                            */
#define VOLT_12V        544U

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
  
  u16Help = voltage_GetAvg();
  
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
    if(u16Help >= VOLT_10V)
    {
      LED1_ON();
    }
    else
    {
      LED1_OFF();
    }
    
    if(u16Help >= VOLT_10V5)
    {
      LED2_ON();
    }
    else
    {
      LED2_OFF();
    }
    
    if(u16Help >= VOLT_11V)
    {
      LED3_ON();
    }
    else
    {
      LED3_OFF();
    }
    
    if(u16Help >= VOLT_11V5)
    {
      LED4_ON();
    }
    else
    {
      LED4_OFF();
    }
    
    if(u16Help >= VOLT_12V)
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
  else
  {
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


