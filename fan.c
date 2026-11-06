/*!@addtogroup sources Source files*//*@{*/
/**
@file             fan.c
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
                  
@brief            This file contains the methods to control a 5V fan, connected 
                  to the "emitter" pin of the C5 instrument pod.
*//*@}*/

/******************************************************************************/
/* Includes (#include)                                                        */
#include <avr/io.h>

/* Own header files */
#include "xtypes.h"
#include "global.h"

#define EXPORT
  #include "fan.h"
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
@brief  Init function of module <fan.c>.
        
        This function sets configures the IO pin connected to the fan as an 
        output pin.
        
@return void
*/
/* ****************************************************************************/
void fan_Init (void)
{
  FAN_DDR  |=  (1U << FAN_BIT);
  
  fan_Set(OFF);
} /* End: fan_Init() */


/* ****************************************************************************/
/**
@brief  Method to activate or deactivate the fan.
        
@param  eState E_SWITCH new fan state (ON or OFF)
@return void
*/
/* ****************************************************************************/
void fan_Set (E_SWITCH eState)
{
  if(ON == eState)
  {
    FAN_PORT |=  (1U << FAN_BIT);
  }
  else
  {
    FAN_PORT &= ~(1U << FAN_BIT);
  }
} /* End: fan_Set() */


/* End: Global functions                                                *//*@}*/
/******************************************************************************/

/**** Last line of code                                                    ****/


