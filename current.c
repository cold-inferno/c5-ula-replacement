/*!@addtogroup sources Source files*//*@{*/
/**
@file             current.c
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
                  
@brief            Functions to meassure motor current and current draw sum.
                  
                  This module provides service-level functions to get the 
                  momentary motor current load, as well as the recent current
                  sum.
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
  #include "current.h"
#undef EXPORT

/******************************************************************************/
/* Constants (#define)                *//*!@addtogroup define Constants *//*@{*/

/** @brief This value defines the array size of mg_au16Data and thus the span of time 
    used to calculate the motor current sum.
    @see mg_au16Data                                                          */
#define SUM_COUNT   20U

/** @brief This value defines the array size of mg_au16AvgData and thus the 
    span of time used to calculate the average motor current.
    @see mg_au16AvgData                                                          */
#define AVG_COUNT   8U
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

/** @brief UINT16 variable to store the latest current ADC value.             */
STATIC UINT16 mg_u16Value   = 0U;

/** @brief UINT16 variable to store the ADC values of motor current for 
    accumulation.                                                                */
STATIC UINT16 mg_au16Data[SUM_COUNT];

/** @brief UINT16 variable to store the ADC values of motor current for 
    averaging.                                                                */
STATIC UINT16 mg_au16AvgData[AVG_COUNT];

/** @brief UINT16 variable to store the current sum.                          */
STATIC UINT16 mg_u16Sum     = 0U;

/** @brief UINT8 index variable for array mg_au16Data.                        */
STATIC UINT8  mg_u8SumIdx   = 0U;

/** @brief UINT8 index variable for array mg_au16AvgData.                        */
STATIC UINT8  mg_u8AvgIdx   = 0U;

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
@brief  Init function of module current.c.
        
        This function reads the ADC current value to set up the according 
        module variable mg_u16Value.
        Furthermore the ring array will be initialized.
        
@see    mg_u16Value
@see    mg_au16Data
@see    mg_u16Sum
@see    mg_u8Idx
@return void
*/
/* ****************************************************************************/
void current_Init (void)
{
  mg_u16Value = adc_Read(ADC_CURRENT);
  
  for(UINT8 i=0U; i<SUM_COUNT; i++)
  {
    mg_au16Data[i] = 0U;
  }
  
  for(UINT8 i=0U; i<AVG_COUNT; i++)
  {
    mg_au16AvgData[i] = 0U;
  }

  mg_u16Sum   = 0U;
  mg_u8SumIdx = 0U;
  mg_u8AvgIdx = 0U;
}


/* ****************************************************************************/
/**
@brief  Reads the motor current load.
        
        This function reads the momentary motor current draw and checks the 
        value for a warning condition.
        
@see    mg_u16Value
@see    COND_CURR_WARN
@return void
*/
/* ****************************************************************************/
void current_Update (void)
{
  mg_u16Value = adc_Read(ADC_CURRENT);
  if(mg_u16Value > MAX_CURRENT)
  {
    safety_SetCondition(COND_CURR_WARN);
  }
  else
  {
    safety_ClearCondition(COND_CURR_WARN);
  }
}


/* ****************************************************************************/
/**
@brief  Updates the motor current load sum.
        
        This function reads the momentary motor current draw and uses this 
        value to recalculate the average current consumption.
        The new calculated value is stored in mg_u16Sum and is checked for an 
        error condition.
        The frequency of calling this function and the size of the mg_au16Data 
        array define the span of time used for calculating the motor current 
        sum.
        
        An example: SUM_COUNT = 20 and calling current_UpdateSum() every 512ms 
        (see application loop in main()) results in a time span of 10.24s.
        
@see    mg_au16Data
@see    mg_u16Sum
@see    COND_CURR_ERR
@see    MAX_CURRENT
@see    SUM_COUNT
@return void
*/
/* ****************************************************************************/
void current_UpdateSum (void)
{
  /* get momentary current value */
  UINT16 u16Help = adc_Read(ADC_CURRENT);
  
  /* insert in ring array */
  mg_au16Data[mg_u8SumIdx] = u16Help;
  mg_u8SumIdx++;
  if(mg_u8SumIdx >= SUM_COUNT)
  {
    mg_u8SumIdx = 0U;
  }
  
  /* recalculate new sum from array */
  u16Help = 0U;
  for(UINT8 i=0U; i<SUM_COUNT; i++)
  {
    u16Help += mg_au16Data[i];
  }
  mg_u16Sum = u16Help;
  
  /* check for current overload */
  if(mg_u16Sum > (MAX_CURRENT * SUM_COUNT))
  {
    safety_SetCondition(COND_CURR_ERR);
    safety_ClearCondition(COND_CURR_WARN);
  }
  else
  {
    safety_ClearCondition(COND_CURR_ERR);
  }
}


/* ****************************************************************************/
/**
@brief  Updates the average motor current.
        
        This function reads the momentary motor current draw and uses this 
        value to store it in the mg_au16AvgData array for futher calculations.
        The frequency of calling this function and the size of the 
        mg_au16AvgData array define the span of time used for calculating the 
        average motor current.
        
        An example: AVG_COUNT = 8 and calling current_UpdateAvg() every 64ms 
        (see application loop in main()) results in a time span of 0.512s.
        
@see    mg_au16AvgData
@see    AVG_COUNT
@return void
*/
/* ****************************************************************************/
void current_UpdateAvg (void)
{
  /* get momentary current value */
  UINT16 u16Help = adc_Read(ADC_CURRENT);
  
  /* insert in ring array */
  mg_au16AvgData[mg_u8AvgIdx] = u16Help;
  mg_u8AvgIdx++;
  if(mg_u8AvgIdx >= AVG_COUNT)
  {
    mg_u8AvgIdx = 0U;
  }
}


/* ****************************************************************************/
/**
@brief  Returns the average motor current value.

@return average value of mg_au16AvgData[] (UINT16)
@retval 0..1023
*/
/* ****************************************************************************/
UINT16 current_GetAvg (void)
{
  UINT16 u16Help = 0U;
  
  /* calculate average value from array */
  for(UINT8 i=0U; i<AVG_COUNT; i++)
  {
    u16Help += mg_au16AvgData[i];
  }
  u16Help /= AVG_COUNT;
  
  return (u16Help);
}


/* ****************************************************************************/
/**
@brief  Returns the latest value for motor current.

@see    current_Update
@return mg_u16Value (UINT16)
@retval 0..1023
*/
/* ****************************************************************************/
UINT16 current_GetValue (void)
{
  return (mg_u16Value);
}


/* ****************************************************************************/
/**
@brief  Returns the latest value for motor current sum.

@see    current_UpdateSum
@see    SUM_COUNT
@return mg_u16Sum (UINT16)
@retval 0..1023 * SUM_COUNT
*/
/* ****************************************************************************/
UINT16 current_GetSum (void)
{
  return (mg_u16Sum);
}

/* End: Global functions                                                *//*@}*/
/******************************************************************************/

/**** Last line of code                                                    ****/
