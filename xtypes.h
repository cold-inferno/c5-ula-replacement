/*!@addtogroup header Header files*//*@{*/
/**
@file             xtypes.h
@author           Philipp Bank
@copyright        Creative Commons by-nc-sa 3.0
                  (http://creativecommons.org/licenses/by-nc-sa/3.0/)
@brief            Global type definitions
*//*@}*/

#ifndef  _XTYPES_H
#define  _XTYPES_H

#ifdef EXPORT
  #define EXTERN
#else
  #define EXTERN extern
#endif

/******************************************************************************/
/* Constants (#define)                *//*!@addtogroup define Constants *//*@{*/
/* End: define Constants                                                *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Macro definitions (#define)           *//*!@addtogroup macros Macros *//*@{*/
/* End: Macro definitions (#define)                                     *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Basic types (typedef)                 *//*!@addtogroup typedef Types *//*@{*/

typedef           char        CHAR;
typedef           char        INT8;
typedef unsigned  char        UINT8;
typedef           int         INT16;
typedef unsigned  int         UINT16;
typedef           long        INT32;
typedef unsigned  long        UINT32;
typedef           long long   INT64;
typedef unsigned  long long   UINT64;
typedef           float       FLOAT;
typedef           double      DOUBLE;


typedef enum 
{
  FALSE = 0U, 
  TRUE  = 1U
} E_BOOL;


typedef enum 
{
  OFF = 0U, 
  ON  = 1U
} E_SWITCH;


typedef enum
{
  COND_OK           = 0U,
  COND_VOLT_WARN    = 1U,
  COND_CURR_WARN    = 2U,
  COND_TEMP_WARN    = 4U,
  COND_VOLT_ERR     = 8U,
  COND_CURR_ERR     = 16U,
  COND_TEMP_ERR     = 32U
} E_SAFETY_CONDITION;


typedef struct
{
  E_BOOL    bIsPedelec;         // pedelec mode?
  UINT16    u16WheelSize;       // wheel circumflex in mm
  UINT16    u16SpeedLimitMax;   // 10th parts of km/h (i.e. 123 equals 12.3km/h)
  UINT16    u16SpeedLimitMin;   // 10th parts of km/h (i.e. 123 equals 12.3km/h)
  UINT16    u16TempFan;         // fan activation temperature (in °C)
  UINT8     u8NumMagnetsTacho;  // Number of magnets for tacho reed switch
  UINT8     u8NumMagnetsPedal;  // Number of magnets for pedal reed switch
} S_MEM_STRUCTRUE;

/* End: Basic types (typedef)                                           *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Global variables/constan.  *//*!@addtogroup globvar Global variables *//*@{*/
/* End: Global variables                                                *//*@}*/
/******************************************************************************/

/******************************************************************************/
/* Prototype declarations                                                     */
/* End: Prototype declarations                                                */
/******************************************************************************/
#undef EXTERN
#endif /* _XTYPES_H */

/**** Last line of code                                                    ****/
