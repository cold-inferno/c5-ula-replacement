/*!@addtogroup sources Source files*//*@{*/
/**
@file             bootload.c
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
                  
@brief            Bootloader module containing the main() entry point.
                  
                  This module contains the main() function which is the 
                  bootloader firmware entry point.
*//*@}*/

/******************************************************************************/
/* Includes (#include)                                                        */
#include <avr/boot.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

/* Own header files */
#include "../xtypes.h"
#include "../global.h"
#include "../uart.h"

//#include "../memory.h"

#define EXPORT
  #include "bootload.h"
#undef EXPORT

/******************************************************************************/
/* Constants (#define)                *//*!@addtogroup define Constants *//*@{*/

#define BL_VERSION    1042U

#define CMD_FLASH     'f'
#define CMD_READ      'r'
#define CMD_WRITE     'w'
#define CMD_ENTER     'e'
#define CMD_START     's'
#define CMD_BLVERSION 'v'
#define CMD_FWVERSION 'g'
#define CMD_RESET     'x'

#define RET_BOOT      'B'
#define RET_OK        'O'
#define RET_DONE      'D'
#define RET_ERROR     'E'
#define RET_UNKNOWN   '?'
#define RET_START     'S'

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
void (*appl_Start)(void) = (void*)0x0000U;
void (*boot_Start)(void) = (void*)MEMPOS_BOOT;

STATIC E_BOOL mg_WaitForEnter (void);
STATIC void   mg_WritePage    (UINT8 u8Page, UINT8* au8Data);
/* End: Prototype declarations                                                */
/******************************************************************************/

/******************************************************************************/
/* Local functions             *//*!@addtogroup locfunc Local functions *//*@{*/

int main (void)
{
  E_BOOL            bStayInLoop = TRUE;
  UINT8             u8Help;
  UINT16            u16Help;
  UINT8             au8Page[64U];
  UINT8             u8Crc;
  CHAR              c;

  LED1_DDR |= (1U << LED1_BIT);
  LED2_DDR |= (1U << LED2_BIT);
  LED3_DDR |= (1U << LED3_BIT);
  LED4_DDR |= (1U << LED4_BIT);
  LED5_DDR |= (1U << LED5_BIT);
  
  MUX_VOLT_DDR |= (1U << MUX_VOLT_BIT);
  MUX_CURR_DDR |= (1U << MUX_CURR_BIT);
  
  
  MUX_CURR();
  ALL_LEDS_OFF();
  LED3_ON();
  
  uart_Init();
  
#if 0
  uart_PutChar('b');
  uart_PutChar('o');
  uart_PutChar('o');
  uart_PutChar('t');
#endif
  
  
  if(FALSE != mg_WaitForEnter())
  {
    MUX_VOLT();
    uart_PutChar(RET_BOOT);
    
    while (FALSE != bStayInLoop)
    {
      /* block until a command has been received */
      c = uart_GetChar();
      LED3_PORT ^= (1U << LED3_BIT);
      switch (c)
      {
        case CMD_RESET:
        {
          #if 1
          wdt_enable(WDTO_15MS);
          wdt_reset();
          while(1) { asm("nop"); }
          #endif
          boot_Start();
        }
        break;
        
        case CMD_ENTER:
        {
          uart_PutChar(RET_BOOT);
        }
        break;
        
        case CMD_FWVERSION:
        {
          uart_PutChar(HIGH_BYTE(pgm_read_word(MEMPOS_VER)));
          uart_PutChar( LOW_BYTE(pgm_read_word(MEMPOS_VER)));
        }
        break;
        
        case CMD_BLVERSION:
        {
          uart_PutChar(HIGH_BYTE(BL_VERSION));
          uart_PutChar(LOW_BYTE(BL_VERSION));
        }
        break;
        
        case CMD_START:
        {
          bStayInLoop = FALSE;
        }
        break;
        
        case CMD_READ:
        {
          u16Help  = uart_GetChar();
          u16Help += (UINT16)uart_GetChar() << 8U;
          u16Help += (UINT16)&sMemoryEEPROM;
          u8Help = eeprom_read_byte((UINT8*)u16Help);
          uart_PutChar(u8Help);
        }
        break;
        
        case CMD_WRITE:
        {
          u16Help  = uart_GetChar();
          u16Help += (UINT16)uart_GetChar() << 8U;
          u16Help += (UINT16)&sMemoryEEPROM;
          
          u8Help = uart_GetChar();
          
          eeprom_write_byte((UINT8*)u16Help, u8Help);
          uart_PutChar(RET_DONE);
        }
        break;
        
        case CMD_FLASH:
        {
          for(UINT8 pageIdx=0U; pageIdx < 112U; pageIdx++)
          {
            ALL_LEDS_OFF();
            
            u8Help = (pageIdx % 5U);
            switch(u8Help)
            {
              case 0U: LED1_ON(); break;
              case 1U: LED2_ON(); break;
              case 2U: LED3_ON(); break;
              case 3U: LED4_ON(); break;
              case 4U: LED5_ON(); break;
            }
            
            u8Crc = 0U;
            for(UINT8 byteIdx=0U; byteIdx<64U; byteIdx++)
            {
              au8Page[byteIdx] = uart_GetChar();
              u8Crc += au8Page[byteIdx];
            } // for byteIdx
            u8Help = uart_GetChar();
            if(u8Help != u8Crc)
            {
              uart_PutChar(RET_ERROR);
              break;
            }
            else
            {
              mg_WritePage(pageIdx, au8Page);
              uart_PutChar(RET_OK);
            }
          } // for pageIdx
          uart_PutChar(RET_DONE);
          ALL_LEDS_OFF();
        }
        break;
        
        default:
        {
          uart_PutChar(RET_UNKNOWN);
        }
        break;
      } /* switch */
    }/* while(1) */
  }
  
  uart_PutChar(RET_START);
  appl_Start();
  
  return (1U); /* never reached */
  
} /* End: main() */


E_BOOL mg_WaitForEnter(void)
{
  for(UINT16 i=0U; i < 2000U; i++)
  {
    if(0U == (i & 32U))
    {
      MUX_VOLT();
    }
    else
    {
      MUX_CURR();
    }
    _delay_ms(1U);
    
    if(UCSR0A & _BV(RXC0))
    {
      if(CMD_ENTER == UDR0)
      {
        return (TRUE);
      }
    }
  }
  return (FALSE);
}

void mg_WritePage (UINT8 u8Page, UINT8* au8Data)
{
  eeprom_busy_wait();
  
  boot_page_erase(u8Page*64U);
  boot_spm_busy_wait();      // Wait until the memory is erased.
  
  for (UINT8 wordIdx=0U; wordIdx<SPM_PAGESIZE; wordIdx+=2U)
  {
    // Set up little-endian word.
    UINT16 w = *au8Data++;
    w += (*au8Data++) << 8U;
    
    boot_page_fill ((u8Page*64U) + wordIdx, w);
  }
  
  boot_page_write (u8Page*64U);   // Store buffer in flash page.
  boot_spm_busy_wait();           // Wait until the memory is written.
  
  // Reenable RWW-section again. We need this if we want to jump back
  // to the application after bootloading.
  boot_rww_enable();
}

/* End: Local functions                                                 *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Global functions          *//*!@addtogroup globfunc Global functions *//*@{*/
/* End: Global functions                                                *//*@}*/
/******************************************************************************/

/**** Last line of code                                                    ****/