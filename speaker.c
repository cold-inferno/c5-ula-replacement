/*!@addtogroup sources Source files*//*@{*/
/**
@file             speaker.c
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
                  
@brief            This file contains the methods to control the buzzer.
*//*@}*/

/******************************************************************************/
/* Includes (#include)                                                        */
#include <avr/io.h>

/* Own header files */
#include "xtypes.h"
#include "global.h"

#define EXPORT
  #include "speaker.h"
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
void speaker_Init (void)
{
  SPEAKER_DDR  |= (1U << SPEAKER_BIT);
  
  SPEAKER_PORT |= (1U << SPEAKER_BIT);
} /* End: speaker_Init() */


/* ****************************************************************************/
/**
@brief  Method to deactivate the buzzer.
        
@return void
*/
/* ****************************************************************************/
void speaker_Off (void)
{
  SPEAKER_PORT |= (1U << SPEAKER_BIT);
} /* End: speaker_Off() */


/* ****************************************************************************/
/**
@brief  Method to toggle the buzzer IO pin causing a short clicking noise.
    
    If this function is executed continuously very fast, a buzzing sound is 
    generated.
        
@return void
*/
/* ****************************************************************************/
void speaker_Buzz (void)
{
  SPEAKER_PORT ^= (1U << SPEAKER_BIT);
} /* End: speaker_Buzz() */

/* End: Global functions                                                *//*@}*/
/******************************************************************************/

/**** Last line of code                                                    ****/


