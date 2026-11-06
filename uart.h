/*!@addtogroup header Header files*//*@{*/
/**
@file             uart.h
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
@brief            Header file for module <uart.c>
*//*@}*/

#ifndef UART_H
#define UART_H

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

EXTERN void uart_Init     (void);
EXTERN void uart_PutChar  (CHAR c);
#if 0
EXTERN void uart_PutStr   (CHAR* string);
#endif
EXTERN CHAR uart_GetChar  (void);

/* End: Prototype declarations                                                */
/******************************************************************************/

#undef EXTERN
#endif /* UART_H */

/**** Last line of code                                                    ****/
