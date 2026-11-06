/*!@addtogroup header Header files*//*@{*/
/**
@file             pedal.h
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
@brief            Header file for module <pedal.c>
*//*@}*/

#ifndef PEDAL_H
#define PEDAL_H

#ifdef EXPORT
  #define EXTERN
#else
  #define EXTERN extern
#endif

/******************************************************************************/
/* Constants (#define)                *//*!@addtogroup define Constants *//*@{*/

/** @brief This value defines how many milliseconds the pedal interrupt missing
    before setting pedaling RPM to 0.                                         */
#define PEDAL_TIMEOUT 2000U

/** @brief 2026: shortest accepted time between two pedal sensor pulses in ms,
    for ONE magnet (divided by the number of magnets). Shorter pulses are
    treated as reed switch contact bounce. 100 ms = 600 rpm with one magnet. */
#define PEDAL_MIN_PERIOD_MS 100U

/* End: define Constants                                                *//*@}*/
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
/* Global variables/constan.  *//*!@addtogroup globvar Global variables *//*@{*/
/* End: Global variables                                                *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Prototype declarations                                                     */

EXTERN void   pedal_Init (void);
EXTERN UINT32 pedal_GetTimeout (void);
EXTERN UINT32 pedal_GetRpm (void);

/* End: Prototype declarations                                                */
/******************************************************************************/

#undef EXTERN
#endif /* PEDAL_H */

/**** Last line of code                                                    ****/
