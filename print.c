/*!@addtogroup sources Source files*//*@{*/
/**
@file             print.c
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
@brief            Minimal output functions for the serial interface.

                  2026: rewritten without stdio/printf. The original used
                  fdevopen() + printf(), which pulled in vfprintf and malloc
                  and left only ~230 bytes of free flash.
*//*@}*/

/******************************************************************************/
/* Includes (#include)                                                        */
#include <avr/io.h>
#include <avr/pgmspace.h>

/* Own header files */
#include "xtypes.h"
#include "global.h"
#include "uart.h"

#define EXPORT
  #include "print.h"
#undef EXPORT

/******************************************************************************/
/* Global functions          *//*!@addtogroup globfunc Global functions *//*@{*/

/** @brief Init function of module <print.c>. Nothing to do any more.        */
void print_Init (void)
{
}

/** @brief Prints one character ('\n' is expanded to "\r\n" by the UART).    */
void print_Char (CHAR c)
{
  uart_PutChar(c);
}

/** @brief Prints a zero-terminated string stored in flash (use PRINT_P()).  */
void print_StrP (const char* pStr)
{
  CHAR c;
  while ((c = (CHAR)pgm_read_byte(pStr++)) != 0)
  {
    uart_PutChar(c);
  }
}

/** @brief Prints an unsigned 32 bit value in decimal, without padding.      */
void print_U32 (UINT32 u32Value)
{
  CHAR  acBuf[10];
  UINT8 u8Len = 0U;

  do
  {
    acBuf[u8Len++] = (CHAR)('0' + (u32Value % 10UL));
    u32Value /= 10UL;
  } while (u32Value != 0UL);

  while (u8Len != 0U)
  {
    uart_PutChar(acBuf[--u8Len]);
  }
}

/** @brief Prints an unsigned 32 bit value as 8 upper-case hex digits.       */
void print_Hex32 (UINT32 u32Value)
{
  for (UINT8 i = 0U; i < 8U; i++)
  {
    UINT8 u8Nibble = (UINT8)(u32Value >> 28);
    uart_PutChar((CHAR)(u8Nibble < 10U ? ('0' + u8Nibble) : ('A' - 10 + u8Nibble)));
    u32Value <<= 4;
  }
}

/* End: Global functions                                                *//*@}*/
/******************************************************************************/

/**** Last line of code                                                    ****/
