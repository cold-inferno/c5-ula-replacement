/*!@addtogroup sources Source files*//*@{*/
/**
@file             temp.c
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
                  
@brief            Functions to meassure the motor temperature.
                  
                  This module provides service-level functions to get the 
                  averaged motor temperature and to set a warning condition in 
                  case of overheating.
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
  #include "temp.h"
#undef EXPORT

/******************************************************************************/
/* Constants (#define)                *//*!@addtogroup define Constants *//*@{*/

/** @brief Array size of mg_au16Data                                          */
#define AVG_COUNT       10U

/** @brief This value defines how many temperature warnings will be accepted 
    before setting the temperature error flag                                 */
#define WARNING_COUNT   20U

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

/** @brief Variable to keep the last temperature measure values               */
STATIC UINT16 mg_au16Data[AVG_COUNT];

/** @brief Variable to hold the average temperature value                     */
STATIC UINT16 mg_u16Value             = 0U;

/** @brief Index variable for usae with array mg_au16Data                     */
STATIC UINT8  mg_u8Idx                = 0U;

/** @brief Variable to count temperature exceedance                           */
STATIC UINT8  mg_u8WarningCnt         = 0U;

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
@brief  Init function for the temperature module.

        This init function reads the current temperature and fills the array 
        mg_au16Data with this value.
        The averaged temperature is also preset to the current temperature.
        
@return void
*/
/* ****************************************************************************/
void temp_Init (void)
{
  UINT16 u16Help = adc_Read(ADC_TEMPERATURE);
  
  for(UINT8 i=0U; i<AVG_COUNT; i++)
  {
    mg_au16Data[i] = u16Help;
  }
  mg_u16Value = u16Help;
  mg_u8Idx = 0U;
}


/* ****************************************************************************/
/**
@brief  Function to read the current motor temperature.

        The current motor temperature will be read and used to recalculate the
        average temperature value.
        
@return void
*/
/* ****************************************************************************/
void temp_Update (void)
{
  UINT16 u16Help = adc_Read(ADC_TEMPERATURE);
  
  mg_au16Data[mg_u8Idx] = u16Help;
  mg_u8Idx++;
  if(mg_u8Idx >= AVG_COUNT)
  {
    mg_u8Idx = 0U;
  }
  u16Help = 0U;
  for(UINT8 i=0U; i<AVG_COUNT; i++)
  {
    u16Help += mg_au16Data[i];
  }
  u16Help /= AVG_COUNT;
  mg_u16Value = u16Help;
}


/* ****************************************************************************/
/**
@brief  Function to check the motor temperature for overheating.

        If the averaged motor temperature exeeds a specified value a counter 
        will be incremented and a warning condition flag will be set.
        
        If this counter in turn exeeds a specified limit, a temperature error
        flag will be set.
        
        If the temperature falls, the warning or error flags will be cleared.
        
@see safety_SetCondition()
@see safety_ClearCondition()
@return void
*/
/* ****************************************************************************/
void temp_Check (void)
{
  if(temp_GetCelsius(mg_u16Value) < VALUE_TEMP_WARN)
  {
    if (mg_u8WarningCnt > 0U)
    {
      mg_u8WarningCnt--;
    }
  }
  else if(temp_GetCelsius(mg_u16Value) < VALUE_TEMP_ERROR)
  {
    mg_u8WarningCnt++;
  }
  else
  {
    mg_u8WarningCnt = WARNING_COUNT;
  }
  
  if(mg_u8WarningCnt >= WARNING_COUNT)
  {
    mg_u8WarningCnt = WARNING_COUNT;
    safety_ClearCondition(COND_TEMP_WARN);
    safety_SetCondition(COND_TEMP_ERR);
  }
  else if (mg_u8WarningCnt > 0U)
  {
    safety_SetCondition(COND_TEMP_WARN);
    safety_ClearCondition(COND_TEMP_ERR);
  }
  else
  {
    safety_ClearCondition(COND_TEMP_WARN);
    safety_ClearCondition(COND_TEMP_ERR);
  }
}


/* ****************************************************************************/
/**
@brief  Get method for mg_u16Value.

        This function returns the averaged motor temperature.
        
@return Value of mg_u16Value
@retval UINT16
*/
/* ****************************************************************************/
UINT16 temp_GetAvg (void)
{
  return (mg_u16Value);
}



/* ****************************************************************************/
/**
@brief  A helper method to convert ADC value to temperature.

        This function returns given ADC value as temperature in °C.
        
@param  adc_val ADC value
@return Temperature value in °C
@retval UINT16
*/
/* ****************************************************************************/
UINT16 temp_GetCelsius (UINT16 adc_val)
{
  INT16   u16Ret = 85U;
  UINT16  u16NTC;
  
  if (0U != adc_val)
  {
    u16NTC = (UINT16) (((1024UL * 1100UL) / ((UINT32)adc_val)) - 1100UL);
    
    if (u16NTC >= 12500U)
    {
      u16Ret = 0U;
    }
    else if (u16NTC >= 9000U)
    {
      u16Ret = 5U;
    }
    else if (u16NTC >= 7500U)
    {
      u16Ret = 10U;
    }
    else if (u16NTC >= 6000U)
    {
      u16Ret = 15U;
    }
    else if (u16NTC >= 4700U)
    {
      u16Ret = 20U;
    }
    else if (u16NTC >= 3800U)
    {
      u16Ret = 25U;
    }
    else if (u16NTC >= 3300U)
    {
      u16Ret = 30U;
    }
    else if (u16NTC >= 2500U)
    {
      u16Ret = 35U;
    }
    else if (u16NTC >= 2000U)
    {
      u16Ret = 40U;
    }
    else if (u16NTC >= 1700U)
    {
      u16Ret = 45U;
    }
    else if (u16NTC >= 1200U)
    {
      u16Ret = 50U;
    }
    else if (u16NTC >= 1000U)
    {
      u16Ret = 55U;
    }
    else if (u16NTC >= 800U)
    {
      u16Ret = 60U;
    }
    else if (u16NTC >= 900U)
    {
      u16Ret = 65U;
    }
    else if (u16NTC >= 800U)
    {
      u16Ret = 70U;
    }
    else if (u16NTC >= 600U)
    {
      u16Ret = 75U;
    }
    else if (u16NTC >= 500U)
    {
      u16Ret = 80U;
    }
    else
    {
      u16Ret = 85U;
    }
  }
  
  return (u16Ret);
}

/* End: Global functions                                                *//*@}*/
/******************************************************************************/

/**** Last line of code                                                    ****/
