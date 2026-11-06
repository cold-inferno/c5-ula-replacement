/*!@addtogroup header Header files*//*@{*/
/**
@file             safety.h
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
@brief            Header file for module <safety.c>
*//*@}*/

#ifndef SAFETY_H
#define SAFETY_H

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

EXTERN void               safety_Init           (void);
EXTERN void               safety_SetCondition   (E_SAFETY_CONDITION eCond);
EXTERN void               safety_ClearCondition (E_SAFETY_CONDITION eCond);
EXTERN UINT32             safety_GetCRC         (void);
EXTERN E_SAFETY_CONDITION safety_GetCondition   (void);
EXTERN void               safety_MemoryTest     (void);
EXTERN void               safety_SafeMode       (void);

/* End: Prototype declarations                                                */
/******************************************************************************/

#undef EXTERN
#endif /* SAFETY_H */

/**** Last line of code                                                    ****/
