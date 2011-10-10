/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief USB host hub support header file
//!
//! - Compiler:           IAR EWAVR and GNU GCC for AVR
//! - Supported devices:  AT90USB1287, AT90USB1286, AT90USB647, AT90USB646
//!
//! \author               Atmel Corporation: http://www.atmel.com \n
//!                       Support and FAQ: http://support.atmel.no/
//!
//! ***************************************************************************

/* Copyright (c) 2009 Atmel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an Atmel
 * AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE EXPRESSLY AND
 * SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _USB_HOST_HUB_H_
#define _USB_HOST_HUB_H_

//_____ I N C L U D E S ____________________________________________________


#include "usb_task.h"

//_____ M A C R O S ________________________________________________________


//_____ S T A N D A R D    D E F I N I T I O N S ___________________________


#ifndef HUB_CLASS
   #define HUB_CLASS 0x09
#endif

//Hub class features selectors
#define C_HUB_LOCAL_POWER     0
#define C_HUB_OVER_CURRENT    1
#define PORT_CONNECTION       0
#define PORT_ENABLE           1
#define PORT_SUSPEND          2
#define PORT_OVER_CURRENT     3
#define PORT_RESET            4
#define PORT_POWER            8
#define PORT_LOW_SPEED        9
#define C_PORT_CONNECTION     16
#define C_PORT_ENABLE         17
#define C_PORT_SUSPEND        18
#define C_PORT_OVER_CURRENT   19
#define C_PORT_RESET          20
#define PORT_TEST             21
#define PORT_INDICATOR        22


#define NB_PORT_OFFSET        2


#define HUB_DEVICE_POWERED           2
#define HUB_DEVICE_CONNECT           3
#define HUB_DEVICE_WAIT_RST_1        4
#define HUB_DEVICE_WAIT_RST_2        5
#define HUB_DEVICE_READY             6


#define HUB_PORT_CONNECTION          0
#define HUB_PORT_RESET               1
#define HUB_PORT_DISCONNECT          2

#if (USB_HUB_SUPPORT==ENABLE)

extern  U8 f_hub_port_disconnect;
extern  U8 hub_device_address[USB_MAX_HUB_NUMBER];
extern  U8 hub_port_state[USB_MAX_HUB_NUMBER][HUB_MAX_NB_PORT];
extern  U8 nb_hub_present;


U32 get_port_status(U8 port_number);
void hub_manage_port_change_status(U8 port_change_status, U8 hub_number);
void enumerate_hub_port_device(U8 hub_port, U8 port_ev, U8 hub_number);
void hub_init(U8 hub_number);

/**
 * Clear_hub_feature
 *
 * @brief this function send a Clear_hub_feature request
 *
 * @param U8 feature_selector
 *
 * @return status
 */
#define Clear_hub_feature(feature_selector)        (usb_request.bmRequestType = USB_SETUP_SET_CLASS_DEVICE,\
                                           usb_request.bRequest      = SETUP_CLEAR_FEATURE,\
                                           usb_request.wValue        = (U16)feature_selector,\
                                           usb_request.wIndex        = 0,\
                                           usb_request.wLength       = 0,\
                                           usb_request.uncomplete_read = FALSE,\
                                           host_send_control(data_stage))

/**
 * Clear_port_feature
 *
 * @brief this function send a Clear port feature request
 *
 * @param U8 feature_selector
 * @param U8 port
 *
 * @return status
 */
#define Clear_port_feature(feature_selector,port)        (usb_request.bmRequestType = USB_SETUP_SET_CLASS_OTHER,\
                                           usb_request.bRequest      = SETUP_CLEAR_FEATURE,\
                                           usb_request.wValue        = (U16)feature_selector,\
                                           usb_request.wIndex        = port,\
                                           usb_request.wLength       = 0,\
                                           usb_request.uncomplete_read = FALSE,\
                                           host_send_control(data_stage))

/**
 * Get_hub_descriptor
 *
 * @brief this function send a get hub dscriptor request
 *
 * @return status
 */
#define Get_hub_descriptor()              (usb_request.bmRequestType = USB_SETUP_GET_CLASS_DEVICE,\
                                           usb_request.bRequest      = SETUP_GET_DESCRIPTOR,\
                                           usb_request.wValue        = 0x2900,\
                                           usb_request.wIndex        = 0,\
                                           usb_request.wLength       = 0xFF,\
                                           usb_request.uncomplete_read = FALSE,\
                                           host_send_control(data_stage))

/**
 * Get_hub_status
 *
 * @brief this function send a get hub status request
 *
 * @return status
 */
#define Get_hub_status()              (usb_request.bmRequestType = USB_SETUP_GET_CLASS_DEVICE,\
                                           usb_request.bRequest      = SETUP_GET_STATUS,\
                                           usb_request.wValue        = 0,\
                                           usb_request.wIndex        = 0,\
                                           usb_request.wLength       = 4,\
                                           usb_request.uncomplete_read = FALSE,\
                                           host_send_control(data_stage))

/**
 * Get_port_status
 *
 * @brief this function send a get port status request
 *
 * @param U8 port
 * @return status
 */
#define Get_port_status(port)              (usb_request.bmRequestType = USB_SETUP_GET_CLASS_OTHER,\
                                           usb_request.bRequest      = SETUP_GET_STATUS,\
                                           usb_request.wValue        = 0,\
                                           usb_request.wIndex        = port,\
                                           usb_request.wLength       = 4,\
                                           usb_request.uncomplete_read = FALSE,\
                                           host_send_control(data_stage))


/**
 * Set_hub_feature
 *
 * @brief this function send a set hub feature request
 *
 * @param U8 feature_selector
 *
 * @return status
 */
#define Set_hub_feature(feature)              (usb_request.bmRequestType = USB_SETUP_SET_CLASS_DEVICE,\
                                           usb_request.bRequest      = SETUP_SET_FEATURE,\
                                           usb_request.wValue        = (U16)feature,\
                                           usb_request.wIndex        = 0,\
                                           usb_request.wLength       = 0,\
                                           usb_request.uncomplete_read = FALSE,\
                                           host_send_control(data_stage))


/**
 * Set_port_feature
 *
 * @brief this function send a set port feature request
 *
 * @param U8 feature_selector
 * @param U8 port
 *
 * @return status
 */
#define Set_port_feature(feature,port)              (usb_request.bmRequestType = USB_SETUP_SET_CLASS_OTHER,\
                                           usb_request.bRequest      = SETUP_SET_FEATURE,\
                                           usb_request.wValue        = (U16)feature,\
                                           usb_request.wIndex        = port,\
                                           usb_request.wLength       = 0,\
                                           usb_request.uncomplete_read = FALSE,\
                                           host_send_control(data_stage))



#define Set_port_indicator_green(i)             Set_port_feature(PORT_INDICATOR,0x0200+i)
#define Set_port_indicator_amber(i)             Set_port_feature(PORT_INDICATOR,0x0100+i)
#define Set_port_indicator_auto(i)              Set_port_feature(PORT_INDICATOR,0x0300+i)
#define Set_port_indicator_off(i)               Set_port_feature(PORT_INDICATOR,0x0000+i)

#define Set_port_reset(i)                       Set_port_feature(PORT_RESET,i)
#define Set_port_suspend(i)                     Set_port_feature(PORT_SUSPEND,i)
#define Clear_port_suspend(i)                   Clear_port_feature(PORT_SUSPEND,i)

#define Set_port_power(i)                       Set_port_feature(PORT_POWER,i);
#define Clear_port_power(i)                     Clear_port_feature(PORT_POWER,i);



//! @}

//! Table to store the device address of the hub connected in the USB tree 
extern U8 hub_device_address[USB_MAX_HUB_NUMBER];

//! The number of hub connected in the USB tree
extern U8 nb_hub_present;

#endif

#endif  // _USB_HOST_HUB_H_

