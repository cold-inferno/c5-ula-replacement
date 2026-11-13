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

/** @brief 2026: resting battery voltage (ADC value * 16, filtered), updated
    only while the motor has been off for VOLT_REST_SETTLE periods.           */
STATIC UINT16 mg_u16Rest16              = 0U;

/** @brief 2026: number of 512 ms periods the motor has been off              */
STATIC UINT8  mg_u8MotorOffCnt          = 0U;

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
/* 2026: reads the VREF potentiometer. The original formula underflowed when
   the pot was set below ADC 695, which caused a permanent voltage error. */
STATIC UINT16 mg_ReadRef (void)
{
  UINT16 u16Adc = adc_Read(ADC_VREF);
  return ((u16Adc > 695U) ? (UINT16)((u16Adc - 695U) * 26U / 10U) : 0U);
}

void voltage_Init (void)
{
  UINT16 u16Help = adc_Read(ADC_VOLTAGE);
  
  mg_u16Ref   = mg_ReadRef();
  mg_u16Value = u16Help;
  
  for(UINT8 i=0U; i<AVG_COUNT; i++)
  {
    mg_au16Data[i] = u16Help;
  }
  mg_u16Avg = u16Help;
  mg_u8Idx = 0U;
  /* the motor is off at power-on: the first reading is a resting voltage */
  mg_u16Rest16     = (UINT16)(u16Help * 16U);
  mg_u8MotorOffCnt = VOLT_REST_SETTLE;
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
  mg_u16Ref   = mg_ReadRef();
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

#if defined(BATTERY_LIFEPO4)
  /* resting voltage: only while the motor has been off long enough.
     V1203: "motor off" is reported by the main loop (voltage_MotorActive(),
     button pressed and relay enabled). Up to V1202 this looked at the relay
     enable signal only, which is permanently on without pedal assist, so
     the resting voltage froze at its power-on value. */
  if(mg_u8MotorOffCnt < VOLT_REST_SETTLE)
  {
    mg_u8MotorOffCnt++;
  }
  else
  {
    /* first-order low-pass filter (ADC*16), time constant 8 * 512 ms */
    UINT16 u16Sample = mg_au16Data[(mg_u8Idx + AVG_COUNT - 1U) % AVG_COUNT];
    mg_u16Rest16 = (UINT16)(mg_u16Rest16 - (mg_u16Rest16 / 8U) + (u16Sample * 2U));
  }

  {
    UINT16 u16Rest = voltage_GetRest();

    /* low battery warning with hysteresis */
    if(u16Rest < VOLT_WARN_ON)
    {
      safety_SetCondition(COND_VOLT_WARN);
    }
    else if(u16Rest >= VOLT_WARN_OFF)
    {
      safety_ClearCondition(COND_VOLT_WARN);
    }

    /* motor cut-off: battery empty, or voltage collapsing under load */
    if((u16Rest < VOLT_ERR_ON) || (mg_u16Avg < VOLT_ERR_LOADED))
    {
      safety_SetCondition(COND_VOLT_ERR);
    }
    else if(u16Rest >= VOLT_ERR_OFF)
    {
      safety_ClearCondition(COND_VOLT_ERR);
    }
  }
#else
  if(mg_u16Avg < mg_u16Ref)
  {
    safety_SetCondition(COND_VOLT_ERR);
  }
  else
  {
    safety_ClearCondition(COND_VOLT_ERR);
  }
#endif
}

/** @brief V1203: tells this module that the motor can draw current right now
    (motor button pressed while the relay is enabled). Called from the main
    loop every 64 ms; restarts the settling time of the resting voltage.     */
void voltage_MotorActive (void)
{
  mg_u8MotorOffCnt = 0U;
}

/** @brief 2026: returns the resting battery voltage as ADC value.            */
UINT16 voltage_GetRest (void)
{
  return ((UINT16)((mg_u16Rest16 + 8U) / 16U));
}

/** @brief 2026: returns the value shown on the LED voltage bar.              */
UINT16 voltage_GetDisplay (void)
{
#if defined(BATTERY_LIFEPO4)
  return (voltage_GetRest());
#else
  return (mg_u16Avg);
#endif
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
