/*!@addtogroup sources Source files*//*@{*/
/**
@file             init.c
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
                  
@brief            This file contains the sources of hardware configuration and 
                  module initialization functions.
                  
                  This module provides a service-level interface to setup pin
                  configuration and to initiate all software modules.
*//*@}*/

/******************************************************************************/
/* Includes (#include)                                                        */
#include <avr/io.h>

/* Own header files */
#include "xtypes.h"
#include "global.h"

#include "adc.h"
#include "fan.h"
#include "uart.h"
#include "temp.h"
#include "print.h"
#include "timer.h"
#include "pedal.h"
#include "relay.h"
#include "speed.h"
#include "safety.h"
#include "switch.h"
#include "memory.h"
#include "speaker.h"
#include "voltage.h"
#include "current.h"

#define EXPORT
  #include "init.h"
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
@brief  Method to initiate all application modules.
        
        This function just calls all module initiation functions.
        
@return void
*/
/* ****************************************************************************/
void init_InitModules (void)
{
  speaker_Init();
  relay_Init();
  fan_Init();
  memory_Init();
  adc_Init();
  timer_Init();
  temp_Init();
  pedal_Init();
  speed_Init();
  switch_Init();
  voltage_Init();
  current_Init();
  uart_Init();
  print_Init();
  safety_Init();
}

/* ****************************************************************************/
/**
@brief  Method to pre-initiate I/O pins.
        
        This function is used to pre-initiate the controller I/O pins.
        
        This settings may be overwritten later through other functions/modules.
        
@return void
*/
/* ****************************************************************************/
void init_InitIO (void)
{
  /* IO initialisation */
  DDRB = 0x00U;
  DDRC = 0x00U;
  DDRD = 0x00U;
  TXD_DDR       |=  (1U << TXD_BIT);
  LED1_DDR      |=  (1U << LED1_BIT);
  LED2_DDR      |=  (1U << LED2_BIT);
  LED3_DDR      |=  (1U << LED3_BIT);
  LED4_DDR      |=  (1U << LED4_BIT);
  LED5_DDR      |=  (1U << LED5_BIT);
  MUX_VOLT_DDR  |=  (1U << MUX_VOLT_BIT);
  MUX_CURR_DDR  |=  (1U << MUX_CURR_BIT);
  SPEAKER_DDR   |=  (1U << SPEAKER_BIT);
  RELAY_DDR     |=  (1U << RELAY_BIT);
  FAN_DDR       |=  (1U << FAN_BIT);

  PORTB = 0x00U;
  PORTC = 0x00U;
  PORTD = 0x00U;
}

/* End: Global functions                                                *//*@}*/
/******************************************************************************/

/**** Last line of code                                                    ****/


