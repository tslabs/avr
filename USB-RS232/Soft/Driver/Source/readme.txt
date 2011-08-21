;***************************************************************************
;* U S B   d r i v e r   f o r   A V R   F A M I L Y   U S B  s t a c k
;* 
;* File Name            :"AVR309.sys"
;* Title                :AVR309:USB driver for "USB to UART protocol converter"
;* Date                 :22.11.2003
;* Version              :1.0
;* Target Platform      :Wndows98, Windows2000, Windows XP
;* AUTHOR		:Ing. Igor Cesko
;* 			 Slovakia
;* 			 cesko@internet.sk
;* 			 http://www.cesko.host.sk
;*
;* DESCRIPTION:
;*  USB driver for device "USB stack in AVR microcontrollers" (USB protocol
;*  implementation into MCU with noUSB interface)
;*  Device:
;*  Universal USB interface (3x8-bit I/O port + RS232 serial line + EEPROM)
;*  + added RS232 FIFO buffer
;*
;* DRIVER CUSTOMIZATION AND MODIFICATION:
;*  This driver is modified example "isousb.sys" from
;*  Microsoft Windows 2000 DDK (Driver Development Kit):
;*  original "isousb.sys" is located in DDK sample directory:
;*  "\NTDDK\src\wdm\usb\isousb"
;*
;*  Modified part of "isousb" files is described in document:
;*   "\PDF doc\Differences to isousb DDK.pdf"
;*
;* To customize device driver to your purpose:
;*  - change driver link name "AVR309USB_0" in files "isopnp.c" and "isousb.c"
;*    to your name
;*  - set target SYS filename "TARGETNAME=" to your name in file "sources"
;*  - modify driver information in file "AVR309.rc"
;*  - modify registry path "ISOUSB_REGISTRY_PARAMETERS_PATH" in file "iusbdbg.h"
;*
;***************************************************************************