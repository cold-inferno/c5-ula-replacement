/*!@addtogroup header Header files*//*@{*/
/**
@file             ridelog.h
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
@brief            Header file for module <ridelog.c> (added 2026).

                  Optional ride log in the EEPROM, switched on/off with the
                  setting u8LogEnable (offset 11 of the settings). Layout:

                  offset 0..10   settings (unchanged), 11 = log on (1)/off
                  offset 16..31  header S_RL_HDR
                  offset 32..    event ring buffer, S_RL_EVT records
                  last 306 bytes black box: 6 byte header + 60 S_RL_SAMPLE

                  tools/ula_log.py and the configurator decode this layout;
                  change RL_LAYOUT when changing it.
*//*@}*/

#ifndef RIDELOG_H
#define RIDELOG_H

#ifdef EXPORT
  #define EXTERN
#else
  #define EXTERN extern
#endif

/******************************************************************************/
/* Constants (#define)                *//*!@addtogroup define Constants *//*@{*/
#define RL_LAYOUT            1U     // layout version stored in the header
#define RL_HDR_ADDR         16U
#define RL_EVT_ADDR         32U
#define RL_EVT_SIZE          8U
#define RL_BB_SAMPLES       60U     // black box: last 60 s, one sample per s
#define RL_BB_SAMPLE_SIZE    5U
#define RL_BB_SIZE          (6U + (RL_BB_SAMPLES * RL_BB_SAMPLE_SIZE))
#define RL_EE_SIZE          ((UINT16)E2END + 1U)
#define RL_BB_ADDR          (RL_EE_SIZE - RL_BB_SIZE)
#define RL_EVT_COUNT        ((RL_BB_ADDR - RL_EVT_ADDR) / RL_EVT_SIZE)
#define RL_BB_VALID       0xA5U

/** @brief Wear limits: same event type at most once per RL_MIN_GAP_S
    seconds, at most RL_MAX_EVENTS_PER_RIDE events per power-on.             */
#define RL_MIN_GAP_S        30U
#define RL_MAX_EVENTS_PER_RIDE 40U

/** @brief Temperature ADC values below this mean "thermistor disconnected"
    (the original firmware reads them as 85 °C = temperature error).         */
#define RL_TEMP_SENSOR_LOW  10U

/* event codes */
#define RL_EV_START_POWERON   0x01U
#define RL_EV_START_BROWNOUT  0x02U
#define RL_EV_START_WATCHDOG  0x03U
#define RL_EV_START_EXTERNAL  0x04U
#define RL_EV_START_UNKNOWN   0x05U  // bootloader older than V1044
#define RL_EV_VOLT_WARN       0x10U
#define RL_EV_VOLT_ERR        0x11U
#define RL_EV_CURR_WARN       0x12U
#define RL_EV_CURR_ERR        0x13U
#define RL_EV_TEMP_WARN       0x14U
#define RL_EV_TEMP_ERR        0x15U
#define RL_EV_TEMP_SENSOR     0x16U
#define RL_EV_BLACKBOX_SAVED  0x20U

/* sample flag bits */
#define RL_F_RELAY       0x01U
#define RL_F_BUTTON      0x02U
#define RL_F_PEDALLING   0x04U
#define RL_F_VOLT        0x08U   // voltage warning or error
#define RL_F_CURR        0x10U   // current warning or error
#define RL_F_TEMP        0x20U   // temperature warning or error
#define RL_F_TEMPSENSOR  0x40U   // thermistor reading implausibly low
/* End: define Constants                                                *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Basic types (typedef)                 *//*!@addtogroup typedef Types *//*@{*/

/** @brief EEPROM header (16 bytes)                                           */
typedef struct
{
  UINT8   au8Magic[2];      // 'L', 'G'
  UINT8   u8Layout;         // RL_LAYOUT
  UINT8   u8SizeKb;         // EEPROM size in 256-byte units (2 = 512 B, 4 = 1 KB)
  UINT8   u8RideId;         // increments at every start (wraps)
  UINT8   u8Reserved;
  UINT16  au16Starts[4];    // power-on, brown-out, watchdog, external reset
  UINT16  u16Events;        // events written in total (ring index = % count)
} S_RL_HDR;

/** @brief One event record (8 bytes)                                         */
typedef struct
{
  UINT16  u16Time;          // seconds since power-on
  UINT8   u8Code;           // RL_EV_...
  UINT8   u8RideId;
  UINT8   u8Speed;          // 0.5 km/h
  UINT8   u8Volt;           // battery ADC - 400 (0..255)
  UINT8   u8Curr;           // current ADC / 2
  UINT8   u8Temp;           // °C
} S_RL_EVT;

/** @brief One black box sample (5 bytes)                                     */
typedef struct
{
  UINT8   u8Speed;          // 0.5 km/h
  UINT8   u8Volt;           // battery ADC - 400
  UINT8   u8Curr;           // current ADC / 2
  UINT8   u8Temp;           // °C
  UINT8   u8Flags;          // RL_F_...
} S_RL_SAMPLE;

/** @brief Current measurements handed over by the main loop                 */
typedef struct
{
  UINT32  u32Speed;         // 10th of km/h
  UINT16  u16Volt;          // ADC
  UINT16  u16Curr;          // ADC
  UINT16  u16TempAdc;       // ADC
  UINT8   u8TempC;          // °C
  UINT8   u8Cond;           // E_SAFETY_CONDITION bits
  UINT8   u8Flags;          // RL_F_RELAY / RL_F_BUTTON / RL_F_PEDALLING
} S_RL_INPUT;

/* End: Basic types (typedef)                                           *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Prototype declarations                                                     */
EXTERN void   ridelog_Init    (void);
EXTERN void   ridelog_Sample  (const S_RL_INPUT* psIn);
EXTERN void   ridelog_Check   (const S_RL_INPUT* psIn);
EXTERN E_BOOL ridelog_IsOn    (void);
/* End: Prototype declarations                                                */
/******************************************************************************/

#undef EXTERN
#endif /* RIDELOG_H */

/**** Last line of code                                                    ****/
