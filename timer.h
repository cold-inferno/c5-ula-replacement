/*!@addtogroup header Header files*//*@{*/
/**
@file             timer.h
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
@brief            Header file for module <timer.c>
*//*@}*/

#ifndef TIMER_H
#define TIMER_H

#ifdef EXPORT
  #define EXTERN
#else
  #define EXTERN extern
#endif

/******************************************************************************/
/* Constants (#define)                *//*!@addtogroup define Constants *//*@{*/

/** @brief No timer flags                                                     */
#define NO_TICK   0U

/** @brief Flag for 128ms events                                              */
#define TICK128   1U

/** @brief Flag for 256ms events                                              */
#define TICK256   2U

/** @brief Flag for 512ms events                                              */
#define TICK512   4U

/** @brief Flag for 1024ms events                                             */
#define TICK1024  8U

/* End: define Constants                                                *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Macro definitions (#define)           *//*!@addtogroup macros Macros *//*@{*/

/** @brief This macro checks whether the 128ms time flag is set or not        */
#define TIMER_IS_TICK128()   ((timer_GetTicks() & TICK128)  != NO_TICK)

/** @brief This macro checks whether the 256ms time flag is set or not        */
#define TIMER_IS_TICK256()   ((timer_GetTicks() & TICK256)  != NO_TICK)

/** @brief This macro checks whether the 512ms time flag is set or not        */
#define TIMER_IS_TICK512()   ((timer_GetTicks() & TICK512)  != NO_TICK)

/** @brief This macro checks whether the 1024ms time flag is set or not       */
#define TIMER_IS_TICK1024()  ((timer_GetTicks() & TICK1024) != NO_TICK)

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

EXTERN void   timer_Init          (void);
EXTERN UINT32 timer_GetSystemTime (void);
EXTERN void   timer_LockTicks     (void);
EXTERN UINT8  timer_GetTicks      (void);

/* End: Prototype declarations                                                */
/******************************************************************************/

#undef EXTERN
#endif /* TIMER_H */

/**** Last line of code                                                    ****/
