/*!@addtogroup header Header files*//*@{*/
/**
@file             adc.h
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
@brief            Header file for module <adc.c>
*//*@}*/

#ifndef ADC_H
#define ADC_H

#ifdef EXPORT
  #define EXTERN
#else
  #define EXTERN extern
#endif

/******************************************************************************/
/* Constants (#define)                *//*!@addtogroup define Constants *//*@{*/

/** @brief ADC channel #0                                                     */
#define ADC0 0

/** @brief ADC channel #1                                                     */
#define ADC1 1

/** @brief ADC channel #2                                                     */
#define ADC2 2

/** @brief ADC channel #3                                                     */
#define ADC3 3

/** @brief ADC channel #4                                                     */
#define ADC4 4

/** @brief ADC channel #5                                                     */
#define ADC5 5

/** @brief ADC channel #6                                                     */
#define ADC6 6

/** @brief ADC channel #7                                                     */
#define ADC7 7

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

EXTERN void   adc_Init (void);
EXTERN UINT16 adc_Read (UINT8 u8Channel);

/* End: Prototype declarations                                                */
/******************************************************************************/

#undef EXTERN
#endif /* ADC_H */

/**** Last line of code                                                    ****/
