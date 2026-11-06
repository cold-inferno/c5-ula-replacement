/*!@addtogroup header Header files*//*@{*/
/**
@file             global.h
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
@brief            Global definitions and macros
*//*@}*/
#ifndef _GLOBAL_H
#define _GLOBAL_H

#define EXTERN extern

/******************************************************************************/
/* Constants (#define)                *//*!@addtogroup define Constants *//*@{*/

/******************************************************************************/
/* USER EDITABLE CONSTANTS (#define)                                          */

/** @brief  This defines the wheel circumference in millimeters.
        
        This value will be stored in EEPROM memory.
        You may edit this value according to your wheel size.                 */
#define WHEEL_SIZE                 1280U

/** @brief  This value defines the maximum speed at which the motor can be 
        activated in pedelec mode while the driver is pedaling.
        
        The value is represented in 10th of km/h (250 equals 25.0 km/h) and 
        will be stored in EEPROM memory.
        WARNING:
        You may edit this value but doing so might be illegal in your country!*/
#define SPEEDLIMIT_MAX              250U

/** @brief  This value defines the minimum speed required to enable the motor.
        
        The value is represented in 10th of km/h (30 equals 3.0 km/h) and
        will be stored in EEPROM memory.                                      */
#define SPEEDLIMIT_MIN               30U

/** @brief  This value defines the default number of magnets attached to 
        activate the tacho reed switch.                                                    */
#define DEFAULT_MAGNET_NUMBER_TACHO   1U

/** @brief  This value defines the default number of magnets attached to
        activate the pedal reed switch.                                                    */
#define DEFAULT_MAGNET_NUMBER_PEDAL   1U

/******************************************************************************/
/* BATTERY PROFILE (added 2026)                                               */
/*                                                                            */
/* Select exactly one battery type (or pass -DBATTERY_... to make):           */
/*   BATTERY_LIFEPO4    12.8 V LiFePO4 pack (4 cells in series, with BMS)     */
/*   BATTERY_LEAD_ACID  original 12 V lead-acid behaviour (VREF pot cut-off)  */
/******************************************************************************/
#if !defined(BATTERY_LIFEPO4) && !defined(BATTERY_LEAD_ACID)
#define BATTERY_LIFEPO4
#endif

/** @brief  Converts millivolts to the battery voltage ADC value.
        Derived from the original calibration constants (10.0 V = 448,
        12.0 V = 544, i.e. 48 counts per volt). Check it against a multimeter:
        the serial output field VOLT shows the raw ADC value.                 */
#define VOLT_ADC(mV)    ((UINT16)(((UINT32)(mV) * 48UL) / 1000UL - 32UL))

#if defined(BATTERY_LIFEPO4)
/* LiFePO4 voltages are evaluated while the motor relay has been off for a
   few seconds ("resting" voltage), because the pack voltage curve is very
   flat and the voltage drop under motor load would hide the state of charge.
   Typical resting voltages of a 12.8 V pack: 13.3 V ~90 %, 13.15 V ~55 %,
   13.05 V ~35 %, 12.9 V ~20 %, 12.5 V ~14 %, 12.0 V ~9 %.                    */
  #define BATTERY_NAME          "LiFePO4"
  /** @brief LED bar thresholds (resting voltage), LED1 = lowest              */
  #define VOLT_LED1             VOLT_ADC(12500U)
  #define VOLT_LED2             VOLT_ADC(12900U)
  #define VOLT_LED3             VOLT_ADC(13050U)
  #define VOLT_LED4             VOLT_ADC(13150U)
  #define VOLT_LED5             VOLT_ADC(13300U)
  /** @brief Low battery warning below / cleared above (resting voltage)      */
  #define VOLT_WARN_ON          VOLT_ADC(12800U)
  #define VOLT_WARN_OFF         VOLT_ADC(12900U)
  /** @brief Motor cut-off below / re-enabled above (resting voltage)         */
  #define VOLT_ERR_ON           VOLT_ADC(12000U)
  #define VOLT_ERR_OFF          VOLT_ADC(12300U)
  /** @brief Motor cut-off if the 20 s average under load drops below this.
      Protects the cells and avoids a sudden BMS shutdown while driving.     */
  #define VOLT_ERR_LOADED       VOLT_ADC(11000U)
  /** @brief Relay-off time before a reading counts as resting voltage, in
      512 ms steps (6 = about 3 s)                                            */
  #define VOLT_REST_SETTLE      6U
#else
  #define BATTERY_NAME          "lead-acid"
  /* original lead-acid behaviour: LED bar from the 20 s average,
     cut-off adjustable with the VREF potentiometer in the POD */
  #define VOLT_LED1             VOLT_ADC(10000U)
  #define VOLT_LED2             VOLT_ADC(10500U)
  #define VOLT_LED3             VOLT_ADC(11000U)
  #define VOLT_LED4             VOLT_ADC(11500U)
  #define VOLT_LED5             VOLT_ADC(12000U)
  #define VOLT_REST_SETTLE      6U
#endif
/******************************************************************************/

/******************************************************************************/
/* OTHER CONSTANTS (#define)                                                  */
/* WARNING:                                                                   */
/* Do NOT change anything below this line. Except you know what you are doing.*/

/******************************************************************************/
/* PIN CONFIGURATION (#define)                                                */

/* RS232 RXD */
#define RXD_PORT            PORTD
#define RXD_DDR             DDRD
#define RXD_BIT             PD0

/* RS232 TXD */
#define TXD_PORT            PORTD
#define TXD_DDR             DDRD
#define TXD_BIT             PD1

/* LEDs */
#define LED1_PORT           PORTB
#define LED1_DDR            DDRB
#define LED1_BIT            PB1

#define LED2_PORT           PORTB
#define LED2_DDR            DDRB
#define LED2_BIT            PB0

#define LED3_PORT           PORTD
#define LED3_DDR            DDRD
#define LED3_BIT            PD7

#define LED4_PORT           PORTD
#define LED4_DDR            DDRD
#define LED4_BIT            PD6

#define LED5_PORT           PORTD
#define LED5_DDR            DDRD
#define LED5_BIT            PD4

/* Multiplex */
#define MUX_VOLT_PORT       PORTC
#define MUX_VOLT_DDR        DDRC
#define MUX_VOLT_BIT        PC4

#define MUX_CURR_PORT       PORTC
#define MUX_CURR_DDR        DDRC
#define MUX_CURR_BIT        PC5

/* Speaker */
#define SPEAKER_PORT        PORTB
#define SPEAKER_DDR         DDRB
#define SPEAKER_BIT         PB2

/* Motor relay */
#define RELAY_PORT          PORTD
#define RELAY_DDR           DDRD
#define RELAY_BIT           PD5

/* Fan */
#define FAN_PORT            PORTC
#define FAN_DDR             DDRC
#define FAN_BIT             PC3

/* Tacho sensor */
#define INT0_PORT           PORTD
#define INT0_DDR            DDRD
#define INT0_BIT            PD2

/* Pedal sensor */
#define INT1_PORT           PORTD
#define INT1_DDR            DDRD
#define INT1_BIT            PD3

/* VREF */
#define VREF_PORT           PORTC
#define VREF_DDR            DDRC
#define VREF_BIT            7

/* CSBASE (current) */
#define CSBASE_PORT         PORTC
#define CSBASE_DDR          DDRC
#define CSBASE_BIT          PC1

/* R3 (voltage) */
#define R3_PORT             PORTC
#define R3_DDR              DDRC
#define R3_BIT              PC0

/* VTH (temperature) */
#define VTH_PORT            PORTC
#define VTH_DDR             DDRC
#define VTH_BIT             PC2

/* SWITCH */
#define SWITCH_PORT         PORTC
#define SWITCH_DDR          DDRC
#define SWITCH_BIT          6

/* ADC assignment */
#define ADC_SWITCH          ADC6
#define ADC_TEMPERATURE     ADC2
#define ADC_VOLTAGE         ADC0
#define ADC_VREF            ADC7
#define ADC_CURRENT         ADC1

/******************************************************************************/

/******************************************************************************/

/** @brief  This value defines the maximum current as ADC value
        
        The value is calculatet as:
        523 = (140A * 0.001Ohm * (1+ 47k/2.7k) )/5.04V * 2^16
        With
        140A          = max. motor current
        0.001Ohm      = shunt resistor
        1+ 47k/2.7k   = OPV (IC2) gain
        5.04V         = 5VB = AREF
        2^16          = ADC range (16 bit resolution)                         */
#define MAX_CURRENT         523U

/** @brief  This value defines the fan activation temperature                 */
#define VALUE_TEMP_FAN        45U // 45°C

/** @brief  This value defines temperature warning threshold                  */
#define VALUE_TEMP_WARN       70U // 70°C

/** @brief  This value defines temperature error threshold                    */
#define VALUE_TEMP_ERROR      80U // 80°C

/** @brief This value is just a difinition for one minute in milliseconds     */
#define ONE_MINUTE_MS       ((UINT32)60000UL)

/** @brief This value defines the firmware version which will be stored in 
    flash memory.                                                             */
#define FW_VERSION          1200U  // 2026: LiFePO4 + pedelec fixes

/** @brief This value defines the firmware CRC32 checksum version which will be 
    stored in flash memory.                                                   */
#define APPLICATION_CRC     0xFFFFFFFFUL // patched after build by tools/fwcrc.py

#define CRC32POLY           0xEDB88320UL

/* Just some defines to ease compability with other development environments: */
#define STATIC              static
#define LOCAL_STATIC        static
#define CONST               const
#define VOLATILE            volatile

/* Replacement for eeprom section                                             */
#ifndef SECTION_EEMEM
#define SECTION_EEMEM       __attribute__ ((section (".eeprom")))
#endif

#ifndef SECTION_VERSION
#define SECTION_VERSION     __attribute__ ((section (".app_ver")))
#endif

#ifndef SECTION_CRC
#define SECTION_CRC         __attribute__ ((section (".app_crc")))
#endif

/* End: define Constants                                                *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Macro definitions (#define)           *//*!@addtogroup macros Macros *//*@{*/

/** @brief This macro returns the higher byte of an UINT16 value              */
#define HIGH_BYTE(x) ((UINT8) (x >> 8U))

/** @brief This macro returns the lower byte of an UINT16 value               */
#define LOW_BYTE(x)  ((UINT8)  x)

/** @brief Macro function to turn LED1 on                                     */
#define LED1_ON()       LED1_PORT &= ~(1U << LED1_BIT);
/** @brief Macro function to turn LED1 off                                    */
#define LED1_OFF()      LED1_PORT |=  (1U << LED1_BIT);

/** @brief Macro function to turn LED2 on                                     */
#define LED2_ON()       LED2_PORT &= ~(1U << LED2_BIT);
/** @brief Macro function to turn LED2 off                                    */
#define LED2_OFF()      LED2_PORT |=  (1U << LED2_BIT);

/** @brief Macro function to turn LED3 on                                     */
#define LED3_ON()       LED3_PORT &= ~(1U << LED3_BIT);
/** @brief Macro function to turn LED3 off                                    */
#define LED3_OFF()      LED3_PORT |=  (1U << LED3_BIT);

/** @brief Macro function to turn LED4 on                                     */
#define LED4_ON()       LED4_PORT &= ~(1U << LED4_BIT);
/** @brief Macro function to turn LED4 off                                    */
#define LED4_OFF()      LED4_PORT |=  (1U << LED4_BIT);

/** @brief Macro function to turn LED5 on                                     */
#define LED5_ON()       LED5_PORT &= ~(1U << LED5_BIT);
/** @brief Macro function to turn LED5 off                                    */
#define LED5_OFF()      LED5_PORT |=  (1U << LED5_BIT);

/** @brief Short macro to turn all LEDs on (used for flashing)                */
#define ALL_LEDS_ON()   {LED1_ON(); LED2_ON(); LED3_ON(); \
                         LED4_ON(); LED5_ON();}

/** @brief Short macro to turn all LEDs off (used for flashing)               */
#define ALL_LEDS_OFF()  {LED1_OFF(); LED2_OFF(); LED3_OFF(); \
                         LED4_OFF();LED5_OFF();}

/** @brief Macro to switch the multiplexer to current mode                    */
#define MUX_CURR()      { MUX_CURR_PORT |=  (1U << MUX_CURR_BIT); \
                          MUX_VOLT_PORT &= ~(1U << MUX_VOLT_BIT); }

/** @brief Macro to switch the multiplexer to voltage mode                    */
#define MUX_VOLT()      { MUX_VOLT_PORT |=  (1U << MUX_VOLT_BIT); \
                          MUX_CURR_PORT &= ~(1U << MUX_CURR_BIT); }

/* End: Macro definitions (#define)                                     *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Basic types (typedef)                 *//*!@addtogroup typedef Types *//*@{*/
/* End: Basic types (typedef)                                           *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Global variables/constan.  *//*!@addtogroup globvar Global variables *//*@{*/
#ifndef BOOTLOADER
EXTERN const UINT32           g_u32ApplicationCRC;
EXTERN const UINT16           g_u16ApplicationVersion;
#endif
EXTERN const S_MEM_STRUCTRUE  sMemoryEEPROM;
/* End: Global variables                                                *//*@}*/
/******************************************************************************/

#undef EXTERN
#endif //_GLOBAL_H

/**** Last line of code                                                    ****/
