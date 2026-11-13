/*!@addtogroup sources Source files*//*@{*/
/**
@file             ridelog.c
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
@brief            Optional ride log in the EEPROM (added 2026).

                  - Reset-cause counters (power-on, brown-out, watchdog,
                    external reset), one header update per start.
                  - Event log: rising edges of warnings/errors and an
                    implausibly low thermistor reading, rate limited.
                  - Black box: the last 60 s (1 sample/s) are always kept in
                    RAM (section .noinit, survives a reset but not power-off).
                    After an unexpected restart (watchdog, brown-out, external
                    reset) they are copied to the EEPROM.

                  Nothing is written to the EEPROM while the log is switched
                  off (setting u8LogEnable != 1). The reset cause is handed
                  over by bootloader V1044+ in GPIOR0 (bit 7 = valid).
*//*@}*/

/******************************************************************************/
/* Includes (#include)                                                        */
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>

/* Own header files */
#include "xtypes.h"
#include "global.h"
#include "memory.h"
#include "timer.h"

#define EXPORT
  #include "ridelog.h"
#undef EXPORT

/******************************************************************************/
/* Local constants and types                                                  */
#define RL_RAM_MAGIC   0xB10CU

typedef struct
{
  UINT16       u16Magic;
  UINT8        u8RideId;
  UINT8        u8Idx;            // next write position
  UINT8        u8Count;          // valid samples (<= RL_BB_SAMPLES)
  UINT16       u16Uptime;        // seconds since power-on of the last sample
  S_RL_SAMPLE  asSample[RL_BB_SAMPLES];
  UINT8        u8Check;
  UINT16       u16Magic2;
} S_RL_RAM;

/* index of the counters in S_RL_HDR.au16Starts */
#define RL_START_POWERON   0U
#define RL_START_BROWNOUT  1U
#define RL_START_WATCHDOG  2U
#define RL_START_EXTERNAL  3U

/******************************************************************************/
/* Local variables                                                            */

/** @brief Black box ring buffer; not initialised at start-up on purpose.    */
STATIC S_RL_RAM mg_sRam __attribute__ ((section (".noinit")));

STATIC E_BOOL   mg_bOn        = FALSE;
STATIC S_RL_HDR mg_sHdr;
STATIC UINT8    mg_u8LastCond = 0U;
STATIC E_BOOL   mg_bTempLow   = FALSE;
STATIC UINT8    mg_u8Written  = 0U;
/** @brief time (s) an event code was last written, index = code bit         */
STATIC UINT16   mg_au16LastEv[8];

/******************************************************************************/
/* Local functions                                                            */

STATIC UINT8 mg_RamCheck (void)
{
  const UINT8* p = (const UINT8*)&mg_sRam;
  UINT8 u8Sum = 0x5AU;
  for (UINT16 i = 0U; i < (UINT16)((const UINT8*)&mg_sRam.u8Check - p); i++)
  {
    u8Sum = (UINT8)((u8Sum << 1) | (u8Sum >> 7)) ^ p[i];
  }
  return (u8Sum);
}

STATIC void mg_RamReset (UINT8 u8RideId)
{
  mg_sRam.u16Magic  = RL_RAM_MAGIC;
  mg_sRam.u8RideId  = u8RideId;
  mg_sRam.u8Idx     = 0U;
  mg_sRam.u8Count   = 0U;
  mg_sRam.u16Uptime = 0U;
  mg_sRam.u16Magic2 = RL_RAM_MAGIC;
  mg_sRam.u8Check   = mg_RamCheck();
}

STATIC E_BOOL mg_RamValid (void)
{
  return ((RL_RAM_MAGIC == mg_sRam.u16Magic) &&
          (RL_RAM_MAGIC == mg_sRam.u16Magic2) &&
          (mg_sRam.u8Idx < RL_BB_SAMPLES) &&
          (mg_sRam.u8Count <= RL_BB_SAMPLES) &&
          (mg_sRam.u8Count > 0U) &&
          (mg_RamCheck() == mg_sRam.u8Check)) ? TRUE : FALSE;
}

STATIC UINT8 mg_Enc8 (UINT16 u16Value)
{
  return ((u16Value > 255U) ? 255U : (UINT8)u16Value);
}

STATIC void mg_Encode (const S_RL_INPUT* psIn, S_RL_SAMPLE* psOut)
{
  psOut->u8Speed = mg_Enc8((UINT16)((psIn->u32Speed > 2550UL) ? 2550UL : psIn->u32Speed) / 5U);
  psOut->u8Volt  = (psIn->u16Volt < 400U) ? 0U : mg_Enc8(psIn->u16Volt - 400U);
  psOut->u8Curr  = mg_Enc8(psIn->u16Curr / 2U);
  psOut->u8Temp  = psIn->u8TempC;
  psOut->u8Flags = psIn->u8Flags;
  if ((psIn->u8Cond & (COND_VOLT_WARN | COND_VOLT_ERR)) != 0U) { psOut->u8Flags |= RL_F_VOLT; }
  if ((psIn->u8Cond & (COND_CURR_WARN | COND_CURR_ERR)) != 0U) { psOut->u8Flags |= RL_F_CURR; }
  if ((psIn->u8Cond & (COND_TEMP_WARN | COND_TEMP_ERR)) != 0U) { psOut->u8Flags |= RL_F_TEMP; }
  if (psIn->u16TempAdc < RL_TEMP_SENSOR_LOW)                   { psOut->u8Flags |= RL_F_TEMPSENSOR; }
}

STATIC void mg_WriteHdr (void)
{
  eeprom_update_block(&mg_sHdr, (void*)RL_HDR_ADDR, sizeof(S_RL_HDR));
}

STATIC void mg_WriteEvent (UINT8 u8Code, const S_RL_INPUT* psIn)
{
  S_RL_EVT    sEvt;
  S_RL_SAMPLE sSmp;
  UINT16      u16Addr;

  if (psIn != 0)
  {
    mg_Encode(psIn, &sSmp);
  }
  else
  {
    sSmp.u8Speed = sSmp.u8Volt = sSmp.u8Curr = sSmp.u8Temp = 0U;
  }
  sEvt.u16Time  = (UINT16)(timer_GetSystemTime() / 1000UL);
  sEvt.u8Code   = u8Code;
  sEvt.u8RideId = mg_sHdr.u8RideId;
  sEvt.u8Speed  = sSmp.u8Speed;
  sEvt.u8Volt   = sSmp.u8Volt;
  sEvt.u8Curr   = sSmp.u8Curr;
  sEvt.u8Temp   = sSmp.u8Temp;

  u16Addr = RL_EVT_ADDR + ((mg_sHdr.u16Events % RL_EVT_COUNT) * RL_EVT_SIZE);
  eeprom_update_block(&sEvt, (void*)u16Addr, sizeof(S_RL_EVT));
  mg_sHdr.u16Events++;
  eeprom_update_block(&mg_sHdr.u16Events,
                      (void*)(RL_HDR_ADDR + (UINT16)((UINT8*)&mg_sHdr.u16Events - (UINT8*)&mg_sHdr)),
                      sizeof(mg_sHdr.u16Events));
  mg_u8Written++;
}

/** @brief Writes an event if logging is on and the wear limits allow it.    */
STATIC void mg_Event (UINT8 u8Code, UINT8 u8Slot, const S_RL_INPUT* psIn)
{
  UINT16 u16Now = (UINT16)(timer_GetSystemTime() / 1000UL);

  if ((FALSE == mg_bOn) || (mg_u8Written >= RL_MAX_EVENTS_PER_RIDE))
  {
    return;
  }
  if ((mg_au16LastEv[u8Slot] != 0xFFFFU) &&
      ((UINT16)(u16Now - mg_au16LastEv[u8Slot]) < RL_MIN_GAP_S))
  {
    return;
  }
  mg_au16LastEv[u8Slot] = u16Now;
  mg_WriteEvent(u8Code, psIn);
}

/** @brief Copies the black box from RAM to the EEPROM, oldest sample first. */
STATIC void mg_SaveBlackBox (UINT8 u8Cause)
{
  UINT8  au8Head[6];
  UINT16 u16Addr = RL_BB_ADDR + 6U;
  UINT8  u8Idx   = (UINT8)((mg_sRam.u8Idx + RL_BB_SAMPLES - mg_sRam.u8Count) % RL_BB_SAMPLES);

  au8Head[0] = 0U;                              /* invalid while writing */
  au8Head[1] = u8Cause;
  au8Head[2] = mg_sRam.u8RideId;
  au8Head[3] = mg_sRam.u8Count;
  au8Head[4] = (UINT8)mg_sRam.u16Uptime;
  au8Head[5] = (UINT8)(mg_sRam.u16Uptime >> 8);
  eeprom_update_block(au8Head, (void*)RL_BB_ADDR, sizeof(au8Head));

  for (UINT8 i = 0U; i < mg_sRam.u8Count; i++)
  {
    eeprom_update_block(&mg_sRam.asSample[u8Idx], (void*)u16Addr, RL_BB_SAMPLE_SIZE);
    u16Addr += RL_BB_SAMPLE_SIZE;
    u8Idx = (UINT8)((u8Idx + 1U) % RL_BB_SAMPLES);
    wdt_reset();                                /* up to ~1 s of EEPROM writes */
  }
  eeprom_update_byte((UINT8*)RL_BB_ADDR, RL_BB_VALID);
}

/******************************************************************************/
/* Global functions                                                           */

/**
@brief  Init function: evaluates the reset cause, updates the counters and
        saves the black box after an unexpected restart (log on only).
        Call once after init_InitModules().
*/
void ridelog_Init (void)
{
  UINT8  u8Reset = GPIOR0;
  UINT8  u8Code;
  UINT8  u8Slot  = 0xFFU;
  E_BOOL bSaveBb = FALSE;

  GPIOR0 = 0U;
  for (UINT8 i = 0U; i < 8U; i++)
  {
    mg_au16LastEv[i] = 0xFFFFU;
  }

  if ((u8Reset & 0x80U) == 0U)          { u8Code = RL_EV_START_UNKNOWN; }
  else if ((u8Reset & (1U << PORF)) != 0U)  { u8Code = RL_EV_START_POWERON;  u8Slot = RL_START_POWERON; }
  else if ((u8Reset & (1U << WDRF)) != 0U)  { u8Code = RL_EV_START_WATCHDOG; u8Slot = RL_START_WATCHDOG; bSaveBb = TRUE; }
  else if ((u8Reset & (1U << BORF)) != 0U)  { u8Code = RL_EV_START_BROWNOUT; u8Slot = RL_START_BROWNOUT; bSaveBb = TRUE; }
  else if ((u8Reset & (1U << EXTRF)) != 0U) { u8Code = RL_EV_START_EXTERNAL; u8Slot = RL_START_EXTERNAL; bSaveBb = TRUE; }
  else                                      { u8Code = RL_EV_START_UNKNOWN; }

  mg_bOn = (1U == memory_Get()->u8LogEnable) ? TRUE : FALSE;

  if (FALSE != mg_bOn)
  {
    eeprom_read_block(&mg_sHdr, (const void*)RL_HDR_ADDR, sizeof(S_RL_HDR));
    if (('L' != mg_sHdr.au8Magic[0]) || ('G' != mg_sHdr.au8Magic[1]) ||
        (RL_LAYOUT != mg_sHdr.u8Layout))
    {
      UINT8* p = (UINT8*)&mg_sHdr;
      for (UINT8 i = 0U; i < sizeof(S_RL_HDR); i++) { p[i] = 0U; }
      mg_sHdr.au8Magic[0] = 'L';
      mg_sHdr.au8Magic[1] = 'G';
      mg_sHdr.u8Layout    = RL_LAYOUT;
      mg_sHdr.u8SizeKb    = (UINT8)(RL_EE_SIZE / 256U);
      eeprom_update_byte((UINT8*)RL_BB_ADDR, 0U);   /* no black box yet */
    }
    mg_sHdr.u8RideId++;
    if ((u8Slot < 4U) && (mg_sHdr.au16Starts[u8Slot] != 0xFFFFU))
    {
      mg_sHdr.au16Starts[u8Slot]++;
    }
    mg_WriteHdr();
    mg_WriteEvent(u8Code, 0);

    if ((FALSE != bSaveBb) && (FALSE != mg_RamValid()))
    {
      mg_SaveBlackBox(u8Code);
      mg_WriteEvent(RL_EV_BLACKBOX_SAVED, 0);
    }
  }

  mg_RamReset(mg_bOn ? mg_sHdr.u8RideId : 0U);
}

/** @brief Returns TRUE if the ride log is switched on.                      */
E_BOOL ridelog_IsOn (void)
{
  return (mg_bOn);
}

/** @brief Adds one black box sample (RAM only). Call once per second.       */
void ridelog_Sample (const S_RL_INPUT* psIn)
{
  mg_Encode(psIn, &mg_sRam.asSample[mg_sRam.u8Idx]);
  mg_sRam.u8Idx = (UINT8)((mg_sRam.u8Idx + 1U) % RL_BB_SAMPLES);
  if (mg_sRam.u8Count < RL_BB_SAMPLES)
  {
    mg_sRam.u8Count++;
  }
  mg_sRam.u16Uptime = (UINT16)(timer_GetSystemTime() / 1000UL);
  mg_sRam.u8Check   = mg_RamCheck();
}

/**
@brief  Detects new warnings/errors and a disconnected thermistor and logs
        them (log on only, rate limited). Call periodically (e.g. 64 ms).
*/
void ridelog_Check (const S_RL_INPUT* psIn)
{
  UINT8 u8New = (UINT8)(psIn->u8Cond & (UINT8)~mg_u8LastCond);
  E_BOOL bTempLow = (psIn->u16TempAdc < RL_TEMP_SENSOR_LOW) ? TRUE : FALSE;

  mg_u8LastCond = psIn->u8Cond;

  if ((u8New & COND_VOLT_WARN) != 0U) { mg_Event(RL_EV_VOLT_WARN, 0U, psIn); }
  if ((u8New & COND_CURR_WARN) != 0U) { mg_Event(RL_EV_CURR_WARN, 1U, psIn); }
  if ((u8New & COND_TEMP_WARN) != 0U) { mg_Event(RL_EV_TEMP_WARN, 2U, psIn); }
  if ((u8New & COND_VOLT_ERR)  != 0U) { mg_Event(RL_EV_VOLT_ERR,  3U, psIn); }
  if ((u8New & COND_CURR_ERR)  != 0U) { mg_Event(RL_EV_CURR_ERR,  4U, psIn); }
  if ((u8New & COND_TEMP_ERR)  != 0U) { mg_Event(RL_EV_TEMP_ERR,  5U, psIn); }
  if ((FALSE != bTempLow) && (FALSE == mg_bTempLow))
  {
    mg_Event(RL_EV_TEMP_SENSOR, 6U, psIn);
  }
  mg_bTempLow = bTempLow;
}

/**** Last line of code                                                    ****/
