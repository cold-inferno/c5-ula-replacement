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

/** @brief V1203: flash timing of fault codes, as bit of the system time in
    ms. The LED (and buzzer) is on while the bit is 0.
    Error: 128 ms on / 128 ms off (fast). Warning: 512 ms on / 512 ms off.   */
#define LED_FLASH_ERR     128U
#define LED_FLASH_WARN    512U
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
