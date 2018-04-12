/** @file

Copyright (c) 2006 - 2011, Byosoft Corporation.<BR> 
All rights reserved.This software and associated documentation (if any)
is furnished under a license and may only be used or copied in 
accordance with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be reproduced, 
stored in a retrieval system, or transmitted in any form or by any 
means without the express written consent of Byosoft Corporation.

File Name:
  SmmUsbDispatch2OnSmmUsbDispatchThunk.c

Abstract: 
  SMM UsbDispatch2 Protocol on SMM UsbDispatch Protocol Thunk driver.

Revision History:
Bug 1920: Create some SMM thunk drivers to support framework style silicon drivers.
TIME: 2011-05-13
$AUTHOR:  Cassie Liu
$REVIEWERS:  Hawker Chen
$SCOPE: Invoke framework implement on EDKII platform.
$TECHNICAL: Produce PI style protocol consuming framework style protocol.
$END----------------------------------------------------------------------------

**/

#include <PiDxe.h>
#include <FrameworkSmm.h>

#include <Protocol/SmmUsbDispatch2.h>
#include <Protocol/SmmUsbDispatch.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/DebugLib.h>

typedef struct {
  LIST_ENTRY                     Link;
  EFI_HANDLE                     DispatchHandle;
  UINTN                          UsbSmiInputValue;
  UINTN                          DispatchFunction;
} EFI_SMM_USB_DISPATCH2_THUNK_CONTEXT;

/**
  Register a child SMI source dispatch function for the specified USB SMI.

  This service registers a function (DispatchFunction) which will be called when the USB 
  SMI source specified is detected. On return, 
  DispatchHandle contains a unique handle which may be used later to unregister the function 
  using UnRegister().

  @param[in]  This                  Pointer to the EFI_SMM_USB_DISPATCH2_PROTOCOL instance.
  @param[in]  DispatchFunction      Function to register for handler when the specified USB 
                                    SMI is generated. 
  @param[in, out]  RegisterContext  Pointer to the dispatch function's context.
                                    The caller fills this context in before calling
                                    the register function to indicate to the register
                                    function which type and device of USB SMI the
                                    dispatch function should be invoked for.
  @param[out] DispatchHandle        Handle generated by the dispatcher to track the
                                    function instance.

  @retval EFI_SUCCESS               The dispatch function has been successfully
                                    registered and the SMI source has been enabled.
  @retval EFI_DEVICE_ERROR          The USB SMI driver was unable to enable the SMI source.
  @retval EFI_INVALID_PARAMETER     RegisterContext is invalid. The USB SMI type 
                                    is not within valid range.
  @retval EFI_OUT_OF_RESOURCES      There is not enough memory (system or SMM) to manage this
                                    child.
  @retval EFI_OUT_OF_RESOURCES      USB SMI could not be assigned for this dispatch.
**/
EFI_STATUS
EFIAPI
SmmUsbDispatch2Register (
  IN  CONST EFI_SMM_USB_DISPATCH2_PROTOCOL  *This,
  IN        EFI_SMM_HANDLER_ENTRY_POINT2    DispatchFunction,
  IN  CONST EFI_SMM_USB_REGISTER_CONTEXT    *RegisterContext,
  OUT       EFI_HANDLE                      *DispatchHandle
  );

/**
  Unregister a child SMI source dispatch function for the specified USB SMI.

  This service removes the handler associated with DispatchHandle so that it will no longer be 
  called in response to a USB SMI.

  @param[in] This                Pointer to the EFI_SMM_USB_DISPATCH2_PROTOCOL instance.
  @param[in] DispatchHandle      Handle of dispatch function to deregister.

  @retval EFI_SUCCESS            The dispatch function has been successfully unregistered.
  @retval EFI_INVALID_PARAMETER  The DispatchHandle was not valid.
**/
EFI_STATUS
EFIAPI
SmmUsbDispatch2UnRegister (
  IN CONST EFI_SMM_USB_DISPATCH2_PROTOCOL  *This,
  IN       EFI_HANDLE                      DispatchHandle
  );

EFI_SMM_USB_DISPATCH2_PROTOCOL gSmmUsbDispatch2 = {
  SmmUsbDispatch2Register,
  SmmUsbDispatch2UnRegister
};

EFI_SMM_USB_DISPATCH_PROTOCOL  *mSmmUsbDispatch;

/**
  Register a child SMI source dispatch function for the specified USB SMI.

  This service registers a function (DispatchFunction) which will be called when the USB 
  SMI source specified is detected. On return, 
  DispatchHandle contains a unique handle which may be used later to unregister the function 
  using UnRegister().

  @param[in]  This                  Pointer to the EFI_SMM_USB_DISPATCH2_PROTOCOL instance.
  @param[in]  DispatchFunction      Function to register for handler when the specified USB 
                                    SMI is generated. 
  @param[in, out]  RegisterContext  Pointer to the dispatch function's context.
                                    The caller fills this context in before calling
                                    the register function to indicate to the register
                                    function which type and device of USB SMI the
                                    dispatch function should be invoked for.
  @param[out] DispatchHandle        Handle generated by the dispatcher to track the
                                    function instance.

  @retval EFI_SUCCESS               The dispatch function has been successfully
                                    registered and the SMI source has been enabled.
  @retval EFI_DEVICE_ERROR          The USB SMI driver was unable to enable the SMI source.
  @retval EFI_INVALID_PARAMETER     RegisterContext is invalid. The USB SMI type 
                                    is not within valid range.
  @retval EFI_OUT_OF_RESOURCES      There is not enough memory (system or SMM) to manage this
                                    child.
  @retval EFI_OUT_OF_RESOURCES      USB SMI could not be assigned for this dispatch.
**/
EFI_STATUS
EFIAPI
SmmUsbDispatch2Register (
  IN  CONST EFI_SMM_USB_DISPATCH2_PROTOCOL  *This,
  IN        EFI_SMM_HANDLER_ENTRY_POINT2    DispatchFunction,
  IN  CONST EFI_SMM_USB_REGISTER_CONTEXT    *RegisterContext,
  OUT       EFI_HANDLE                      *DispatchHandle
  )
{
  EFI_STATUS                            Status;

  Status = mSmmUsbDispatch->Register (
                              mSmmUsbDispatch,
                              (EFI_SMM_USB_DISPATCH) DispatchFunction,
                              (EFI_SMM_USB_DISPATCH_CONTEXT *) RegisterContext,
                              DispatchHandle
                              );
  return Status;
}

/**
  Unregister a child SMI source dispatch function for the specified USB SMI.

  This service removes the handler associated with DispatchHandle so that it will no longer be 
  called in response to a USB SMI.

  @param[in] This                Pointer to the EFI_SMM_USB_DISPATCH2_PROTOCOL instance.
  @param[in] DispatchHandle      Handle of dispatch function to deregister.

  @retval EFI_SUCCESS            The dispatch function has been successfully unregistered.
  @retval EFI_INVALID_PARAMETER  The DispatchHandle was not valid.
**/
EFI_STATUS
EFIAPI
SmmUsbDispatch2UnRegister (
  IN CONST EFI_SMM_USB_DISPATCH2_PROTOCOL  *This,
  IN       EFI_HANDLE                      DispatchHandle
  )
{
  EFI_STATUS                            Status;

  Status = mSmmUsbDispatch->UnRegister (mSmmUsbDispatch, DispatchHandle);
  
  return Status;
}

/**
  Entry Point for this thunk driver.

  @param[in] ImageHandle  Image handle of this driver.
  @param[in] SystemTable  A Pointer to the EFI System Table.

  @retval EFI_SUCCESS     The entry point is executed successfully.
  @retval other           Some error occurred when executing this entry point.
**/
EFI_STATUS
EFIAPI
SmmUsbDispatch2ThunkMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS               Status;

  ///
  /// Locate Framework SMM UsbDispatch Protocol
  ///
  Status = gBS->LocateProtocol (&gEfiSmmUsbDispatchProtocolGuid, NULL, (VOID **)&mSmmUsbDispatch);
  ASSERT_EFI_ERROR (Status);


  ///
  /// Publish PI SMM UsbDispatch2 Protocol
  ///
  ImageHandle = NULL;
  Status = gSmst->SmmInstallProtocolInterface (
                    &ImageHandle,
                    &gEfiSmmUsbDispatch2ProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    &gSmmUsbDispatch2
                    );
  ASSERT_EFI_ERROR (Status);
  return Status;
}
