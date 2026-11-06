/*!@addtogroup sources Source files*//*@{*/
/**
@file             print.c
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
                  
@brief            This module provides an interface to stdout.
                  
                  This module is used to provide an exchangeable interface for 
                  stdout-functions like printf.
                  
                  Printing characters is redirected to uart_PutChar().
*//*@}*/

/******************************************************************************/
/* Includes (#include)                                                        */
#include <avr/io.h>

/* Own header files */
#include "xtypes.h"
#include "global.h"

#include "uart.h"

#define EXPORT
  #include "print.h"
#undef EXPORT

/******************************************************************************/
/* Constants (#define)                *//*!@addtogroup define Constants *//*@{*/
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
/* Local constants (const)      *//*!@addtogroup loccon Local constants *//*@{*/
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
@brief  Init fnction of module print.

        This function redirects outputs to stdout to the function print_Char().
        
@see print_Char()
@return void
*/
/* ****************************************************************************/
void print_Init (void)
{
  /* connect with printf (STDOUT) */
  fdevopen (print_Char, NULL);
}


/* ****************************************************************************/
/**
@brief  Common interface function for stdout.

        This function directly redirects to uart_PutChar().
        
        Since the parameter stream isn't used it can be set to NULL.
        
@param c CHAR charcater to be writen on stdout
@param stream FILE* file pointer for compability with stdout access.
@return INT16
@retval always 0
*/
/* ****************************************************************************/
INT16  print_Char (CHAR c, FILE* stream)
{
  uart_PutChar(c);
  
  return (0U);
}


/* End: Global functions                                                *//*@}*/
/******************************************************************************/

/**** Last line of code                                                    ****/


