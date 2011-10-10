/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file manages the usb host hub requests and enumeration of the devices connected to the hub.
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

//_____  I N C L U D E S ___________________________________________________


#include "config.h"
#include "conf_usb.h"
#include "usb_host_hub.h"
#include "usb_host_task.h"
#include "usb_host_enum.h"
#include "usb_host_hub.h"
#include "usb_drv.h"


#if (USB_HUB_SUPPORT == ENABLE)

#ifndef HUB_MAX_NB_PORT
#warning Using default value for HUB_MAX_NB_PORT
   #define HUB_MAX_NB_PORT   4
#endif

//_____ M A C R O S ________________________________________________________

//_____ D E F I N I T I O N S ______________________________________________

//_____ D E C L A R A T I O N S ____________________________________________


//! Enumeration state machine for each port of each hub
U8 hub_port_state[USB_MAX_HUB_NUMBER][HUB_MAX_NB_PORT];

//! Table to store the device address of the hub connected in the USB tree 
U8 hub_device_address[USB_MAX_HUB_NUMBER];

//! Flag to store device connection event
U8 f_hub_port_disconnect=FALSE;

//! The number of hub connected in the USB tree
U8 nb_hub_present;


/**
 * @brief This function initializes the enumeration state machines for each hub port
 *
 * @param U8 hub_number
 *
 * @return void
 *
 */
void hub_init(U8 hub_number)
{
   U8 i;

   for(i=0;i<HUB_MAX_NB_PORT;i++)
   {
      hub_port_state[hub_number][i]=HUB_DEVICE_POWERED;
   }
}



/**
 * @brief This function returns the status of the specified usb hub port
 *
 * @param U8 port_number
 *
 * @return status
 *
 */
U32 get_port_status(U8 port_number)
{
   U32 to_return;
   
   Get_port_status(port_number);
   LSB3(to_return)=data_stage[3];
   LSB2(to_return)=data_stage[2];
   LSB1(to_return)=data_stage[1];
   LSB0(to_return)=data_stage[0];
   return to_return;
}



/**
 * @brief This function decodes the change status repport from the hub interrupt pipe.
 *
 * The function decodes hub port event and call the "enumerate_hub_port_device" function to
 * manage the enumeration of the device connected to the hub port.
 *
 * @param U8 port_change_status
 * @param U8 hub_number the number of the hub taht detects the event
 *
 * @return void
 *
 */
void hub_manage_port_change_status(U8 port_change_status, U8 hub_number)
{
   U8 i,c;
   U32 port_status;
   U8 port_addr_0_in_use=0;
   
   // There should be one and only one device on the bus responding to device address 0
   // Look in the hub_port_state state machine if one device is currently using this address
   // If another hub is currently using address zero, the function returns imediately
   for(i=0;i<USB_MAX_HUB_NUMBER;i++)
   {
      for(c=1;c<=HUB_MAX_NB_PORT;c++)
      {
         if((hub_port_state[i][c-1] != HUB_DEVICE_POWERED) && (hub_port_state[i][c-1] != HUB_DEVICE_READY))
         {
            //Found one port with one device using the address 0
            // memorize this port number
            if(i!=(hub_number-1)) 
            {  return; } //Other hub currently using address zero
            // ok the hub using the device address zero is the hub that currently generates the event (port_change_status)
            port_addr_0_in_use=c; 
            break;        
         }
      }
   }
   
   // For all ports 
   for(c=1;c<=HUB_MAX_NB_PORT;c++)
   {
      // One device connected to one port is using device address zero
      // Skip all other port activity management
      if((port_addr_0_in_use!=0) && (c!=port_addr_0_in_use) )
      {
         continue;
      }
      
      // Does this port generates port change status ?
      if(port_change_status & (1<<c))
      {
         // Get port status bytes
         port_status=get_port_status(c);
         // For each port event, send this event to the function that manage the 
         // state of the device connected to the hub port
         if( port_status & ((U32)1<<C_PORT_CONNECTION))
         {
            Clear_port_feature(C_PORT_CONNECTION,c);
            if(port_status&((U32)1<<PORT_CONNECTION))
            {
               // Port connection low speed -> Unsupported (no PRE token in current OTG interface implementation)
               if(port_status&((U32)1<<PORT_LOW_SPEED))
               {

               }
               else // Port Connection full speed : OK
               {
                  enumerate_hub_port_device(c,HUB_PORT_CONNECTION,hub_number);
               }
            }
            else
            {
               // Port disconnection
               enumerate_hub_port_device(c,HUB_PORT_DISCONNECT,hub_number);
            }
         }
         else if( port_status & ((U32)1<<C_PORT_SUSPEND))
         {
            // Port Suspend state
            Clear_port_feature(C_PORT_SUSPEND,c);
         }
         else if( port_status & ((U32)1<<C_PORT_RESET))
         {
            // Port Reset sent
            Clear_port_feature(C_PORT_RESET,c);
            enumerate_hub_port_device(c,HUB_PORT_RESET,hub_number);            
         }
      }

      // Previous port using device address zero abort all port device management  
      // (There should be one and only device with adddress zero on the USB tree)
      if((hub_port_state[hub_number-1][c-1] != HUB_DEVICE_POWERED) && (hub_port_state[hub_number-1][c-1] != HUB_DEVICE_READY))
      {
         break;  // Break port loop and exit from function      
      }
   }
}

/**
 * @brief Manage the enumeration process of the device connected to the hub port number specified
 *
 * @param U8 hub_port: the usb hub downstream port number
 * @param U8 port_ev: the hub port event detected
 *
 * @return none
 *
 */
void enumerate_hub_port_device(U8 hub_port, U8 port_ev, U8 hub_number)
{
   U8 device_index=0;
   U8 hub_device_index=0;
   U8 nb_port,c;
   
   // Find the device index of the hub_number
   for(c=0;c<MAX_DEVICE_IN_USB_TREE;c++)
   {
      if(usb_tree.device[c].device_address==hub_device_address[hub_number-1])
      {
         hub_device_index=c;         
      }
   }
   
   // Find the device index in linked to the specified hub port    
   // -> look in the usb_tree structure
   for(c=0;c<MAX_DEVICE_IN_USB_TREE;c++)
   {
      if((usb_tree.device[c].hub_port_nb == hub_port) && (usb_tree.device[c].parent_hub_number==hub_number))
      {
         device_index=c;
         break;
      }
   }

   // Port disconnection detected -> delete device entry and RAZ state machine
   // and return ...
   if(port_ev==HUB_PORT_DISCONNECT)
   {
      remove_device_entry(device_index);  // Remove device entry in usb tree
      usb_tree.nb_device--; 
      hub_port_state[hub_number-1][hub_port-1]=HUB_DEVICE_POWERED; // RAZ state machine 
      f_hub_port_disconnect = TRUE;   // Flag for device disconnection signaling
      return;
   }
   
   // Device enumeration state machine for hub port
   switch(hub_port_state[hub_number-1][hub_port-1])
   {
      // New device connection
      // -> Generate first USB reset for the device
      case HUB_DEVICE_POWERED:
         if(port_ev==HUB_PORT_CONNECTION)
         {
            device_index=usb_tree.nb_device++;
            usb_tree.device[device_index].hub_port_nb=hub_port;
            usb_tree.device[device_index].parent_hub_number=hub_number;
            Host_select_device(hub_device_index);
            Set_port_reset(hub_port);
            hub_port_state[hub_number-1][hub_port-1]=HUB_DEVICE_WAIT_RST_1;
         }
         break;
         
      // Wait First reset successfully generated then:
      // -> Get device descriptor to get endpoint zero lenght
      // -> Generate second USB reset for the device
      case HUB_DEVICE_WAIT_RST_1:
         if(port_ev==HUB_PORT_RESET)
         {
            c = 0;
            while (c<250)               // wait 250ms after USB reset
            {
               if (Is_usb_event(EVT_HOST_SOF)) { Usb_ack_event(EVT_HOST_SOF); c++; }// Count Start Of frame
               if (Is_host_emergency_exit() || Is_usb_bconnection_error_interrupt()) 
               {hub_port_state[hub_number-1][hub_port-1]=HUB_DEVICE_POWERED; return;}
            }
            Host_select_device(device_index);
            // By default assumes its ctrl endpoint is 8 bytes only
            usb_tree.device[device_index].ep_ctrl_size=8;
            // Get its device descriptor to get ctrl endpoint size 
            if( CONTROL_GOOD == host_get_device_descriptor_uncomplete())
            {
               Host_select_device(hub_device_index);
               usb_tree.device[device_index].ep_ctrl_size=data_stage[OFFSET_FIELD_MAXPACKETSIZE];
               Set_port_reset(hub_port);         // Second USB reset
               hub_port_state[hub_number-1][hub_port-1]=HUB_DEVICE_WAIT_RST_2;
            }
         }
         break;

      // Wait Second USB reset successfully generated then:
      // -> Enumerate the new device
      case HUB_DEVICE_WAIT_RST_2:
         if(port_ev==HUB_PORT_RESET)
         {
            c = 0;
            while (c<250)               // wait 250ms after USB reset
            {
               if (Is_usb_event(EVT_HOST_SOF)) { Usb_ack_event(EVT_HOST_SOF); c++; }// Count Start Of frame
               if (Is_host_emergency_exit() || Is_usb_bconnection_error_interrupt()) 
               {hub_port_state[hub_number-1][hub_port-1]=HUB_DEVICE_POWERED; return;}
            }
            // Select this new device
            Host_select_device(device_index);

            // Give an absolute device address
            host_set_address(DEVICE_BASE_ADDRESS+device_index);
            usb_tree.device[+device_index].device_address=DEVICE_BASE_ADDRESS+device_index;

            // Select this new address
            Host_select_device(device_index); 

            // Get the device descriptor 
            if (CONTROL_GOOD == host_get_device_descriptor())
            {
               // Check if the device connected belongs to the supported devices table
               if (HOST_TRUE == host_check_VID_PID())
               {
                  Host_set_device_supported();
                  Host_device_supported_action();
               }
               else
               {
               #if (HOST_STRICT_VID_PID_TABLE==ENABLE)
                  Host_device_not_supported_action();
                  return
               #endif
               }
               // Get Confiuration descriptor
               if (CONTROL_GOOD == host_get_configuration_descriptor())
               {
                  if (HOST_FALSE != host_check_class()) // Class support OK?
                  {
                  #if (HOST_AUTO_CFG_ENDPOINT==ENABLE)
                     if (host_auto_configure_endpoint())
                  #else
                     if(User_configure_endpoint()) // User call here instead of autoconfig
                  #endif
                     {
                        if (CONTROL_GOOD == host_set_configuration(1))  // Send Set_configuration
                        {
                           hub_port_state[hub_number-1][hub_port-1] = HUB_DEVICE_READY;
                            // Check if the connected device is a HUB
                           if(Get_class(0)==HUB_CLASS )
                           {
                                 // Get hub descriptor
                                 if( Get_hub_descriptor()==CONTROL_GOOD)
                                 {
                                    // Power each port of the hub
                                    nb_port=data_stage[NB_PORT_OFFSET];
                                    for(c=1;c<=nb_port;c++)
                                    {
                                       Set_port_feature(PORT_POWER,c);
                                    }
                                    nb_hub_present++;
                                    hub_init(nb_hub_present-1);
                                    hub_device_address[nb_hub_present-1]=usb_tree.device[device_index].device_address;
                                 }
                           }
                           else // New device is not a hub -> Signals device connection
                           {
                              new_device_connected=TRUE;
                           }
                        }
                     }
                  }
               }
            }
         }
         break;
         
      case HUB_DEVICE_READY:
         break;
      default:
         break;
   }
}


#endif
