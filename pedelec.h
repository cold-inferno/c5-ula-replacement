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

/** @brief V1203 stall protection: motor current (ADC value) that counts as
    "high". 299 = about 80 A (the scale is 523 = 140 A). Cruising needs
    20-30 A, a blocked motor draws well over 100 A.                           */
#define STALL_CURRENT_ADC    299U
/** @brief Stall protection is evaluated every 64 ms (one "tick").           */
#define STALL_TICK_MS         64U
/** @brief Stall protection: the motor is cut when the current has been high
    for this many ticks in total without a single pulse from the wheel
    sensor (47 x 64 ms = 3 s). One wheel turn (1.28 m) in 3 s is 1.5 km/h.    */
#define STALL_TICKS           47U
/** @brief Stall protection: every momentary overcurrent cut (140 A) counts
    as this many ticks (about 1 s), so the third cut in a row without wheel
    movement latches the protection instead of retrying forever.             */
#define STALL_TRIP_TICKS      16U
/** @brief Speed per LED of the speed bar, in 10th of km/h (50 = 5 km/h)     */
#define SPEEDBAR_STEP         50U

/** @brief Above this speed the top LED of the speed bar flashes (10th km/h) */
#define SPEEDBAR_FLASH       300U

/** @brief Speed bar level meaning "all 5 LEDs on, top LED flashing"         */
#define SPEEDBAR_LEVEL_FLASH   6U

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

/** @brief Inputs for the stall protection                                    */
typedef struct
{
  UINT16  u16Distance;      // trip distance (lower 16 bits); changes with
                            // every wheel pulse
  UINT16  u16Current;       // motor current, ADC value
  E_BOOL  bButton;          // TRUE: motor button pressed
  E_BOOL  bOvercurrent;     // TRUE: a momentary overcurrent cut happened
                            // since the previous call
} S_STALL_INPUT;
/** @brief Stall protection state kept between calls                         */
typedef struct
{
  UINT16  u16Distance;      // distance at the previous call
  UINT8   u8Ticks;          // high-current ticks without a wheel pulse
  E_BOOL  bSensor;          // TRUE: the wheel sensor has given a pulse
  E_BOOL  bOverPrev;        // overcurrent flag at the previous call
  E_BOOL  bLatched;         // TRUE: motor cut until the button is released
} S_STALL_STATE;
/* End: Basic types (typedef)                                           *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Prototype declarations                                                     */
EXTERN void   pedelec_Init   (S_PEDELEC_STATE* psState, UINT32 u32Now);
EXTERN E_BOOL pedelec_Decide (S_PEDELEC_STATE* psState,
                              const S_PEDELEC_INPUT* psIn);
EXTERN UINT8  pedelec_SpeedBarLevel (UINT32 u32Speed);
EXTERN void   pedelec_StallInit (S_STALL_STATE* psState);
EXTERN E_BOOL pedelec_Stall  (S_STALL_STATE* psState,
                              const S_STALL_INPUT* psIn);
/* End: Prototype declarations                                                */
/******************************************************************************/

#undef EXTERN
#endif /* PEDELEC_H */

/**** Last line of code                                                    ****/
