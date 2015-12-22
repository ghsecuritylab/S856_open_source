/**=============================================================================

@file
scveTypes.hpp

@brief
SCVE API Definition containing Common Scve Data Types.

Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

=============================================================================**/


/**=============================================================================
@mainpage Snapdragon CV Engine API Documentation

@version 1.0

@section Overview Overview

SCVE is a set of high-level CV features that Qualcomm provides as part
of their chipset offerings. \n\n

The feature list includes:

- @ref scveObjectTracker \n
- @ref scveImageRemoval \n
- @ref scveImageCloning \n

Support APIs: \n
- @ref scveTypes \n
- @ref scveContext \n
- @ref scveUtils \n

=============================================================================**/

//=============================================================================
///@defgroup scve SCVE
///@brief All modules that are part of SCVE Namespace.
//=============================================================================

//=============================================================================
///@defgroup scveTypes Types
///@brief Defines generic Types used across all SCVE features
///@ingroup scve
//=============================================================================

#ifndef SCVE_TYPES_HPP
#define SCVE_TYPES_HPP

#include <stdint.h>
#include <stddef.h>

namespace SCVE
{

#if defined(WIN32)                               // Microsoft Visual C/C++
    #ifdef MSVC
        #define SCVE_API
    #else
        #define SCVE_API __declspec(dllexport)
    #endif
    #define INLINE __forceinline
#else                                            // Other
    #define SCVE_API __attribute__ ((visibility ("default")))
    #define INLINE inline
#endif

//------------------------------------------------------------------------------
/// @brief
///    Definition of Status type used in SCVE. Wherever appropriate, functions
///    in SCVE APIs return the Status of function execution. This the common
///    Status type used in all SCVE features.
///
/// @ingroup scveTypes
//------------------------------------------------------------------------------
typedef int32_t Status;


//------------------------------------------------------------------------------
/// @brief
///    Enumeration of Base Error codes of all SCVE features. For specific error
///    codes, please refer the specific header file of the feature.
///
/// @ingroup scveTypes
//------------------------------------------------------------------------------
typedef enum
{
   /// Universal success status for all SCVE features
   SCVE_SUCCESS                                         = 0x00000000,

   //Generic error codes
   /// Unspecified internal error occurred
   SCVE_ERROR_INTERNAL                                  = 0xF0000001,
   /// Internal memory allocation failed
   SCVE_ERROR_OUT_OF_MEMORY                             = 0xF0000002,
   /// Invalid input/output parameter supplied
   SCVE_ERROR_INVALID_PARAM                             = 0xF0000003,
   /// Color Format used is not supported
   SCVE_ERROR_UNSUPPORTED_FORMAT                        = 0xF0000004,
   /// Async function called without callback being registed
   SCVE_ERROR_CALLBACK_NOT_REGISTERED                   = 0xF0000005,

   //Base error codes for different features
   /// Base error code for Object-Tracker
   SCVE_OBJECTTRACKER_ERROR                             = 0xF0000100,
   /// Base error code for Face Detection
   SCVE_FACEDETECT_ERROR                                = 0xF0000200,
   /// Base error code for Sparse Feature Detection
   SCVE_SPARSEFEATURE_ERROR                             = 0xF0000400,
   /// Base error code for Smile Detection
   SCVE_SMILEDETECT_ERROR                               = 0xF0000800,
   /// Base error code for Dense Feature Detection
   SCVE_DENSEFEATURE_ERROR                              = 0xF0001000,
   /// Base error code for Image Cloning
   SCVE_IMAGECLONING_ERROR                              = 0xF0002000,
   /// Base error code for Image Cloning
   SCVE_IMAGEREMOVAL_ERROR                              = 0xF0004000,
}StatusCodes;


//------------------------------------------------------------------------------
/// @brief
///    Enumeration representing several Color Formats supported throughout SCVE.
///
/// @ingroup scveTypes
//------------------------------------------------------------------------------
typedef enum
{
   /// 8-bit Greyscale format, with 8 bits-per-pixel
   SCVE_COLORFORMAT_GREY_8BIT                  = 0,
   /// NV21 YUV format, with 12 bits-per-pixel
   SCVE_COLORFORMAT_YUV_NV21                   = 1,
   /// NV12 YUV format, with 12 bits-per-pixel
   SCVE_COLORFORMAT_YUV_NV12                   = 2,
   /// RGB-565 format, with 16 bits-per-pixel
   SCVE_COLORFORMAT_RGB_565                    = 3,
   /// RGB-888 format, with 24 bits-per-pixel
   SCVE_COLORFORMAT_RGB_888                    = 4,
   /// RGBA-8888 format, with 32 bits-per-pixel
   SCVE_COLORFORMAT_RGBA_8888                  = 5
} ColorFormat;


//------------------------------------------------------------------------------
/// @brief
///    Structure representing dimension of an image.
///
/// @param nWidth
///    Width of the Image
///
/// @param nHeight
///    Height of the Image
///
/// @param nStride
///    Stride length of the image in bytes. Number of bytes from beginning of
///    one line of the image to the next.
///
/// @ingroup scveTypes
//------------------------------------------------------------------------------
typedef struct {
   uint32_t   nWidth;
   uint32_t   nHeight;
   uint32_t   nStride;
} ImageDimension;


//------------------------------------------------------------------------------
/// @brief
///    Structure representing an image.
///
/// @param pPtr
///    Pointer to the memory location with contains the raw image data.
///
/// @param sDim
///    Structure representing the dimension of the image.
///
/// @param sFormat
///    Enum representing the color format of the image.
///
/// @ingroup scveTypes
//------------------------------------------------------------------------------
typedef struct {
   uint8_t         *pPtr;
   ImageDimension   sDim;
   ColorFormat      sFormat;
   uint32_t         nChannels;
}Image;


//------------------------------------------------------------------------------
/// @brief
///    Structure representing a Point.
///
/// @param nLocX
///    X co-ordinate
///
/// @param nLocY
///    Y co-ordinate
///
/// @ingroup scveTypes
//------------------------------------------------------------------------------
typedef struct {
   int32_t nLocX;
   int32_t nLocY;
}Point;


//------------------------------------------------------------------------------
/// @brief
///    Structure representing a Rectangle.
///
/// @param nLocX
///    X co-ordinates of the top-left corner of the Rectangle
///
/// @param nLocY
///    Y co-ordinates of the top-left corner of the Rectangle
///
/// @param nWidth
///    Width of the Rectangle
///
/// @param nHeight
///    Height of the Rectangle
///
/// @ingroup scveTypes
//------------------------------------------------------------------------------
typedef struct {
   int32_t  nLocX;
   int32_t  nLocY;
   uint32_t nWidth;
   uint32_t nHeight;
} Rect;


//------------------------------------------------------------------------------
/// @brief
///    Structure representing a Bounding Box.
///
/// @param topLeft
///    co-ordinates of the top-left corner of the Bounding Box
///
/// @param topRight
///    co-ordinates of the top-right corner of the Bounding Box
///
/// @param botLeft
///    co-ordinates of the bottom-left corner of the Bounding Box
///
/// @param botRight
///    co-ordinates of the bottom-right corner of the Bounding Box
///
/// @ingroup scveTypes
//------------------------------------------------------------------------------
typedef struct {
   Point topLeft;
   Point topRight;
   Point botLeft;
   Point botRight;
}BoundingBox;


} //namespace SCVE

#endif //SCVE_TYPES_HPP