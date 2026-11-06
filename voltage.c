/*!@addtogroup sources Source files*//*@{*/
/**
@file             voltage.c
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
                  
@brief            Functions to meassure battery and reference voltage.
                  
                  This module provides service-level functions to get the 
                  momentary and averaged battery voltage and the reference 
                  voltage.
*//*@}*/

/******************************************************************************/
/* Includes (#include)                                                        */
#include <avr/io.h>

/* Own headers */
#include "xtypes.h"
#include "global.h"

#include "adc.h"
#include "safety.h"

#define EXPORT
  #include "voltage.h"
#undef EXPORT

/******************************************************************************/
/* Constants (#define)                *//*!@addtogroup define Constants *//*@{*/

/**
@brief  Defines the number of voltage readings used for calculating the average
        voltage.
        
        Since voltage_UpdateAvg() will be called every 512ms the averaging 
        period is then (AVG_COUNT * 512ms = 20.48 seconds).                   */
#define AVG_COUNT   40U

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

/** @brief UINT16 variable to store the ADC value of reference voltage.       */
STATIC UINT16 mg_u16Ref                 = 0U;

/** @brief UINT16 variable to store the ADC value of battery voltage.         */
STATIC UINT16 mg_u16Value               = 0U;

/** @brief UINT16 variable to store the ADC values of battery voltage for 
    averaging.                                                                */
STATIC UINT16 mg_au16Data[AVG_COUNT];

/** @brief UINT16 variable to store averaged value of battery voltage.        */
STATIC UINT16 mg_u16Avg                 = 0U;

/** @brief UINT8 variable to keep the ring array index position.              */
STATIC UINT8  mg_u8Idx                  = 0U;

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
@brief  Init function of module <voltage.c>.
        
        This function reads the battery voltage and reference voltage to set up
        the according module variables.
@see    mg_u16Ref
@see    mg_u16Value
@see    mg_u16Avg
@return void
*/
/* ****************************************************************************/
void voltage_Init (void)
{
  UINT16 u16Help = adc_Read(ADC_VOLTAGE);
  
  mg_u16Ref   = (adc_Read(ADC_VREF)-695U) * 26/10;
  mg_u16Value = u16Help;
  
  for(UINT8 i=0U; i<AVG_COUNT; i++)
  {
    mg_au16Data[i] = u16Help;
  }
  mg_u16Avg = u16Help;
  mg_u8Idx = 0U;
}


/* ****************************************************************************/
/**
@brief  Read and update ADC values for battery voltage and reference voltage.
        
        This function reads the battery voltage and reference voltage.
@see    mg_u16Ref
@see    mg_u16Value
@return void
*/
/* ****************************************************************************/
void voltage_Update (void)
{
  mg_u16Ref   = (adc_Read(ADC_VREF)-695U) * 26/10;
  mg_u16Value = adc_Read(ADC_VOLTAGE);
}


/* ****************************************************************************/
/**
@brief  Reads the battery voltage and recalculates the average value.
        
        This function will be called every 512ms and reads the battery voltage
        and recalculates the average voltage value.
        The battery voltage will be read and stored in the ring array 
        mg_au16Data at index mg_u8Idx.
        After this, the index position will be incremented.
        Finally the new average value will be calculated from all array values
        and stored in mg_u16Avg.
@see    mg_u16Avg
@see    mg_au16Data
@see    AVG_COUNT
@see    mg_u8Idx
@return void
*/
/* ****************************************************************************/
void voltage_UpdateAvg (void)
{
  UINT16 u16Help = adc_Read(ADC_VOLTAGE);
  
  /* insert voltage value in ring array */
  mg_au16Data[mg_u8Idx] = u16Help;
  
  /* step to next position in array */
  mg_u8Idx++;
  if(mg_u8Idx >= AVG_COUNT)
  {
    mg_u8Idx = 0U;
  }
  
  /* calculate new average value */
  u16Help = 0U;
  for(UINT8 i=0U; i<AVG_COUNT; i++)
  {
    u16Help += mg_au16Data[i];
  }
  u16Help /= AVG_COUNT;
  mg_u16Avg = u16Help;
  
  if(mg_u16Avg < mg_u16Ref)
  {
    safety_SetCondition(COND_VOLT_ERR);
  }
  else
  {
    safety_ClearCondition(COND_VOLT_ERR);
  }
}


/* ****************************************************************************/
/**
@brief  Returns the battery voltage.
        
        This function returns the ADC value for battery voltage stored 
        in mg_u16Value.
@see    mg_u16Value
@return mg_u16Value (UINT16)
@retval 0..1023
*/
/* ****************************************************************************/
UINT16 voltage_GetValue (void)
{
  return (mg_u16Value);
}


/* ****************************************************************************/
/**
@brief  Returns average battery voltage.
        
        This function returns the average ADC value for battery voltage stored 
        in mg_u16Avg.
@see    mg_u16Avg
@return mg_u16Avg (UINT16)
@retval 0..1023
*/
/* ****************************************************************************/
UINT16 voltage_GetAvg (void)
{
  return (mg_u16Avg);
}


/* ****************************************************************************/
/**
@brief  Returns reference voltage.
        
        This function returns the ADC value for the reference voltage stored 
        in mg_u16Ref.
@see    mg_u16Ref
@return mg_u16Ref (UINT16)
@retval 0..1023
*/
/* ****************************************************************************/
UINT16 voltage_GetRef (void)
{
  return (mg_u16Ref);
}


/* End: Global functions                                                *//*@}*/
/******************************************************************************/

/**** Last line of code                                                    ****/
