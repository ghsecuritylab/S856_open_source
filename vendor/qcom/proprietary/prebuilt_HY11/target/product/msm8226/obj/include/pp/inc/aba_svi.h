#ifndef ABA_SVI_H
#define ABA_SVI_H

#ifdef __cplusplus
extern "C"
{
#endif

  /*=================================================================================================

  File: aba_avi.h

  DESCRIPTION
  This file contains the declaration for types that are specific to SVI.
  These types are SVI configuration parameters.

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  =================================================================================================*/

  /*=================================================================================================
  Defines
  =================================================================================================*/

#ifndef TRUE
#define TRUE   1   /* Boolean true value. */
#endif

#ifndef FALSE
#define FALSE  0   /* Boolean false value. */
#endif

#ifndef _BOOL32_DEFINED
  typedef  unsigned long int  bool32;        /* Boolean value type. */
#define _BOOL32_DEFINED
#endif

#define ABA_SVI_LUX_ADJUSTMENT_TABLE_LENGTH 32
#define ABA_SVI_VERSION   1

  /*=================================================================================================
  Types
  =================================================================================================*/
#include <stdint.h>

  /* CablInitialConfigType
  Defines generic SVI parameters. Use when get/set-ing default/OEM parameters.
  */
  typedef struct
  {
    uint32_t   uContentBoostingFactor;               // 0 - 255, histogram segmentation (content based contrast gain)
    uint32_t   uTemporalFilterFactor;                // Default speed of the temporal filter
    uint32_t   uLuxBoostingFactor;                   // 0 - 255, whole histogram (ambient light based contrast gain)
    uint32_t   uDarkPixBrightnessGain;               // 0 - 255, use to control maximal low centroid boost ratio
    uint32_t   uBrightPixBrightnessGain;             // 0 - 255, use to control the high centroid location
    uint32_t  *pSensorMappingTableReference;         // Lux Mapping table for sensor calibration - reference axis
    uint32_t  *pSensorMappingTableSensor;            // Lux Mapping table for sensor calibration - sensor axis
    uint32_t   uSensorMappingTableLength;            // Lux Mapping table length
    uint32_t  *pBrightnessStrengthMappingLux;        // Brightness strength mapping table - lux axis
    uint32_t  *pBrightnessStrengthMappingFactor;     // Brightness strength mapping table - boosting factor axis
    uint32_t   uBrightnessMappingTableLength;        // Brightness strength mapping table length
    uint32_t   uPanelReflectanceRatio;               // Reflectance ratio for the panel
    uint32_t   uPanelPeakBrightness;                 // Peak brightness for the panel
    uint32_t  *pBacklightResponseTableInput;         // Backlight mapping table - input(user) axis
    uint32_t  *pBacklightResponseTableOutput;        // Backlight mapping table - output axis
    uint32_t   uBacklightResponseTableLength;        // Backlight Mapping table length
    uint32_t   uBacklightReductionRatio;             // Value 0-1024 indicating the reduction ratio relative to the original backlight
    bool32     bEnableBacklightReduction;            // TRUE or FALSE, use to enable backlight/brightness reduction
  } SVIConfigParametersType;

#ifdef __cplusplus
}
#endif

#endif /* ABA_SVI_H */
