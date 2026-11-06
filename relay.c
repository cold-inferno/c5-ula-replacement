/*!@addtogroup sources Source files*//*@{*/
/**
@file             relay.c
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
                  
@brief            This file contains the methods to enable or disable the motor
                  relay.
*//*@}*/

/******************************************************************************/
/* Includes (#include)                                                        */
#include <avr/io.h>

/* Own header files */
#include "xtypes.h"
#include "global.h"

#define EXPORT
  #include "relay.h"
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
@brief  Init function of module <relay.c>.
        
        This function sets configures the IO pin connected to the motor relay as
        an output pin.
        
@return void
*/
/* ****************************************************************************/
void relay_Init (void)
{
  RELAY_DDR  |=  (1U << RELAY_BIT);
  
  relay_Enable(FALSE);
} /* End: relay_Init() */


/* ****************************************************************************/
/**
@brief  Method to enable or disable the motor relay.
        
@param  bEnabled E_BOOL relay enabled or not (TRUE or FALSE)
@return void
*/
/* ****************************************************************************/
void relay_Enable (E_BOOL bEnabled)
{
  if(TRUE == bEnabled)
  {
    RELAY_PORT |=  (1U << RELAY_BIT);
  }
  else
  {
    RELAY_PORT &= ~(1U << RELAY_BIT);
  }
} /* End: relay_Enable() */


/* End: Global functions                                                *//*@}*/
/******************************************************************************/

/**** Last line of code                                                    ****/


