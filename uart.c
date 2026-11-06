/*!@addtogroup sources Source files*//*@{*/
/**
@file             uart.c
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
                  
@brief            Serial communication functions.
                  
                  This module provides low-level functions to communicate via
                  the serial interface.
                  
                  Note that this module is used by application software modules
                  and by the bootloader.
*//*@}*/

/******************************************************************************/
/* Includes (#include)                                                        */
#include <avr/io.h>

/* Own header files */
#include "xtypes.h"
#include "global.h"

#define EXPORT
  #include "uart.h"
#undef EXPORT

/******************************************************************************/
/* Constants (#define)                *//*!@addtogroup define Constants *//*@{*/

/** @brief Definition for serial transmission speed                           */
#define BAUD_RATE 9600

#if defined (UCSR0A) /* ATmega48/88/168/328 family (incl. P/PA variants) */
  #define USR   UCSR0A
  #define UCR   UCSR0B
  #define UBRR  UBRR0L
  #define UDRE  UDRE0
  #define UDR   UDR0
  #define RXC   RXC0
#endif

/* End: Constants (#define)                                             *//*@}*/
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
/* Local constants (const)     *//*!@addtogroup loccon  Local constants *//*@{*/
/* End: Local constants (const)                                         *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Local variables              *//*!@addtogroup locvar Local variables *//*@{*/
/* End: Local variables                                                 *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Global variables           *//*!@addtogroup globvar Global variables *//*@{*/
/* End: Global variables                                                *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Prototype declarations                                                     */
/* End: Prototype declarations                                                */
/******************************************************************************/

/******************************************************************************/
/* Local functions             *//*!@addtogroup locfunc Local functions *//*@{*/
/* End: Local functions                                                 *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Global functions          *//*!@addtogroup globfunc Global functions *//*@{*/


/* ****************************************************************************/
/**
@brief  Sending a single character via serial port.
        
@return void
*/
/* ****************************************************************************/
void uart_PutChar (CHAR c)
{
#ifndef BOOTLOADER
  if('\n' == c)
  {
    uart_PutChar('\r');
  }
#endif
  /* wait until ready to send */
  loop_until_bit_is_set(USR, UDRE);
  
  /* send char */
  UDR = c;
}


#if 0
/* ****************************************************************************/
/**
@brief  Not used.
        
@return void
*/
/* ****************************************************************************/
void uart_PutStr(CHAR* string)
{
  while (*string != 0U)
  {
    uart_PutChar(*string);
    string++;
  }
}
#endif


/* ****************************************************************************/
/**
@brief  Reading a single character from serial port.
        
@return void
*/
/* ****************************************************************************/
CHAR uart_GetChar(void)
{
  CHAR c;
  
  loop_until_bit_is_set(USR, RXC);
  c = UDR;
  return (c);
}


/* ****************************************************************************/
/**
@brief  Init method of the uart module.
        
        This init function configures the serial port.
        
@return void
*/
/* ****************************************************************************/
void uart_Init (void)
{
  RXD_DDR   &= ~(1U << RXD_BIT);  /* set RXD as input pin */
  TXD_DDR   |=  (1U << TXD_BIT);  /* set TXD as output pin */
  RXD_PORT  |=  (1U << RXD_BIT);  /* activate RXD pull-up resistor */

  /* Enable TXEN and RXEN in register UCR */
  UCR   = (1 << TXEN0)|(1 << RXEN0);
  
  /* set baudrate prescaler */
  UBRR  = (F_CPU / (BAUD_RATE * 16L) - 1U);
}

/* End: Global functions                                                *//*@}*/
/******************************************************************************/

/**** Last line of code                                                    ****/


