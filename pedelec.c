/*!@addtogroup sources Source files*//*@{*/
/**
@file             pedelec.c
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
@brief            Motor relay decision logic (added 2026).

                  Replaces the decision part of the original mg_RelayControl().
                  Differences to the original behaviour:
                  - hysteresis at the minimum and maximum speed limit,
                  - a minimum time between pedelec-caused relay changes,
                  so the 100+ A motor relay no longer chatters when riding
                  near a speed limit or with irregular pedal pulses.
                  Safety cut-offs remain immediate.
                  A maximum speed setting of 0 (PEDELEC_NO_MAX) disables the
                  upper speed cut-off: the motor then assists at any speed
                  while pedalling.

                  The handlebar motor button is not evaluated here: it powers
                  the relay coil directly in the control box, so the motor only
                  runs when the button is pressed AND this logic allows it.
*//*@}*/

/******************************************************************************/
/* Includes (#include)                                                        */
#include "xtypes.h"

#define EXPORT
  #include "pedelec.h"
#undef EXPORT

/******************************************************************************/
/* Global functions          *//*!@addtogroup globfunc Global functions *//*@{*/

/** @brief Resets the decision state (motor off).                             */
void pedelec_Init (S_PEDELEC_STATE* psState, UINT32 u32Now)
{
  psState->bMotorOn      = FALSE;
  psState->u32LastSwitch = u32Now;
}

/**
@brief  Decides whether the motor relay may be enabled.
@param  psState  decision state, updated by this function
@param  psIn     current inputs
@return TRUE if the relay shall be enabled
*/
E_BOOL pedelec_Decide (S_PEDELEC_STATE* psState, const S_PEDELEC_INPUT* psIn)
{
  E_BOOL bWanted;

  /* safety cut-off: immediately, regardless of dwell time */
  if (FALSE != psIn->bSafetyCut)
  {
    if (FALSE != psState->bMotorOn)
    {
      psState->bMotorOn      = FALSE;
      psState->u32LastSwitch = psIn->u32Now;
    }
    return (FALSE);
  }

  if (FALSE == psIn->bPedelecMode)
  {
    /* not in pedelec mode: motor enabled (button decides) */
    bWanted = TRUE;
  }
  else if (FALSE == psIn->bPedalling)
  {
    bWanted = FALSE;
  }
  else if (FALSE != psState->bMotorOn)
  {
    /* running: stay on until clearly outside the speed window */
    UINT16 u16Low = (psIn->u16SpeedMin > PEDELEC_HYST_MIN)
                  ? (UINT16)(psIn->u16SpeedMin - PEDELEC_HYST_MIN) : 0U;
    bWanted = ((psIn->u32Speed >= u16Low) &&
               ((PEDELEC_NO_MAX == psIn->u16SpeedMax) ||
                (psIn->u32Speed <= psIn->u16SpeedMax))) ? TRUE : FALSE;
  }
  else
  {
    /* stopped: start only when clearly inside the speed window */
    UINT16 u16High = (psIn->u16SpeedMax > PEDELEC_HYST_MAX)
                   ? (UINT16)(psIn->u16SpeedMax - PEDELEC_HYST_MAX) : 0U;
    bWanted = ((psIn->u32Speed >= psIn->u16SpeedMin) &&
               ((PEDELEC_NO_MAX == psIn->u16SpeedMax) ||
                (psIn->u32Speed <= u16High))) ? TRUE : FALSE;
  }

  if ((bWanted != psState->bMotorOn) &&
      ((psIn->u32Now - psState->u32LastSwitch) >= RELAY_MIN_DWELL_MS))
  {
    psState->bMotorOn      = bWanted;
    psState->u32LastSwitch = psIn->u32Now;
  }

  return (psState->bMotorOn);
}

/**
@brief  Converts the speed into the number of lit LEDs of the speed bar.
@param  u32Speed  speed in 10th of km/h
@return 0..5 = number of LEDs lit (one per 5 km/h),
        SPEEDBAR_LEVEL_FLASH = above 30 km/h (all on, top LED flashing)
*/
UINT8 pedelec_SpeedBarLevel (UINT32 u32Speed)
{
  if (u32Speed > SPEEDBAR_FLASH)
  {
    return (SPEEDBAR_LEVEL_FLASH);
  }
  if (u32Speed >= (5UL * SPEEDBAR_STEP))
  {
    return (5U);
  }
  return ((UINT8)(u32Speed / SPEEDBAR_STEP));
}

/* End: Global functions                                                *//*@}*/
/******************************************************************************/

/**** Last line of code                                                    ****/
