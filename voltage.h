/*!@addtogroup header Header files*//*@{*/
/**
@file             voltage.h
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
@brief            Header file for module <voltage.c>
*//*@}*/

#ifndef VOLTAGE_H
#define VOLTAGE_H

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
EXTERN void   voltage_Init      (void);
EXTERN void   voltage_Update    (void);
EXTERN void   voltage_UpdateAvg (void);
EXTERN UINT16 voltage_GetValue  (void);
EXTERN UINT16 voltage_GetAvg    (void);
EXTERN UINT16 voltage_GetRef    (void);
EXTERN UINT16 voltage_GetRest   (void);
EXTERN UINT16 voltage_GetDisplay(void);
/* End: Prototype declarations                                                */
/******************************************************************************/

#undef EXTERN
#endif /* VOLTAGE_H */

/**** Last line of code                                                    ****/
