/*!@addtogroup sources Source files*//*@{*/
/**
@file             switch.c
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
                  
@brief            Contains a service-level function to read the motor switch.
*//*@}*/

/******************************************************************************/
/* Includes (#include)                                                        */
#include <avr/io.h>

/* Own header files */
#include "xtypes.h"
#include "global.h"

#include "adc.h"

#define EXPORT
  #include "switch.h"
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
@brief  Init function for this module.

        This function is empty and currently just provided for the sake of 
        consistency with other modules.
        
@return void
*/
/* ****************************************************************************/
void switch_Init (void)
{
  /* Nothing to do here */
}


/* ****************************************************************************/
/**
@brief  Get method for motor switch condition.

        This function reads the ADC channel assigned to the motor switch.
        If the switch is pressed ON will be returned; otherwise OFF.
        
@see adc_Read()
@return Condition of the motor switch
@retval E_SWITCH (ON or OFF)
*/
/* ****************************************************************************/
E_SWITCH switch_Get (void)
{
  UINT16 u16Help = adc_Read(ADC_SWITCH);
  if (u16Help > 0U)
  {
    return (ON);
  }
  else
  {
    return (OFF);
  }
}

/* End: Global functions                                                *//*@}*/
/******************************************************************************/

/**** Last line of code                                                    ****/