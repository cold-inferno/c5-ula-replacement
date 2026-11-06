/*!@addtogroup header Header files*//*@{*/
/**
@file             leds.h
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
@brief            Header file for module <leds.c>
*//*@}*/

#ifndef LEDS_H
#define LEDS_H

#ifdef EXPORT
  #define EXTERN
#else
  #define EXTERN extern
#endif

/******************************************************************************/
/* Constants (#define)                *//*!@addtogroup define Constants *//*@{*/
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

EXTERN void leds_Init         (void);
EXTERN void leds_ShowVoltage  (void);
EXTERN void leds_ShowCurrent  (void);
EXTERN void leds_SetLeftBar   (E_BOOL bButtonPressed, UINT8 u8SpeedLevel);

/* End: Prototype declarations                                                */
/******************************************************************************/

#undef EXTERN
#endif /* LEDS_H */

/**** Last line of code                                                    ****/
