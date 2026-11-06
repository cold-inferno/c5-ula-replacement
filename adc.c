/*!@addtogroup sources Source files*//*@{*/
/**
@file             adc.c
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
                  
@brief            This file contains the implementation of analog-digital 
                  comparator functions.
                  
                  This module provides a common interface to other modules to
                  read analog voltages from ADC input pins.
*//*@}*/

/******************************************************************************/
/* Includes (#include)                                                        */
#include <avr/io.h>
#include <util/delay.h>

/* Own header files */
#include "xtypes.h"
#include "global.h"

#define EXPORT
  #include "adc.h"
#undef EXPORT

/******************************************************************************/
/* Constants (#define)                *//*!@addtogroup define Constants *//*@{*/

/** @brief Presetting value for ADC multiplexer                               */
#define AD_REF_EXT      0x00U

/** @brief This value defines how many ADC values will be read and averaged for 
    1 measure                                                                 */
#define AD_READIN_COUNT 8   /* must be <= 64 (2^10 * AD_READIN_COUNT <= 2^16) */

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
@brief  Init function of module <adc.c>.
        
        This function sets the analog-in pins to inputs and configures the ADC.
        
        This function should be called before any other functions of this 
        module.
        
@return void
*/
/* ****************************************************************************/
void adc_Init (void)
{
  SWITCH_DDR  &= ~(1U << SWITCH_BIT);
  VTH_DDR     &= ~(1U << VTH_BIT);
  R3_DDR      &= ~(1U << R3_BIT);
  CSBASE_DDR  &= ~(1U << CSBASE_BIT);
  VREF_DDR    &= ~(1U << VREF_BIT);

  ADMUX  = AD_REF_EXT;
  ADCSRA =  (0<<ADEN)   /* AD Enable */
          | (0<<ADSC)   /* start conversion */
       // | (0<<ADFR)   /* free run */
          | (0<<ADIE)   /* interrupt enable */
          | (1<<ADPS2)
          | (1<<ADPS1)
          | (1<<ADPS0);
} /* End: adc_Init() */


/* ****************************************************************************/
/**
@brief  Reads an averaged value from an ADC channel.
        
@param u8Channel UINT8 index of ADC channel [0..7]
@return retval (UINT16)
@retval 0..1023 (10 bit ADC value)
*/
/* ****************************************************************************/
UINT16 adc_Read (UINT8 u8Channel)
{
  UINT8 i;
  UINT16 retval;
  
  ADMUX   = AD_REF_EXT | u8Channel; /* select channel */
  ADCSRA |= (1<<ADEN);              /* activate ADC */
  
  /* dummy read */
  ADCSRA |= (1<<ADSC);              /* read ADC */
  while (ADCSRA & (1<<ADSC))
  {
    asm("nop");                     /* wait for conversion end */
  }
  retval = ADCW;
  
  /* actual meassure (averaged) */
  retval = 0U; 
  for(i=0U; i<AD_READIN_COUNT; i++)
  {
    ADCSRA |= (1<<ADSC);            /* read ADC */
    while (ADCSRA & (1<<ADSC))
    {
      asm("nop");                   /* wait for conversion end */
    }
    retval += ADCW;                 /* add results */
  }
  
  ADCSRA &= ~(1<<ADEN);             /* deactivate ADC */
  
  retval /= AD_READIN_COUNT;        /* calculate average value */
 
  return retval;
} /* End: adc_Read() */


/* End: Global functions                                                *//*@}*/
/******************************************************************************/

/**** Last line of code                                                    ****/


