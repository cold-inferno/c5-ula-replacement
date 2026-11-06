/*!@addtogroup header Header files*//*@{*/
/**
@file             temp.h
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
@brief            Header file for module <temp.c>
*//*@}*/

#ifndef TEMP_H
#define TEMP_H

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

EXTERN void   temp_Init       (void);
EXTERN void   temp_Update     (void);
EXTERN void   temp_Check      (void);
EXTERN UINT16 temp_GetAvg     (void);
EXTERN UINT16 temp_GetCelsius (UINT16 adc_val);

/* End: Prototype declarations                                                */
/******************************************************************************/

#undef EXTERN
#endif /* TIMER_H */

/**** Last line of code                                                    ****/
