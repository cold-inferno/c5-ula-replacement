/*!@addtogroup header Header files*//*@{*/
/**
@file             print.h
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
@brief            Header file for module <print.c>

                  2026: printf()/stdio replaced by small dedicated print
                  functions to free flash memory (the ATmega88 application
                  area is only 7 KB).
*//*@}*/

#ifndef PRINT_H
#define PRINT_H

#include <avr/pgmspace.h>

#ifdef EXPORT
  #define EXTERN
#else
  #define EXTERN extern
#endif

/******************************************************************************/
/* Macro definitions (#define)           *//*!@addtogroup macros Macros *//*@{*/
/** @brief Print a string literal stored in flash memory                      */
#define PRINT_P(s)  print_StrP(PSTR(s))
/* End: Macro definitions (#define)                                     *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Prototype declarations                                                     */
EXTERN void   print_Init  (void);
EXTERN void   print_Char  (CHAR c);
EXTERN void   print_StrP  (const char* pStr);
EXTERN void   print_U32   (UINT32 u32Value);
EXTERN void   print_Hex32 (UINT32 u32Value);
/* End: Prototype declarations                                                */
/******************************************************************************/

#undef EXTERN
#endif /* PRINT_H */

/**** Last line of code                                                    ****/
