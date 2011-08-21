/*++

Copyright (c) 1997-1998  Microsoft Corporation

Module Name:

   ioctliso.c

Abstract:

    USB device driver for Intel 82930 USB test board.
    IOCTL handlers

Environment:

    kernel mode only

Notes:

  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
  PURPOSE.

  Copyright (c) 1997-1998 Microsoft Corporation.  All Rights Reserved.


Revision History:

    11/17/97 : created

--*/


#include "wdm.h"
#include "stdarg.h"
#include "stdio.h"


#include "usbdi.h"
#include "usbdlib.h"
#include "Iso82930.h"

#include "IsoUsb.h"
#include "usbdlib.h"



ULONG
VendorCommand(
	IN PDEVICE_OBJECT 	DeviceObject,
        IN UCHAR 		Request,
	IN USHORT 		Value,
	IN USHORT 		Index,
	PVOID			ioBuffer,
	IN ULONG		ioBufferLength
        )
{

	NTSTATUS ntStatus;
	PURB urb;

	if (ioBufferLength>255) ioBufferLength=255;  //hardware support max 255 block size length
	
	urb = (PURB) ExAllocatePool(NonPagedPool, sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST));
        if (urb) 
        {
	 UsbBuildVendorRequest(urb, 
		 	      URB_FUNCTION_VENDOR_ENDPOINT,
			      sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),
		   	      (USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK),  
			      0, 	 // Reserved Bits 		
			      Request,	 // Request 	
			      Value, 	 // Value 	
			      Index, 	 // Index	
			      ioBuffer,	 // Transfer Buffer
			      NULL,	 // TransferBufferMDL OPTIONAL
			      ioBufferLength,	 // Transfer Buffer Lenght	
			      NULL);	 // Link OPTIONAL
        	
	 ntStatus = IsoUsb_CallUSBD(DeviceObject, urb);

	
	 if (NT_SUCCESS(ntStatus))
	  {
	   ULONG ans = urb->UrbControlVendorClassRequest.TransferBufferLength;
           ExFreePool(urb);
	   return(ans);
	  }
	 else
	  {
           ExFreePool(urb);
	   return(0);
	  }
        }
  	else
  	{
  		KdPrint(("Error to alocate\n"));
  		 return(0); 
  	}

}



NTSTATUS
IsoUsb_ProcessIOCTL(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Routine Description:

    Dispatch table handler for IRP_MJ_DEVICE_CONTROL; 
    Handle DeviceIoControl() calls  from User mode


Arguments:

    DeviceObject - pointer to the FDO for this instance of the 82930 device.


Return Value:

    NT status code

--*/
{
   struct {
		  char fncnumber;
		  char param1;
		  char param2;
	  }*IgorbufferOut;
   struct {
		  UCHAR fncnumber;
		  UCHAR param1Lo;
		  UCHAR param1Hi;
		  UCHAR param2Lo;
		  UCHAR param2Hi;		  
	  }*IgorbufferOutLong;
	  
    PIO_STACK_LOCATION irpStack;
    PVOID ioBuffer;
    ULONG inputBufferLength;
    ULONG outputBufferLength;
    PDEVICE_EXTENSION deviceExtension;
    ULONG ioControlCode;
    NTSTATUS ntStatus;
    ULONG length;
    PUCHAR pch;
    PUSB_CONFIGURATION_DESCRIPTOR configurationDescriptor;
    unsigned short param1;
    unsigned short param2;

    ISOUSB_KdPrint( DBGLVL_DEFAULT,("IRP_MJ_DEVICE_CONTROL\n"));

    IsoUsb_IncrementIoCount(DeviceObject);

    //
    // Get a pointer to the current location in the Irp. This is where
    //     the function codes and parameters are located.
    //

    deviceExtension = DeviceObject->DeviceExtension;
    

    // Can't accept a new io request if:
    //  1) device is removed, 
    //  2) has never been started, 
    //  3) is stopped,
    //  4) has a remove request pending,
    //  5) has a stop device pending
    if ( !IsoUsb_CanAcceptIoRequests( DeviceObject ) ) {
        ntStatus = STATUS_DELETE_PENDING;
        Irp->IoStatus.Status = ntStatus;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );

        IsoUsb_DecrementIoCount(DeviceObject);                          
        return ntStatus;
    }

    irpStack = IoGetCurrentIrpStackLocation (Irp);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    // get pointers and lengths of the caller's (user's) IO buffer
    ioBuffer           = Irp->AssociatedIrp.SystemBuffer;
    inputBufferLength  = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
    KdPrint(("inputBufferLength = %d , outputBufferLength = %d \n",inputBufferLength,outputBufferLength));

    ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;

    //
    // Handle Ioctls from User mode
    //
    
    IgorbufferOut = ioBuffer;
    IgorbufferOutLong = ioBuffer;

    switch (ioControlCode) {
    case 8:
            if (inputBufferLength<3)
              {
                ntStatus = Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
                break;
              }
	    KdPrint(("Function8 number %d call with param1= %d and param2= %d\n",IgorbufferOut->fncnumber,IgorbufferOut->param1,IgorbufferOut->param2));
	    length = VendorCommand(DeviceObject,
	    			   IgorbufferOut->fncnumber,
	    			   IgorbufferOut->param1,
	    			   IgorbufferOut->param2,
	    			   ioBuffer,
	    			   outputBufferLength);
            Irp->IoStatus.Information = length;
            Irp->IoStatus.Status = STATUS_SUCCESS;
            ntStatus = STATUS_SUCCESS;
            KdPrint(("Length= %d\n",length));
	break;	
    case 0x800+8:
            if (inputBufferLength<5)
              {
                ntStatus = Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
                break;
              }    
            param1= IgorbufferOutLong->param1Hi;
            param1<<=8;
            param1|=IgorbufferOutLong->param1Lo;
            param2= IgorbufferOutLong->param2Hi;
            param2<<=8;
            param2|=IgorbufferOutLong->param2Lo;
            
	    KdPrint(("FunctionLong number %d call with param1= %d and param2= %d\n",IgorbufferOutLong->fncnumber,param1,param2));
	    length = VendorCommand(DeviceObject,
	    			   IgorbufferOutLong->fncnumber,
	    			   param1,
	    			   param2,
	    			   ioBuffer,
	    			   outputBufferLength);
            Irp->IoStatus.Information = length;
            Irp->IoStatus.Status = STATUS_SUCCESS;
            ntStatus = STATUS_SUCCESS;
            KdPrint(("Length= %d\n",length));
	break;		
    default:
        ntStatus =
            Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
    }

    IoCompleteRequest (Irp,
                       IO_NO_INCREMENT
                       );

    IsoUsb_DecrementIoCount(DeviceObject);                       

    return ntStatus;

}




NTSTATUS
IsoUsb_ResetDevice(
    IN PDEVICE_OBJECT DeviceObject
    )
/*++

Routine Description:
	Checks port status; if OK, return success and  do no more;
	If bad, attempt reset

Arguments:

    DeviceObject - pointer to the device object for this instance of the 82930
                    device.


Return Value:

    NT status code

--*/
{
    NTSTATUS ntStatus;
    ULONG portStatus;

    ISOUSB_KdPrint(DBGLVL_MEDIUM,("Enter IsoUsb_ResetDevice()\n"));
    
    //
    // Check the port state, if it is disabled we will need 
    // to re-enable it
    //
    ntStatus = IsoUsb_GetPortStatus(DeviceObject, &portStatus);

    if (NT_SUCCESS(ntStatus) && !(portStatus & USBD_PORT_ENABLED) &&
        portStatus & USBD_PORT_CONNECTED) {
        //
        // port is disabled, attempt reset
        //
		ISOUSB_KdPrint( DBGLVL_DEFAULT,("IsoUsb_ResetDevice() will reset\n"));
        ntStatus = IsoUsb_ResetParentPort(DeviceObject);
    }
	return ntStatus;
}



NTSTATUS
IsoUsb_GetPortStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PULONG PortStatus
    )
/*++

Routine Description:

    returns the port status for our device

Arguments:

Return Value:

    STATUS_SUCCESS if successful,
    STATUS_UNSUCCESSFUL otherwise

--*/
{
    NTSTATUS ntStatus, status = STATUS_SUCCESS;
    PIRP irp;
    KEVENT event;
    IO_STATUS_BLOCK ioStatus;
    PIO_STACK_LOCATION nextStack;
    PDEVICE_EXTENSION deviceExtension;

    ISOUSB_KdPrint( DBGLVL_DEFAULT,("enter IsoUsb_GetPortStatus\n"));

    deviceExtension = DeviceObject->DeviceExtension;

    *PortStatus = 0;

    //
    // issue a synchronous request
    //

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    // IoBuildDeviceIoControlRequest allocates and sets up an IRP for a device control request
    irp = IoBuildDeviceIoControlRequest(
                IOCTL_INTERNAL_USB_GET_PORT_STATUS,
                deviceExtension->TopOfStackDeviceObject, //next-lower driver's device object, representing the target device.
                NULL, // no input or output buffers
                0,
                NULL,
                0,
                TRUE, // internal ( use IRP_MJ_INTERNAL_DEVICE_CONTROL )
                &event, // event to be signalled on completion ( we wait for it below )
                &ioStatus);

    //
    // Call the class driver to perform the operation.  If the returned status
    // is PENDING, wait for the request to complete.
    //

    // IoGetNextIrpStackLocation gives a higher level driver access to the next-lower 
    // driver's I/O stack location in an IRP so the caller can set it up for the lower driver.
    nextStack = IoGetNextIrpStackLocation(irp);
    ISOUSB_ASSERT(nextStack != NULL);

    nextStack->Parameters.Others.Argument1 = PortStatus;

    ISOUSB_KdPrint( DBGLVL_DEFAULT,("IsoUsb_GetPortStatus() calling USBD port status api\n"));

    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject,
                            irp);

    ISOUSB_KdPrint( DBGLVL_DEFAULT,("IsoUsb_GetPortStatus() return from IoCallDriver USBD %x\n", ntStatus));

    if (ntStatus == STATUS_PENDING) {

        ISOUSB_KdPrint( DBGLVL_DEFAULT,("Wait for single object\n"));

        status = KeWaitForSingleObject(
                       &event,
                       Suspended,
                       KernelMode,
                       FALSE,
                       NULL);

        ISOUSB_KdPrint( DBGLVL_DEFAULT,("IsoUsb_GetPortStatus() Wait for single object, returned %x\n", status));
        
    } else {
        ioStatus.Status = ntStatus;
    }

    ISOUSB_KdPrint( DBGLVL_DEFAULT,("IsoUsb_GetPortStatus() Port status = %x\n", *PortStatus));

    //
    // USBD maps the error code for us
    //
    ntStatus = ioStatus.Status;

    ISOUSB_KdPrint( DBGLVL_DEFAULT,("Exit IsoUsb_GetPortStatus (%x)\n", ntStatus));

    return ntStatus;
}


NTSTATUS
IsoUsb_ResetParentPort(
    IN IN PDEVICE_OBJECT DeviceObject
    )
/*++

Routine Description:

    Reset the our parent port

Arguments:

Return Value:

    STATUS_SUCCESS if successful,
    STATUS_UNSUCCESSFUL otherwise

--*/
{
    NTSTATUS ntStatus, status = STATUS_SUCCESS;
    PIRP irp;
    KEVENT event;
    IO_STATUS_BLOCK ioStatus;
    PIO_STACK_LOCATION nextStack;
    PDEVICE_EXTENSION deviceExtension;

    ISOUSB_KdPrint( DBGLVL_HIGH,("enter IsoUsb_ResetParentPort\n"));

    deviceExtension = DeviceObject->DeviceExtension;

    //
    // issue a synchronous request
    //

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(
                IOCTL_INTERNAL_USB_RESET_PORT,
                deviceExtension->TopOfStackDeviceObject,
                NULL,
                0,
                NULL,
                0,
                TRUE, // internal ( use IRP_MJ_INTERNAL_DEVICE_CONTROL )
                &event,
                &ioStatus);

    //
    // Call the class driver to perform the operation.  If the returned status
    // is PENDING, wait for the request to complete.
    //

    nextStack = IoGetNextIrpStackLocation(irp);
    ISOUSB_ASSERT(nextStack != NULL);

    ISOUSB_KdPrint( DBGLVL_HIGH,("IsoUsb_ResetParentPort() calling USBD enable port api\n"));

    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject,
                            irp);
                            
    ISOUSB_KdPrint( DBGLVL_HIGH,("IsoUsb_ResetParentPort() return from IoCallDriver USBD %x\n", ntStatus));

    if (ntStatus == STATUS_PENDING) {

        ISOUSB_KdPrint( DBGLVL_HIGH,("IsoUsb_ResetParentPort() Wait for single object\n"));

        status = KeWaitForSingleObject(
                       &event,
                       Suspended,
                       KernelMode,
                       FALSE,
                       NULL);

        ISOUSB_KdPrint( DBGLVL_HIGH,("IsoUsb_ResetParentPort() Wait for single object, returned %x\n", status));
        
    } else {
        ioStatus.Status = ntStatus;
    }

    //
    // USBD maps the error code for us
    //
    ntStatus = ioStatus.Status;

    ISOUSB_KdPrint( DBGLVL_HIGH,("Exit IsoUsb_ResetPort (%x)\n", ntStatus));

    return ntStatus;
}





