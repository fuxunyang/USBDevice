/**
  ******************************************************************************
  * @file    usbd_types.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   Universal Serial Bus Device Driver
  *          Global USB Device types
  *
  * Copyright (c) 2018 Benedek Kupper
  *
  * Licensed under the Apache License, Version 2.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *     http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */
#ifndef __USBD_TYPES_H_
#define __USBD_TYPES_H_

#include <usb_types.h>
#include <usbd_config.h>
#include <usbd_pd_def.h>

/** @ingroup USB
 * @defgroup USBD USB Device
 * @{ */

/** @defgroup USBD_Exported_Macros USB Device Exported Macros
 * @{ */

/** @brief Maximum number of USB configurations per device */
#define USBD_MAX_CONFIGURATION_COUNT    1

#define USBD_SERIAL_BCD_SIZE            12

#ifndef USBD_MAX_IF_COUNT
#define USBD_MAX_IF_COUNT               1
#endif

#ifndef USBD_MAX_EP_COUNT
#define USBD_MAX_EP_COUNT               1
#endif

#ifndef USBD_EP0_MAX_PACKET_SIZE
#define USBD_EP0_MAX_PACKET_SIZE        64
#endif

#ifndef USBD_LANGID_STRING
#define USBD_LANGID_STRING              0x409 /* en-US */
#endif

#ifndef USBD_HS_SUPPORT
#define USBD_HS_SUPPORT                 0
#endif

#if !defined(USBD_SPEC_BCD) && (USBD_LPM_SUPPORT != 0)
/* In order to support reading the BOS descriptor
 * (which specifies the LPM support of the device),
 * the bcdUSB has to be increased to 2.01 at least */
#define USBD_SPEC_BCD                   0x0201
#elif !defined(USBD_SPEC_BCD)
#define USBD_SPEC_BCD                   USB_SPEC_BCD
#endif

/** @} */

/** @defgroup USBD_Exported_Types USB Device Exported Types
 * @{ */

/** @brief USB device operation results */
typedef enum
{
    USBD_E_OK = 0,   /*!< Operation successful */
    USBD_E_ERROR,    /*!< Operation failed */
    USBD_E_BUSY,     /*!< Operation rejected due to ongoing activity */
    USBD_E_INVALID,  /*!< Operation rejected due to invalid input */
}USBD_ReturnType;


/** @brief USB string descriptor indexes */
typedef enum
{
    USBD_ISTR_LANGID     = 0, /*!< Fixed by standard */
    USBD_ISTR_VENDOR     = 1, /*!< Vendor name */
    USBD_ISTR_PRODUCT    = 2, /*!< Product name */
    USBD_ISTR_SERIAL     = 3, /*!< Serial number string */
    USBD_ISTR_CONFIG     = 4, /*!< Configuration name */
    USBD_ISTR_INTERFACES      /*!< Interfaces' names */
}USBD_iStringType;


/** @brief USB serial number definition */
typedef const uint8_t USBD_SerialNumberType[USBD_SERIAL_BCD_SIZE / 2];


/** @brief USB device configuration structure */
typedef struct
{
    const char *Name;       /*!< String description of the configuration */
    uint16_t MaxCurrent_mA; /*!< Maximum current demand (2 .. 500 mA) */
    union {
        struct {
            uint8_t : 5;
            uint8_t RemoteWakeup : 1; /*!< Remote wakeup support */
            uint8_t SelfPowered : 1;  /*!< Self powered vs USB bus powered */
            uint8_t : 1;
        };
        uint8_t b;
    };
    uint8_t LPM;            /*!< Link Power Management activation */

#ifdef USBD_PD_CONFIG_FIELDS
    USBD_PD_CONFIG_FIELDS;  /*!< Peripheral Driver specific configuration elements */
#endif
}USBD_ConfigurationType;


/** @brief USB Device descriptors structure */
typedef struct
{
    USBD_ConfigurationType Config;  /*!< Device configuration */

    struct {
        const char *Name;       /*!< Vendor name */
        uint16_t ID;            /*!< Vendor IDentifier (VID) */
    }Vendor;                    /*!< Vendor properties */

    struct {
        const char *Name;       /*!< Product name */
        uint16_t ID;            /*!< Product IDentifier (PID) */
        union {
            struct {
                uint8_t Major;  /*!< Major version */
                uint8_t Minor;  /*!< Minor version */
            };
            uint16_t bcd;       /*!< BCD-coded version number */
        }Version;               /*!< Version number */
    }Product;                   /*!< Product properties */

    USBD_SerialNumberType *SerialNumber;/*!< Product serial number reference */

}USBD_DescriptionType;


/** @brief USB endpoint handle structure */
typedef struct
{
    struct {
        uint8_t *Data;                  /*!< Current data for transfer */
        uint16_t Length;                /*!< Total length of the transfer */
        uint16_t Progress;
    }Transfer;                          /*!< Endpoint data transfer context */
    uint16_t              MaxPacketSize;/*!< Endpoint Max packet size */
    USB_EndPointType      Type;         /*!< Endpoint type */
    USB_EndPointStateType State;        /*!< Endpoint state */
    uint8_t               IfNum;        /*!< Interface index of non-control endpoint */
#ifdef USBD_PD_EP_FIELDS
    USBD_PD_EP_FIELDS;                  /*!< Peripheral Driver specific endpoint context */
#endif
}USBD_EpHandleType;


struct _USBD_HandleType;

struct _USBD_IfHandleType;


/**
 * @brief Generic callback function pointer type
 * @param itf: reference to the callback sender USBD interface
 */
typedef void            ( *USBD_IfCbkType )     ( struct _USBD_IfHandleType * itf );

/**
 * @brief Setup stage callback function pointer type
 * @param itf: reference to the callback sender USBD interface
 * @return OK if the setup request is accepted, INVALID otherwise
 */
typedef USBD_ReturnType ( *USBD_IfSetupCbkType )( struct _USBD_IfHandleType * itf );

/**
 * @brief Interface descriptor callback function pointer type
 * @param itf: pointer of the callback sender handle
 * @param ifNum: the index of the interface in the device
 * @param dest: the destination buffer
 * @return Length of the provided descriptor
 */
typedef uint16_t        ( *USBD_IfDescCbkType ) ( struct _USBD_IfHandleType *itf,
                                                  uint8_t ifNum,
                                                  uint8_t *dest );

/**
 * @brief String reading callback function pointer type
 * @param itf: reference to the callback sender USBD interface
 * @param intNum: interface-internal string index (high nibble of iIndex)
 * @return The referenced string
 */
typedef const char*     ( *USBD_IfStrCbkType )  ( struct _USBD_IfHandleType *itf,
                                                  uint8_t intNum);

/**
 * @brief Endpoint transfer complete callback function pointer type
 * @param itf: reference to the callback sender USBD interface
 * @param ep: reference to the endpoint structure
 */
typedef void            ( *USBD_IfEpCbkType )   ( struct _USBD_IfHandleType *itf,
                                                  USBD_EpHandleType *ep);


/** @brief USB interface class callback (virtual functions) structure */
typedef struct
{
    USBD_IfDescCbkType  GetDescriptor;  /*!< Read the interface descriptor */
    USBD_IfStrCbkType   GetString;      /*!< Read the interface's string */

    USBD_IfCbkType      Init;           /*!< The configuration has been set */
    USBD_IfCbkType      Deinit;         /*!< The configuration is cleared or device shutdown */

    USBD_IfSetupCbkType SetupStage;     /*!< Ctrl EP setup stage with interface recipient */
    USBD_IfCbkType      DataStage;      /*!< Ctrl EP data stage is completed */

    USBD_IfEpCbkType    OutData;        /*!< OUT EP transfer is completed */
    USBD_IfEpCbkType    InData;         /*!< IN EP transfer is completed */
}USBD_ClassType;


/** @brief USB interface handle base structure */
typedef struct _USBD_IfHandleType
{
    struct _USBD_HandleType *Device;    /*!< Reference of the related USB Device */
    const  USBD_ClassType   *Class;     /*!< Reference of the class specific methods */
    uint8_t AltCount;                   /*!< Number of alternate settings */
    uint8_t AltSelector;                /*!< Current alternate setting */
}USBD_IfHandleType;


/** @brief USB Device handle structure */
typedef struct _USBD_HandleType
{
    const USBD_DescriptionType *Desc;       /*!< Reference of the device description */
    USB_SetupRequestType Setup;             /*!< Setup request is stored */
    uint8_t CtrlData[USBD_EP0_BUFFER_SIZE]; /*!< Control EP buffer for common use */

#ifdef USBD_PD_DEV_FIELDS
    USBD_PD_DEV_FIELDS;                     /*!< Peripheral Driver specific device context */
#endif
    USB_LinkStateType LinkState;            /*!< USB link power state (handled by PD) */

    USB_SpeedType Speed;                    /*!< USB current speed */
    union {
        struct {
            uint16_t SelfPowered : 1;       /*!< Self powered vs USB bus powered */
            uint16_t RemoteWakeup : 1;      /*!< Remote wakeup enabled */
            uint16_t : 14;
        };
        uint16_t w;
    }Features;                              /*!< Device feature configuration */
    uint8_t ConfigSelector;                 /*!< Device active configuration index */

    uint8_t IfCount;                                /*!< Number of device interfaces */
    USBD_IfHandleType* IF[USBD_MAX_IF_COUNT];       /*!< Device interface references */

    struct {
        USBD_EpHandleType IN [USBD_MAX_EP_COUNT];   /*!< IN endpoint status */
        USBD_EpHandleType OUT[USBD_MAX_EP_COUNT];   /*!< OUT endpoint status */
    }EP;                                            /*!< Endpoint management */

}USBD_HandleType;

/** @} */

/** @} */

#endif /* __USBD_TYPES_H_ */
