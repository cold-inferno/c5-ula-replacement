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

/** @brief Resets the stall protection.                                       */
void pedelec_StallInit (S_STALL_STATE* psState)
{
  psState->u16Distance = 0U;
  psState->u8Ticks     = 0U;
  psState->bSensor     = FALSE;
  psState->bOverPrev   = FALSE;
  psState->bLatched    = FALSE;
}

/**
@brief  V1203: stall protection. Call every 64 ms (STALL_TICK_MS).

        A blocked or nearly blocked motor draws a very high current that
        nothing limited before (the temperature sensor reacts slowly, and the
        140 A cut retried every half second forever).
        This function counts the ticks with a high motor current
        (STALL_CURRENT_ADC) during which the wheel sensor gave no pulse, plus
        STALL_TRIP_TICKS for every momentary overcurrent cut. A wheel pulse
        starts the count again. At STALL_TICKS the motor is cut and stays
        cut until the motor button is released.

        The high-current time is only counted once the wheel sensor has given
        at least one pulse since power-on: a C5 without a wheel sensor must
        not be cut at every start. Overcurrent cuts are counted either way.
@param  psState  state, updated by this function
@param  psIn     current inputs
@return TRUE while the motor must stay off (until the button is released)
*/
E_BOOL pedelec_Stall (S_STALL_STATE* psState, const S_STALL_INPUT* psIn)
{
  UINT8  u8Ticks = psState->u8Ticks;
  E_BOOL bMoved  = (psIn->u16Distance != psState->u16Distance) ? TRUE : FALSE;

  if (FALSE != bMoved)
  {
    psState->bSensor = TRUE;       /* the wheel sensor exists and works */
  }

  if (FALSE == psIn->bButton)
  {
    /* button released: the rider reacted, start again */
    u8Ticks = 0U;
    psState->bLatched = FALSE;
  }
  else if (FALSE != bMoved)
  {
    /* the wheel is turning: no stall (an active cut stays until release) */
    u8Ticks = 0U;
  }
  else
  {
    if ((FALSE != psIn->bOvercurrent) && (FALSE == psState->bOverPrev))
    {
      u8Ticks += STALL_TRIP_TICKS;
    }
    else if ((psIn->u16Current >= STALL_CURRENT_ADC) &&
             (FALSE != psState->bSensor))
    {
      u8Ticks++;
    }

    if (u8Ticks >= STALL_TICKS)
    {
      u8Ticks = STALL_TICKS;
      psState->bLatched = TRUE;
    }
  }

  psState->u8Ticks     = u8Ticks;
  psState->u16Distance = psIn->u16Distance;
  psState->bOverPrev   = psIn->bOvercurrent;

  return (psState->bLatched);
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
