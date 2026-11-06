/*!@addtogroup header Header files*//*@{*/
/**
@file             current.h
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
@brief            Header file for module <current.c>
*//*@}*/

#ifndef CURRENT_H
#define CURRENT_H

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

EXTERN void   current_Init      (void);
EXTERN void   current_Update    (void);
EXTERN UINT16 current_GetValue  (void);
EXTERN UINT16 current_GetSum    (void);
EXTERN UINT16 current_GetAvg    (void);
EXTERN void   current_UpdateSum (void);
EXTERN void   current_UpdateAvg (void);

/* End: Prototype declarations                                                */
/******************************************************************************/

#undef EXTERN
#endif /* CURRENT_H */

/**** Last line of code                                                    ****/
