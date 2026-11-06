/*!@addtogroup header Header files*//*@{*/
/**
@file             speed.h
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
@brief            Header file for module <speed.c>
*//*@}*/

#ifndef SPEED_H
#define SPEED_H

#ifdef EXPORT
  #define EXTERN
#else
  #define EXTERN extern
#endif

/******************************************************************************/
/* Constants (#define)                *//*!@addtogroup define Constants *//*@{*/

/** @brief This value defines how many milliseconds the tacho interrupt missing
    before setting current wheel RPM to 0.                                    */
#define SPEED_TIMEOUT 2000U

/* End: define Constants                                                *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Macro definitions (#define)           *//*!@addtogroup macros Macros *//*@{*/

/** @brief This macro returns the current driving speed based on the wheel RPM 
    and the setting for wheel circumference in 10th of km/h                   */
/* 2026: speed now uses the wheel size configured in EEPROM (was the fixed
   WHEEL_SIZE default, so a different wheel gave wrong pedelec limits). */
#define SPEED_GET() speed_GetSpeed() // 10th of km/h

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

EXTERN void   speed_Init          (void);
EXTERN UINT32 speed_GetRpm        (void);
EXTERN UINT32 speed_GetDistance   (void);
EXTERN UINT32 speed_GetSpeed      (void);
EXTERN void   speed_CheckTimeout  (UINT32 u32SystemTime);

/* End: Prototype declarations                                                */
/******************************************************************************/

#undef EXTERN
#endif /* SPEED_H */

/**** Last line of code                                                    ****/
