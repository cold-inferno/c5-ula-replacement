/*!@addtogroup header Header files*//*@{*/
/**
@file             pedelec.h
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
@brief            Header file for module <pedelec.c> (added 2026).

                  Pure decision logic for the motor relay, free of hardware
                  access so it can be unit-tested on a PC (see test/).
*//*@}*/

#ifndef PEDELEC_H
#define PEDELEC_H

#ifdef EXPORT
  #define EXTERN
#else
  #define EXTERN extern
#endif

/******************************************************************************/
/* Constants (#define)                *//*!@addtogroup define Constants *//*@{*/

/** @brief Hysteresis below the maximum speed limit, in 10th of km/h.
    Once the motor was cut at the upper limit it is only enabled again when
    the speed has dropped this far below the limit (10 = 1.0 km/h).          */
#define PEDELEC_HYST_MAX      10U

/** @brief Hysteresis below the minimum speed limit, in 10th of km/h.
    A running motor stays on until the speed drops this far below the
    minimum (5 = 0.5 km/h).                                                   */
#define PEDELEC_HYST_MIN       5U

/** @brief Value of the maximum speed setting that disables the upper speed
    cut-off (the motor then assists at any speed while pedalling).
    WARNING: a pedelec without the 25 km/h cut-off is not road-legal as a
    bicycle in the EU.                                                        */
#define PEDELEC_NO_MAX         0U

/** @brief Minimum time in ms between two relay switching actions that are
    caused by pedelec conditions. Prevents the motor relay from chattering.
    Safety cut-offs (errors, overcurrent) always act immediately.            */
#define RELAY_MIN_DWELL_MS   500U

/* End: define Constants                                                *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Basic types (typedef)                 *//*!@addtogroup typedef Types *//*@{*/

/** @brief Inputs for one relay decision                                      */
typedef struct
{
  UINT32  u32Now;           // system time in ms
  E_BOOL  bSafetyCut;       // TRUE: error condition or momentary overcurrent
  E_BOOL  bPedelecMode;     // TRUE: pedelec mode configured
  E_BOOL  bPedalling;       // TRUE: pedal pulse seen within PEDAL_TIMEOUT
  UINT32  u32Speed;         // 10th of km/h
  UINT16  u16SpeedMin;      // 10th of km/h
  UINT16  u16SpeedMax;      // 10th of km/h, PEDELEC_NO_MAX = no limit
} S_PEDELEC_INPUT;

/** @brief Relay decision state kept between calls                            */
typedef struct
{
  E_BOOL  bMotorOn;         // current relay state
  UINT32  u32LastSwitch;    // system time of the last relay change in ms
} S_PEDELEC_STATE;

/* End: Basic types (typedef)                                           *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Prototype declarations                                                     */
EXTERN void   pedelec_Init   (S_PEDELEC_STATE* psState, UINT32 u32Now);
EXTERN E_BOOL pedelec_Decide (S_PEDELEC_STATE* psState,
                              const S_PEDELEC_INPUT* psIn);
/* End: Prototype declarations                                                */
/******************************************************************************/

#undef EXTERN
#endif /* PEDELEC_H */

/**** Last line of code                                                    ****/
