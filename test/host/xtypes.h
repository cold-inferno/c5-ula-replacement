/* Host (PC) replacement for ../../xtypes.h: same names, but with exact-width
   types so UINT16/UINT32 behave like on the 8-bit AVR (16/32 bit). */
#ifndef _XTYPES_H
#define _XTYPES_H

#include <stdint.h>

typedef char      CHAR;
typedef int8_t    INT8;
typedef uint8_t   UINT8;
typedef int16_t   INT16;
typedef uint16_t  UINT16;
typedef int32_t   INT32;
typedef uint32_t  UINT32;

typedef enum { FALSE = 0U, TRUE = 1U } E_BOOL;
typedef enum { OFF = 0U, ON = 1U } E_SWITCH;

#endif
