/** @file
Entry and initialization module for the browser.

Copyright (c) 2007 - 2012, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Setup.h"

SETUP_DRIVER_PRIVATE_DATA  mPrivateData = {
  SETUP_DRIVER_SIGNATURE,
  NULL,
  {
    SendForm,
    BrowserCallback
  },
  {
    SetScope,
    RegisterHotKey,
    RegiserExitHandler,
    IsBrowserDataModified,
    ExecuteAction,
    FakeOemPlatformReset,
  }
};

EFI_HII_DATABASE_PROTOCOL         *mHiiDatabase;
EFI_HII_STRING_PROTOCOL           *mHiiString;
EFI_HII_CONFIG_ROUTING_PROTOCOL   *mHiiConfigRouting;

UINTN           gBrowserContextCount = 0;
LIST_ENTRY      gBrowserContextList = INITIALIZE_LIST_HEAD_VARIABLE (gBrowserContextList);
LIST_ENTRY      gBrowserFormSetList = INITIALIZE_LIST_HEAD_VARIABLE (gBrowserFormSetList);
LIST_ENTRY      gBrowserHotKeyList  = INITIALIZE_LIST_HEAD_VARIABLE (gBrowserHotKeyList);
LIST_ENTRY      *gCurrentFormSetLink = NULL;
LIST_ENTRY      *gByoFormSetList = NULL;
EFI_BYO_FORMSET_MANAGER_PROTOCOL    *gByoFormsetManager = NULL;
LIST_ENTRY      gModifiedStorgeList  = INITIALIZE_LIST_HEAD_VARIABLE (gModifiedStorgeList);

BANNER_DATA           *gBannerData;
EFI_HII_HANDLE        gFrontPageHandle;
UINTN                 gClassOfVfr;
UINTN                 gFunctionKeySetting;
BOOLEAN               gResetRequired;
BOOLEAN               gExitRequired;
EFI_HII_HANDLE        gHiiHandle;
UINT16                gDirection;
CHAR16                gDirectInputNum;
EFI_SCREEN_DESCRIPTOR gScreenDimensions;
BROWSER_SETTING_SCOPE gBrowserSettingScope = SystemLevel;
BOOLEAN               mBrowserScopeFirstSet = TRUE;
EXIT_HANDLER          ExitHandlerFunction = NULL;
UINTN                 gFooterHeight;
BOOLEAN              bDrawSetupBackground = TRUE;
UINT16                 gFirstFormId = 1;
BOOLEAN              bByoFormsetFlag = FALSE;
BOOLEAN              bShowHelpMessage = TRUE;
//
// Browser Global Strings
//
CHAR16            *gSaveFailed;
CHAR16            *gDiscardFailed;
CHAR16            *gDefaultFailed;
CHAR16            *gEnterString;
CHAR16            *gEnterCommitString;
CHAR16            *gEnterEscapeString;
CHAR16            *gEscapeString;
CHAR16            *gMoveHighlight;
CHAR16            *gMakeSelection;
CHAR16            *gDecNumericInput;
CHAR16            *gHexNumericInput;
CHAR16            *gToggleCheckBox;
CHAR16            *gPromptForData;
CHAR16            *gPromptForPassword;
CHAR16            *gPromptForNewPassword;
CHAR16            *gConfirmPassword;
CHAR16            *gConfirmError;
CHAR16            *gPassowordInvalid;
CHAR16            *gPressEnter;
CHAR16            *gEmptyString;
CHAR16            *gAreYouSure;
CHAR16            *gYesResponse;
CHAR16            *gNoResponse;
CHAR16            *gMiniString;
CHAR16            *gPlusString;
CHAR16            *gMinusString;
CHAR16            *gAdjustNumber;

CHAR16            *gFunctionOne;
CHAR16            *gSelectItem;
CHAR16            *gMinusPlus;
CHAR16            *gChangeValue;
CHAR16            *gFunctionNine;
CHAR16            *gSetupDefault;
CHAR16            *gEsc;
CHAR16            *gEscString;
CHAR16            *gSelectMenu;
CHAR16            *gEnter;
CHAR16            *gFunctionTen;
CHAR16            *gSaveandExit;
CHAR16            *gHelpString;
CHAR16            *gAreYouSureExit;
CHAR16            *gAreYouSureExitWithSaving;
CHAR16            *gAreYouSureSaveChanges;
CHAR16            *gAreYouSureLoadUserDefault;
CHAR16            *gAreYouSureExitWithOutSaving;
CHAR16            *gAreYouSureDiscardChanges;
CHAR16            *gAreYouSureLoadDefault;
CHAR16            *gAreYouSureSaveUserDefault;
CHAR16            *gPoweronPassword;
CHAR16            *gSetupNotice;
CHAR16            *gConfirmSuccess;
CHAR16            *gGeneralHelp;
CHAR16            *gGeneralHelp1;
CHAR16            *gGeneralHelp2;
CHAR16            *gGeneralHelp3;
CHAR16            *gTab;
CHAR16            *gBootMenuPrompt;
CHAR16            *gBootMenuHelp1;
CHAR16            *gBootMenuHelp2;
CHAR16            *gBootMenuHelp3;
CHAR16            *gSaveChanges;
CHAR16            *gOptionMismatch;
CHAR16            *gFormSuppress;

CHAR16            *mUnknownString = L"!";

CHAR16            gPromptBlockWidth;
CHAR16            gOptionBlockWidth;
CHAR16            gHelpBlockWidth;
CHAR16            gMiddleVerticalLineColumn;
CHAR16            gOptionStringWidth;

CHAR16            *gByoSoftWare;
CHAR16            *gFunctionOneString;
CHAR16            *gFunctionTwoString;
CHAR16            *gFunctionNineString;
CHAR16            *gFunctionTenString;
CHAR16            *gSaveFailed;
CHAR16            *gResultSuccess;
CHAR16            *gResultFailed;


EFI_GUID  gZeroGuid = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
EFI_GUID  gSetupBrowserGuid = {
  0xab368524, 0xb60c, 0x495b, {0xa0, 0x9, 0x12, 0xe8, 0x5b, 0x1a, 0xea, 0x32}
};

FORM_BROWSER_FORMSET  *gOldFormSet = NULL;

BOOLEAN    mSynchronizeWithOldFormset = TRUE;
BOOLEAN    gBeLoadDefaultValue = FALSE;
FUNCTIION_KEY_SETTING gFunctionKeySettingTable[] = {
  //
  // Boot Manager
  //
  {
    {
      0x847bc3fe,
      0xb974,
      0x446d,
      {
        0x94,
        0x49,
        0x5a,
        0xd5,
        0x41,
        0x2e,
        0x99,
        0x3b
      }
    },
    NONE_FUNCTION_KEY_SETTING
  },
  //
  // Device Manager
  //
  {
    {
      0x3ebfa8e6,
      0x511d,
      0x4b5b,
      {
        0xa9,
        0x5f,
        0xfb,
        0x38,
        0x26,
        0xf,
        0x1c,
        0x27
      }
    },
    NONE_FUNCTION_KEY_SETTING
  },
  //
  // BMM FormSet.
  //
  {
    {
      0x642237c7,
      0x35d4,
      0x472d,
      {
        0x83,
        0x65,
        0x12,
        0xe0,
        0xcc,
        0xf2,
        0x7a,
        0x22
      }
    },
    NONE_FUNCTION_KEY_SETTING
  },
  //
  // BMM File Explorer FormSet.
  //
  {
    {
      0x1f2d63e1,
      0xfebd,
      0x4dc7,
      {
        0x9c,
        0xc5,
        0xba,
        0x2b,
        0x1c,
        0xef,
        0x9c,
        0x5b
      }
    },
    NONE_FUNCTION_KEY_SETTING
  },
};


/**
  Load all hii formset to the browser.

**/
VOID
LoadAllHiiFormset (
  VOID
  )
{
  FORM_BROWSER_FORMSET    *BackUpFormSet;
  FORM_BROWSER_FORMSET    *LocalFormSet;
  EFI_HII_HANDLE          *HiiHandles;
  UINTN                   Index;
  EFI_GUID                ZeroGuid;
  EFI_STATUS              Status;
  static BOOLEAN    RunFlag = FALSE;

  if (RunFlag) {
    return;
  }
  RunFlag = TRUE;
  //
  // Open all FormSet by locate HII packages.
  // Initiliaze the maintain FormSet to store default data as back up data.
  //
  BackUpFormSet    = gOldFormSet;
  gOldFormSet      = NULL;
  //
  // Get all the Hii handles
  //
  HiiHandles = HiiGetHiiHandles (NULL);
  ASSERT (HiiHandles != NULL);
  //
  // Search for formset of each class type
  //
  for (Index = 0; HiiHandles[Index] != NULL; Index++) {
    //
    // Check HiiHandles[Index] does exist in global maintain list.
    //
    if (GetFormSetFromHiiHandle (HiiHandles[Index]) != NULL) {
      continue;
    }
    //
    // Initilize FormSet Setting
    //
    LocalFormSet = AllocateZeroPool (sizeof (FORM_BROWSER_FORMSET));
    ASSERT (LocalFormSet != NULL);
    ZeroMem (&ZeroGuid, sizeof (ZeroGuid));
    Status = InitializeFormSet (HiiHandles[Index], &ZeroGuid, LocalFormSet, FALSE);
    if (EFI_ERROR (Status) || IsListEmpty (&LocalFormSet->FormListHead)) {
      DestroyFormSet (LocalFormSet);
      continue;
    }

    Status = InitializeCurrentSetting (LocalFormSet);
    if (EFI_ERROR (Status)) {
      DestroyFormSet (LocalFormSet);
      continue;
    }
    //
    // Initilize Questions' Value
    //
    Status = LoadFormSetConfig (NULL, LocalFormSet);
    if (EFI_ERROR (Status)) {
      DestroyFormSet (LocalFormSet);
      continue;
    }
    //
    // Add FormSet into the maintain list.
    //
    InsertTailList (&gBrowserFormSetList, &LocalFormSet->Link);
  }
  //
  // Free resources, and restore gOldFormSet.
  //
  FreePool (HiiHandles);
  gOldFormSet = BackUpFormSet;
}



/**
  Load all byo formset to the browser.

**/
VOID
PreLoadByoFormset (
  VOID
  )
{
  EFI_STATUS    Status;
  EFI_GUID    ZeroGuid;
  LIST_ENTRY    *Link;
  EFI_SCREEN_DESCRIPTOR    LocalScreen;
  BYO_BROWSER_FORMSET   *ByoFormSet;
  FORM_BROWSER_FORMSET    *LocalFormSet;

  if (IsListEmpty(gByoFormSetList)) {
    return;
  }

  //
  // Print help info when it load hii resouce.
  //
  ZeroMem (&LocalScreen, sizeof (EFI_SCREEN_DESCRIPTOR));
  gST->ConOut->QueryMode (gST->ConOut, gST->ConOut->Mode->Mode, &LocalScreen.RightColumn, &LocalScreen.BottomRow);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | FIELD_TEXT);
  PrintStringAt(0, LocalScreen.BottomRow - 1, L"Setup is loading, Please wait a second...");

  //
  // Load all formset in gByoFormSetList.
  //
  Link = GetFirstNode (gByoFormSetList);
  while (!IsNull (gByoFormSetList, Link)) {
    ByoFormSet = BYO_FORM_BROWSER_FORMSET_FROM_LINK (Link);
    Link = GetNextNode (gByoFormSetList, Link);

    if (GetFormSetFromHiiHandle (ByoFormSet->HiiHandle) != NULL) {
      continue;
    }
    //
    // Initilize FormSet Setting
    //
    LocalFormSet = AllocateZeroPool (sizeof (FORM_BROWSER_FORMSET));
    ASSERT (LocalFormSet != NULL);
    ZeroMem (&ZeroGuid, sizeof (ZeroGuid));
    Status = InitializeFormSet (ByoFormSet->HiiHandle, &ZeroGuid, LocalFormSet, FALSE);
    if (EFI_ERROR (Status) || IsListEmpty (&LocalFormSet->FormListHead)) {
      DestroyFormSet (LocalFormSet);
      continue;
    }

    Status = InitializeCurrentSetting (LocalFormSet);
    if (EFI_ERROR (Status)) {
      DestroyFormSet (LocalFormSet);
      continue;
    }
    //
    // Initilize Questions' Value
    //
    Status = LoadFormSetConfig (NULL, LocalFormSet);
    if (EFI_ERROR (Status)) {
      DestroyFormSet (LocalFormSet);
      continue;
    }
    //
    // Add FormSet into the maintain list.
    //
    InsertTailList (&gBrowserFormSetList, &LocalFormSet->Link);
  }
  
  //
  //Clean Gray background.
  //
  gST->ConOut->ClearScreen (gST->ConOut);
  GopBltArea (0, LocalScreen.RightColumn, 2, LocalScreen.BottomRow - 3, EFI_BACKGROUND_LIGHTGRAY);
  return;
}

/**
  Set global variable according Formset.

**/
VOID
RefreshFormsetVariable (
  FORM_BROWSER_FORMSET    *FormSet
  )
{
  UINTN    Index;
  LIST_ENTRY         *Link;
  FORM_BROWSER_FORM  *Form;

  //
  // Set VFR type by FormSet SubClass field,
  // But we only support FORMSET_CLASS_PLATFORM_SETUP of layout style.
  // 
  gClassOfVfr = FORMSET_CLASS_PLATFORM_SETUP;

  //
  // Set VFR type by FormSet class guid
  //
  for (Index = 0; Index < 3; Index ++) {
    if (CompareGuid (&FormSet->ClassGuid[Index], &gEfiHiiPlatformSetupFormsetGuid)) {
      gClassOfVfr |= FORMSET_CLASS_PLATFORM_SETUP;
      break;
    }
  }

  if ((gClassOfVfr & FORMSET_CLASS_FRONT_PAGE) == FORMSET_CLASS_FRONT_PAGE) {
    gFrontPageHandle = FormSet->HiiHandle;
  }

  //
  // Set gFirstFormId according Formset.
  //
  gFirstFormId = 1;
  Form = NULL;
  Link = GetFirstNode (&FormSet->FormListHead);
  Form = FORM_BROWSER_FORM_FROM_LINK (Link);
  if (NULL != Form) {
    gFirstFormId = Form->FormId;
  }

  return;
}
/**
  This is the routine which an external caller uses to direct the browser
  where to obtain it's information.


  @param This            The Form Browser protocol instanse.
  @param Handles         A pointer to an array of Handles.  If HandleCount > 1 we
                         display a list of the formsets for the handles specified.
  @param HandleCount     The number of Handles specified in Handle.
  @param FormSetGuid     This field points to the EFI_GUID which must match the Guid
                         field in the EFI_IFR_FORM_SET op-code for the specified
                         forms-based package. If FormSetGuid is NULL, then this
                         function will display the first found forms package.
  @param FormId          This field specifies which EFI_IFR_FORM to render as the first
                         displayable page. If this field has a value of 0x0000, then
                         the forms browser will render the specified forms in their encoded order.
  @param ScreenDimensions Points to recommended form dimensions, including any non-content area, in
                          characters.
  @param ActionRequest   Points to the action recommended by the form.

  @retval  EFI_SUCCESS            The function completed successfully.
  @retval  EFI_INVALID_PARAMETER  One of the parameters has an invalid value.
  @retval  EFI_NOT_FOUND          No valid forms could be found to display.

**/
EFI_STATUS
EFIAPI
SendForm (
  IN  CONST EFI_FORM_BROWSER2_PROTOCOL *This,
  IN  EFI_HII_HANDLE                   *Handles,
  IN  UINTN                            HandleCount,
  IN  EFI_GUID                         *FormSetGuid, OPTIONAL
  IN  UINT16                           FormId, OPTIONAL
  IN  CONST EFI_SCREEN_DESCRIPTOR      *ScreenDimensions, OPTIONAL
  OUT EFI_BROWSER_ACTION_REQUEST       *ActionRequest  OPTIONAL
  )
{
  EFI_STATUS                    Status;
  UI_MENU_SELECTION             *Selection;
  UINTN                         Index;
  FORM_BROWSER_FORMSET          *FormSet;
  LIST_ENTRY                    *Link;
  BYO_BROWSER_FORMSET   *ByoFormSet;
  FORM_BROWSER_FORMSET  *FormSetTmp = NULL;

  //
  // Calculate total number of Register HotKeys.
  //
Index = 0;
  Link  = GetFirstNode (&gBrowserHotKeyList);
  while (!IsNull (&gBrowserHotKeyList, Link)) {
    Link = GetNextNode (&gBrowserHotKeyList, Link);
    Index ++;
  }
  //
  // Show three HotKeys help information on one ROW.
  //
  gFooterHeight = FOOTER_HEIGHT + (Index / 3);
  //
  // Save globals used by SendForm()
  //
  SaveBrowserContext ();
  gResetRequired = FALSE;
  gExitRequired  = FALSE;
  Status = EFI_SUCCESS;
  ZeroMem (&gScreenDimensions, sizeof (EFI_SCREEN_DESCRIPTOR));
  //
  // Seed the dimensions in the global
  //
  gST->ConOut->QueryMode (
                 gST->ConOut,
                 gST->ConOut->Mode->Mode,
                 &gScreenDimensions.RightColumn,
                 &gScreenDimensions.BottomRow
                 );
  if (ScreenDimensions != NULL) {
    //
    // Check local dimension vs. global dimension.
    //
    if ((gScreenDimensions.RightColumn < ScreenDimensions->RightColumn) ||
        (gScreenDimensions.BottomRow < ScreenDimensions->BottomRow)
        ) {
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    } else {
      //
      // Local dimension validation.
      //
      if ((ScreenDimensions->RightColumn > ScreenDimensions->LeftColumn) &&
          (ScreenDimensions->BottomRow > ScreenDimensions->TopRow) &&
          ((ScreenDimensions->RightColumn - ScreenDimensions->LeftColumn) > 2) &&
          (
            (ScreenDimensions->BottomRow - ScreenDimensions->TopRow) > STATUS_BAR_HEIGHT +
            SCROLL_ARROW_HEIGHT *
            2 +
            FRONT_PAGE_HEADER_HEIGHT +
            gFooterHeight +
            1
          )
        ) {
        CopyMem (&gScreenDimensions, (VOID *) ScreenDimensions, sizeof (EFI_SCREEN_DESCRIPTOR));
      } else {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    }
  }
  gOptionBlockWidth = (CHAR16) ((gScreenDimensions.RightColumn - gScreenDimensions.LeftColumn) / 3);
  gPromptBlockWidth = (CHAR16) (gOptionBlockWidth + 4);
  gMiddleVerticalLineColumn = (CHAR16) ((gScreenDimensions.RightColumn - gScreenDimensions.LeftColumn)* 2 / 3) + 4;
  gHelpBlockWidth   = (CHAR16) (gScreenDimensions.RightColumn - gMiddleVerticalLineColumn);
  gOptionStringWidth = (UINT16) (gMiddleVerticalLineColumn - gPromptBlockWidth - ITEM_GAP_WIDTH - SCROLL_BAR_WIDTH);
  //
  // Initialize the strings for the browser, upon exit of the browser, the strings will be freed
  //
  InitializeBrowserStrings ();
  gFunctionKeySetting = ENABLE_FUNCTION_KEY_SETTING;
  //
  // Load all formst first to reduce time when formset be showed.
  //
  PreLoadByoFormset();
  //
  // Ensure we are in Text mode
  //
  gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_LIGHTGRAY, EFI_BLACK));

  for (Index = 0; Index < HandleCount; Index++) {
    Selection = AllocateZeroPool (sizeof (UI_MENU_SELECTION));
    ASSERT (Selection != NULL);

    Selection->Handle = Handles[Index];
    if (FormSetGuid != NULL) {
      CopyMem (&Selection->FormSetGuid, FormSetGuid, sizeof (EFI_GUID));
      Selection->FormId = FormId;
    } else {
      CopyMem (&Selection->FormSetGuid, &gEfiHiiPlatformSetupFormsetGuid, sizeof (EFI_GUID));
    }

    bDrawSetupBackground = TRUE;

    do {
      if (IsByoMainFormset(Selection->Handle)) {
        bByoFormsetFlag = TRUE;
      } else {
        bByoFormsetFlag = FALSE;
      }
      if ((NULL == GetFormSetFromHiiHandle (Selection->Handle)) ||
        (TRUE == mReloadFormset)) {
        //
        // New formset.
        //
        FormSet = AllocateZeroPool (sizeof (FORM_BROWSER_FORMSET));
        ASSERT (FormSet != NULL);
        //
        // Initialize internal data structures of FormSet
        //
        Status = InitializeFormSet (Selection->Handle, &Selection->FormSetGuid, FormSet, TRUE);
        if (EFI_ERROR (Status) || IsListEmpty (&FormSet->FormListHead)) {
          DestroyFormSet (FormSet);
          break;
        }
        //
        // Try to find pre FormSet in the maintain backup list.
        //
        gOldFormSet = GetFormSetFromHiiHandle (Selection->Handle);
        Selection->FormSet = FormSet;
      } else {
        Selection->FormSet = GetFormSetFromHiiHandle (Selection->Handle);
	    if (mSynchronizeWithOldFormset && Selection->FormSet) {
          SynchronizeWithOldFormset(Selection->FormSet);
        } else {
          mSynchronizeWithOldFormset = TRUE;
        }
        gOldFormSet = Selection->FormSet;
        RefreshFormsetVariable(Selection->FormSet);

    }
      gCurrentSelection = Selection;
      //
      // Get gCurrentFormSetLink From gByoFormSetList.
      //
      Link = GetFirstNode (gByoFormSetList);
      while (!IsNull (gByoFormSetList, Link)) {
        ByoFormSet = BYO_FORM_BROWSER_FORMSET_FROM_LINK (Link);
        if (CompareGuid(&ByoFormSet->Guid, &Selection->FormSetGuid)) {
          gCurrentFormSetLink = Link;
          break;
        }
        Link = GetNextNode (gByoFormSetList, Link);
      }

      Status = SetupBrowser (Selection);
      gCurrentSelection = NULL;
      if (EFI_ERROR (Status)) {
        //	  	
        //Nerver return by Error Status;	  	
        //
        DEBUG ((EFI_D_ERROR, "SendForm(), SetupBrowser() :%r.", Status)); 	
       }
    } while (Selection->Action == UI_ACTION_REFRESH_FORMSET);
    //
    // If no data is changed, Remove FormSet From the maintain list.
    //
    Link = GetFirstNode (gByoFormSetList);
    while (!IsNull (gByoFormSetList, Link)) {
      ByoFormSet = BYO_FORM_BROWSER_FORMSET_FROM_LINK (Link);
      FormSetTmp = NULL;
      FormSetTmp = GetFormSetFromHiiHandle (ByoFormSet->HiiHandle);
      if (NULL != FormSetTmp) {
        if (!IsNvUpdateRequired (FormSetTmp)) {
          RemoveEntryList (&FormSetTmp->Link);
          DestroyFormSet (FormSetTmp);
        }
      }
      Link = GetNextNode (gByoFormSetList, Link);
    }
    FreePool (Selection);
  }

  if (ActionRequest != NULL) {
    *ActionRequest = EFI_BROWSER_ACTION_REQUEST_NONE;
    if (gResetRequired) {
      *ActionRequest = EFI_BROWSER_ACTION_REQUEST_RESET;
    }
  }

  FreeBrowserStrings ();
  gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_LIGHTGRAY, EFI_BLACK));
  gST->ConOut->ClearScreen (gST->ConOut);

Done:
  //
  // Restore globals used by SendForm()
  //
  RestoreBrowserContext ();
  RemoveModifiedStorage ();
  gBeLoadDefaultValue = FALSE;

  return Status;
}

/**
  Get or set data to the storage.

  @param  ResultsDataSize        The size of the buffer associatedwith ResultsData.
  @param  ResultsData            A string returned from an IFR browser or
                                 equivalent. The results string will have no
                                 routing information in them.
  @param  RetrieveData           A BOOLEAN field which allows an agent to retrieve
                                 (if RetrieveData = TRUE) data from the uncommitted
                                 browser state information or set (if RetrieveData
                                 = FALSE) data in the uncommitted browser state
                                 information.
  @param  Storage                The pointer to the storage.
  @param  CurrentSize            The data size has saved in the ResultData.

  @retval EFI_SUCCESS            The results have been distributed or are awaiting
                                 distribution.

**/
EFI_STATUS
ProcessStorage (
  IN OUT UINTN                         ResultsDataSize,
  IN OUT EFI_STRING                    *ResultsData,
  IN BOOLEAN                           RetrieveData,
  IN FORMSET_STORAGE                   *Storage,
  IN OUT UINTN                         *CurrentSize
  )
{
  CHAR16                *ConfigResp;
  EFI_STATUS            Status;
  CHAR16                *StrPtr;
  UINTN                 BufferSize;
  UINTN                 TmpSize;

  if (RetrieveData) {

    //
    // Skip if there is no RequestElement
    //
    if (Storage->ElementCount == 0) {
      return EFI_SUCCESS;
    }

    //
    // Generate <ConfigResp>
    //
    Status = StorageToConfigResp (Storage, &ConfigResp, FALSE);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    //
    // Calculate the buffer size.
    //
    if (*CurrentSize == 0) {
      //
      // Skip <ConfigHdr> and '&' to point to <ConfigBody> when first copy the configbody.
      // Also need to consider add "\0" at first time.
      //
      StrPtr     = ConfigResp + StrLen (Storage->ConfigHdr) + 1;
      BufferSize = StrSize (StrPtr);
    } else {
      //
      // Skip <ConfigHdr>  to point to <ConfigBody> when append the configbody.
      //
      StrPtr = ConfigResp + StrLen (Storage->ConfigHdr);
      BufferSize = StrLen (StrPtr) * sizeof (*StrPtr);
    }

    //
    // Copy the data if the input buffer is bigger enough.
    //
    if (ResultsDataSize >= *CurrentSize + BufferSize) {
      if (*CurrentSize == 0) {
        StrCpy (*ResultsData, StrPtr);
      } else {
        StrCat (*ResultsData, StrPtr);
      }
    }

    *CurrentSize += BufferSize;

    FreePool (ConfigResp);
  } else {
    //
    // Prepare <ConfigResp>
    //
    TmpSize = StrLen (*ResultsData);
    BufferSize = (TmpSize + StrLen (Storage->ConfigHdr) + 2) * sizeof (CHAR16);
    ConfigResp = AllocateZeroPool (BufferSize);
    ASSERT (ConfigResp != NULL);

    StrCpy (ConfigResp, Storage->ConfigHdr);
    StrCat (ConfigResp, L"&");
    StrCat (ConfigResp, *ResultsData);

    //
    // Update Browser uncommited data
    //
    Status = ConfigRespToStorage (Storage, ConfigResp);
    FreePool (ConfigResp);
    if (EFI_ERROR (Status)) {
      return Status;
    }

  }
  return EFI_SUCCESS;
}

/**
  This routine called this service in the browser to retrieve or set certain uncommitted
  state information that resides in the open formsets.

  @param  This                   A pointer to the EFI_FORM_BROWSER2_PROTOCOL
                                 instance.
  @param  ResultsDataSize        A pointer to the size of the buffer associated
                                 with ResultsData.
  @param  ResultsData            A string returned from an IFR browser or
                                 equivalent. The results string will have no
                                 routing information in them.
  @param  RetrieveData           A BOOLEAN field which allows an agent to retrieve
                                 (if RetrieveData = TRUE) data from the uncommitted
                                 browser state information or set (if RetrieveData
                                 = FALSE) data in the uncommitted browser state
                                 information.
  @param  VariableGuid           An optional field to indicate the target variable
                                 GUID name to use.
  @param  VariableName           An optional field to indicate the target
                                 human-readable variable name.

  @retval EFI_SUCCESS            The results have been distributed or are awaiting
                                 distribution.
  @retval EFI_BUFFER_TOO_SMALL   The ResultsDataSize specified was too small to
                                 contain the results data.

**/
EFI_STATUS
EFIAPI
BrowserCallback (
  IN CONST EFI_FORM_BROWSER2_PROTOCOL  *This,
  IN OUT UINTN                         *ResultsDataSize,
  IN OUT EFI_STRING                    ResultsData,
  IN BOOLEAN                           RetrieveData,
  IN CONST EFI_GUID                    *VariableGuid, OPTIONAL
  IN CONST CHAR16                      *VariableName  OPTIONAL
  )
{
  EFI_STATUS            Status;
  LIST_ENTRY            *Link;
  LIST_ENTRY            *LinkFormset;
  FORMSET_STORAGE       *Storage;
  FORM_BROWSER_FORMSET  *FormSet;
  UINTN                 TotalSize;
  BOOLEAN               Found;

  if (ResultsDataSize == NULL || ResultsData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  TotalSize = 0;
  Storage   = NULL;
  Found     = FALSE;
  Status    = EFI_SUCCESS;

  //
  // If set browser data, pre load all hii formset to avoid set the varstore which is not
  // saved in browser.
  //
  if (!RetrieveData && (gBrowserSettingScope == SystemLevel) && bByoFormsetFlag) {
    LoadAllHiiFormset();
  }



  if (VariableGuid != NULL) {
    //
    // Search in the saved formset.
    //
    LinkFormset = GetFirstNode (&gBrowserFormSetList);
    while (!IsNull (&gBrowserFormSetList, LinkFormset)) {
      FormSet = FORM_BROWSER_FORMSET_FROM_LINK (LinkFormset);
      LinkFormset = GetNextNode (&gBrowserFormSetList, LinkFormset);

      //
      // Try to find target storage in the current formset.
      //
      Link = GetFirstNode (&FormSet->StorageListHead);
      while (!IsNull (&FormSet->StorageListHead, Link)) {
        Storage = FORMSET_STORAGE_FROM_LINK (Link);
        Link = GetNextNode (&FormSet->StorageListHead, Link);


        //
        // Check the current storage.
        //
        if (!CompareGuid (&Storage->Guid, (EFI_GUID *) VariableGuid)) {
          continue;
        }

        if (Storage->Type == EFI_HII_VARSTORE_BUFFER ||
            Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER) {
          //
          // Buffer storage require both GUID and Name
          //
          if (VariableName == NULL) {
            return EFI_NOT_FOUND;
          }

          if (StrCmp (Storage->Name, (CHAR16 *) VariableName) != 0) {
            continue;
          }
        }

        Status = ProcessStorage (*ResultsDataSize, &ResultsData, RetrieveData, Storage, &TotalSize);
        if (EFI_ERROR (Status)) {
          return Status;
        }

        //
        // Different formsets may have same varstore, so here just set the flag
        // not exit the circle.
        //
        Found = TRUE;
      }
    }
    if (!Found) {
      return EFI_NOT_FOUND;
    }
  } else {
    if (gCurrentSelection == NULL) {
      return EFI_NOT_READY;
    }
    //
    // GUID/Name is not specified, take the first storage in current FormSet
    //
    FormSet = gCurrentSelection->FormSet;
    Link = GetFirstNode (&FormSet->StorageListHead);
    if (IsNull (&FormSet->StorageListHead, Link)) {
      return EFI_UNSUPPORTED;
    }
    Storage = FORMSET_STORAGE_FROM_LINK (Link);

    Status = ProcessStorage (*ResultsDataSize, &ResultsData, RetrieveData, Storage, &TotalSize);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  if (RetrieveData) {
    Status = TotalSize <= *ResultsDataSize ? EFI_SUCCESS : EFI_BUFFER_TOO_SMALL;
    *ResultsDataSize = TotalSize;
  }
  return Status;
}

/**
  Notify function will remove the formset in the maintain list
  once this formset is removed.

  Functions which are registered to receive notification of
  database events have this prototype. The actual event is encoded
  in NotifyType. The following table describes how PackageType,
  PackageGuid, Handle, and Package are used for each of the
  notification types.

  @param PackageType  Package type of the notification.

  @param PackageGuid  If PackageType is
                      EFI_HII_PACKAGE_TYPE_GUID, then this is
                      the pointer to the GUID from the Guid
                      field of EFI_HII_PACKAGE_GUID_HEADER.
                      Otherwise, it must be NULL.

  @param Package  Points to the package referred to by the
                  notification Handle The handle of the package
                  list which contains the specified package.

  @param Handle       The HII handle.

  @param NotifyType   The type of change concerning the
                      database. See
                      EFI_HII_DATABASE_NOTIFY_TYPE.

**/
EFI_STATUS
EFIAPI
FormsetRemoveNotify (
  IN UINT8                              PackageType,
  IN CONST EFI_GUID                     *PackageGuid,
  IN CONST EFI_HII_PACKAGE_HEADER       *Package,
  IN EFI_HII_HANDLE                     Handle,
  IN EFI_HII_DATABASE_NOTIFY_TYPE       NotifyType
  )
{
  //
  // Remove formset action be canceled which may curse F9 error.
  //
  return EFI_SUCCESS;
}

/**
  Get Formset info for ui showing.

**/
BOOLEAN
GetFormsetInfoFromGuid (
  IN    EFI_GUID     *ComparingGuid,
  OUT    EFI_HII_HANDLE      *FormsetHandle,
  OUT    EFI_STRING_ID       *FormSetTitle
  )
{
  EFI_HII_HANDLE               *HiiHandles;
  UINTN                        Index;
  EFI_HII_PACKAGE_LIST_HEADER  *HiiPackageList;
  UINTN                        BufferSize;
  UINT32                       Offset;
  UINT32                       Offset2;
  UINT32                       PackageListLength;
  EFI_HII_PACKAGE_HEADER       PackageHeader;
  UINT8                        *Package;
  UINT8                        *OpCodeData;
  EFI_STATUS                   Status;
  EFI_HII_HANDLE               HiiHandle;

  ASSERT (ComparingGuid != NULL);

  HiiHandle  = NULL;
  //
  // Get all the Hii handles
  //
  HiiHandles = HiiGetHiiHandles (NULL);
  ASSERT (HiiHandles != NULL);

  //
  // Search for formset of each class type
  //
  for (Index = 0; HiiHandles[Index] != NULL; Index++) {
    BufferSize = 0;
    HiiPackageList = NULL;
    Status = mHiiDatabase->ExportPackageLists (mHiiDatabase, HiiHandles[Index], &BufferSize, HiiPackageList);
    if (Status == EFI_BUFFER_TOO_SMALL) {
      HiiPackageList = AllocatePool (BufferSize);
      ASSERT (HiiPackageList != NULL);

      Status = mHiiDatabase->ExportPackageLists (mHiiDatabase, HiiHandles[Index], &BufferSize, HiiPackageList);
    }
    if (EFI_ERROR (Status) || HiiPackageList == NULL) {
      return FALSE;
    }

    //
    // Get Form package from this HII package List
    //
    Offset = sizeof (EFI_HII_PACKAGE_LIST_HEADER);
    Offset2 = 0;
    CopyMem (&PackageListLength, &HiiPackageList->PackageLength, sizeof (UINT32));

    while (Offset < PackageListLength) {
      Package = ((UINT8 *) HiiPackageList) + Offset;
      CopyMem (&PackageHeader, Package, sizeof (EFI_HII_PACKAGE_HEADER));

      if (PackageHeader.Type == EFI_HII_PACKAGE_FORMS) {
        //
        // Search FormSet in this Form Package
        //
        Offset2 = sizeof (EFI_HII_PACKAGE_HEADER);
        while (Offset2 < PackageHeader.Length) {
          OpCodeData = Package + Offset2;

          if (((EFI_IFR_OP_HEADER *) OpCodeData)->OpCode == EFI_IFR_FORM_SET_OP) {
            //
            // Try to compare against formset GUID
            //
            if (CompareGuid (ComparingGuid, (EFI_GUID *)(OpCodeData + sizeof (EFI_IFR_OP_HEADER)))) {
              HiiHandle = HiiHandles[Index];
         *FormsetHandle = HiiHandles[Index];
    *FormSetTitle = (EFI_STRING_ID)((EFI_IFR_FORM_SET *) OpCodeData)->FormSetTitle;
              break;
            }
          }

          Offset2 += ((EFI_IFR_OP_HEADER *) OpCodeData)->Length;
        }
      }
      if (HiiHandle != NULL) {
        break;
      }
      Offset += PackageHeader.Length;
    }

    FreePool (HiiPackageList);
    if (HiiHandle != NULL) {
      break;
    }
  }

  FreePool (HiiHandles);
  if (NULL == HiiHandle) {
    return FALSE;
  } else {
    return TRUE;
  }
}

/**
  Inset  Byo Formset to ByoFormSetList.

**/
EFI_STATUS
InsertByoFormset (
  IN EFI_BYO_FORMSET_MANAGER_PROTOCOL    *This,
  IN EFI_GUID    *FormsetGuid
  )
{
  EFI_STATUS    Status = EFI_SUCCESS;
  BYO_BROWSER_FORMSET    * FormSet = NULL;
  LIST_ENTRY    *Link;
  //
  // Check Formset.
  //
  if (NULL == FormSetGuidToHiiHandle(FormsetGuid)) {
    return EFI_NOT_FOUND;
  }
  //
  //  Look for the same node.
  //
  Link = GetFirstNode (&This->ByoFormSetList);
  while (!IsNull (&This->ByoFormSetList, Link)) {
    FormSet = BYO_FORM_BROWSER_FORMSET_FROM_LINK (Link);
    Link = GetNextNode (&This->ByoFormSetList, Link);
    if (CompareGuid(&FormSet->Guid, FormsetGuid)) {
      DEBUG((EFI_D_ERROR, "InsertByoFormset(), GUID have be registered.\n"));
      return EFI_INVALID_PARAMETER;
    }
  }
  //
  // Insert new node.
  //
  FormSet = AllocateZeroPool (sizeof (BYO_BROWSER_FORMSET));
  ASSERT (FormSet != NULL);
  FormSet->Signature = BYO_FORM_BROWSER_FORMSET_SIGNATURE;
  CopyMem (&FormSet->Guid, FormsetGuid, sizeof (EFI_GUID));
  InsertTailList (&This->ByoFormSetList, &FormSet->Link);

  return Status;
}

/**
  Remove  Formset from ByoFormSetList.

**/
EFI_STATUS
RemoveByoFormset (
  IN EFI_BYO_FORMSET_MANAGER_PROTOCOL    *This,
  IN EFI_GUID    *FormsetGuid
  )
{
  EFI_STATUS    Status = EFI_SUCCESS;
  LIST_ENTRY    *Link;
  BYO_BROWSER_FORMSET   *FormSet = NULL;

  Link = GetFirstNode (&This->ByoFormSetList);
  while (!IsNull (&This->ByoFormSetList, Link)) {
    FormSet = BYO_FORM_BROWSER_FORMSET_FROM_LINK (Link);
    Link = GetNextNode (&This->ByoFormSetList, Link);

    if (CompareGuid(&FormSet->Guid, FormsetGuid)) {
      RemoveEntryList(&FormSet->Link);
      FreePool(FormSet);
    }
  }

  return Status;
}

VOID
ReconnectVga ()
{
  EFI_STATUS                    Status;
  EFI_HANDLE                    *HandleBuffer;
  UINTN                         NumberOfHandles;
  UINTN                         Index;

  HandleBuffer = NULL;
  
  Status = gBS->LocateHandleBuffer (
                   ByProtocol,
                   &gEfiGraphicsOutputProtocolGuid,
                   NULL,
                   &NumberOfHandles,
                   &HandleBuffer
                   );
  if (!EFI_ERROR (Status)) {
    for (Index = 0; Index < NumberOfHandles; Index++) {
      Status = gBS->DisconnectController (
                      HandleBuffer[Index],
                      NULL,
                      NULL
                      );
      if (!EFI_ERROR (Status)) {
        gBS->ConnectController (HandleBuffer[Index], NULL, NULL, TRUE);
      }
    }
    if(HandleBuffer != NULL){
      gBS->FreePool(HandleBuffer);
    }
  }
}

/**
  Send a formset.
  IF FormsetGuid is NULL, show this first one.

**/
EFI_STATUS
RunByoFormset (
  IN EFI_BYO_FORMSET_MANAGER_PROTOCOL    *This,
  IN EFI_GUID    *FormsetGuid
  )
{
  EFI_STATUS                    Status = EFI_SUCCESS;
  LIST_ENTRY                    *Link;
  EFI_BROWSER_ACTION_REQUEST    ActionRequest;
  BYO_BROWSER_FORMSET           *FormSet = NULL;
  BOOLEAN                       bFound;
  EFI_SCREEN_DESCRIPTOR         LocalScreen;
  EFI_INPUT_KEY                 Key;
  UINT8                         UCREnable;
  UINT32                        Horizontal, Vertical, Column, Row;
  BOOLEAN                       ResChanged = 0;

  //
  // when enter setup and UCR is disabled, then set resolution to 800 * 600
  //
  Horizontal = PcdGet32 (PcdVideoHorizontalResolution);
  Vertical   = PcdGet32 (PcdVideoVerticalResolution);
  Column     = PcdGet32 (PcdConOutColumn);
  Row        = PcdGet32 (PcdConOutRow);
  UCREnable  = PcdGet8 (PcdUCREnable);

  DEBUG((EFI_D_ERROR, "%a() %d*%d %d*%d %d\n", __FUNCTION__, Horizontal, Vertical, Column, Row, UCREnable));

  if (UCREnable == 0) {
    if (Horizontal != 1024 || Column != 128) {
      PcdSet32 (PcdConOutColumn, 128);
      PcdSet32 (PcdConOutRow, 40);
      PcdSet32 (PcdVideoHorizontalResolution, 1024);
      PcdSet32 (PcdVideoVerticalResolution, 768);
      ReconnectVga ();
      ResChanged = 1;
    }
  } else {
    //
    // UCR is enabled, set resolution to 640 * 480
    //
    if (Horizontal != 640 || Column != 80) {
      PcdSet32 (PcdConOutColumn, 80);
      PcdSet32 (PcdConOutRow, 25);
      PcdSet32 (PcdVideoHorizontalResolution, 640);
      PcdSet32 (PcdVideoVerticalResolution, 480);
      ReconnectVga ();
      ResChanged = 1;
    }
  }

  ZeroMem (&LocalScreen, sizeof (EFI_SCREEN_DESCRIPTOR));
  gST->ConOut->QueryMode (gST->ConOut, gST->ConOut->Mode->Mode, &LocalScreen.RightColumn, &LocalScreen.BottomRow);
  gST->ConOut->EnableCursor(gST->ConOut, FALSE);

  DEBUG((EFI_D_ERROR, "SetupBrowser, RunByoFormset(),\n"));

  //
  // Check ByoFormSetList.
  //
  //if (IsNull(&This->ByoFormSetList, GetFirstNode (&This->ByoFormSetList))) {
  if (IsListEmpty(&This->ByoFormSetList)) {
    DEBUG((EFI_D_ERROR, "RunByoFormset(), No any Formset be Inserted.\n"));
    do {
      CreateDialog (2, TRUE, 0, NULL, &Key, L"ByoFormsetManager, No Formset be Inserted.", L"Press Enter to Continue");
    } while (Key.UnicodeChar != CHAR_CARRIAGE_RETURN);
    return EFI_NOT_FOUND;
  }

  //
  // Initialize all HiiHandle.
  //
  Link = GetFirstNode (&This->ByoFormSetList);
  while (!IsNull (&This->ByoFormSetList, Link)) {
    FormSet = BYO_FORM_BROWSER_FORMSET_FROM_LINK (Link);
    Link = GetNextNode (&This->ByoFormSetList, Link);

    GetFormsetInfoFromGuid (
      &FormSet->Guid,
      &FormSet->HiiHandle,
      &FormSet->FormSetTitle
      );
  }
  //
  // Look for runed Formset.
  //
  bFound = FALSE;
  if (NULL == FormsetGuid) {
    Link = GetFirstNode (&This->ByoFormSetList);
    if (!IsNull (&This->ByoFormSetList, Link)) {
      bFound = TRUE;
      FormSet = BYO_FORM_BROWSER_FORMSET_FROM_LINK (Link);
    }
  } else {
    //
    //Look for Runed Guid.
    //
    Link = GetFirstNode (&This->ByoFormSetList);
    while (!IsNull (&This->ByoFormSetList, Link)) {
      FormSet = BYO_FORM_BROWSER_FORMSET_FROM_LINK (Link);
      if (CompareGuid(&FormSet->Guid, FormsetGuid)) {
        bFound = TRUE;
        break;
      }
      Link = GetNextNode (&This->ByoFormSetList, Link);
    }
    //
    // if not find Runed Guid, then run the first.
    //
    if (FALSE == bFound) {
      Link = GetFirstNode (&This->ByoFormSetList);
      if (!IsNull (&This->ByoFormSetList, Link)) {
        bFound = TRUE;
        FormSet = BYO_FORM_BROWSER_FORMSET_FROM_LINK (Link);
      }
    }
  }

  //
  // Not Found Message.
  //
  if (FALSE == bFound) {
    DEBUG((EFI_D_ERROR, "RunByoFormset(), No Formset be Found to Run.\n"));
    do {
      CreateDialog (2, TRUE, 0, NULL, &Key, L"ByoFormsetManager, No Formset be Found to Run.", L"Press Enter to Continue");
    } while (Key.UnicodeChar != CHAR_CARRIAGE_RETURN);
    return EFI_NOT_FOUND;
  }

  //
  // Set global variable of browser.
  //
  mHiiPackageListUpdated = FALSE;
  mCurrentHiiHandle = FormSet->HiiHandle;
  mCurrentFormId = 1;
  CopyGuid (&mCurrentFormSetGuid, &FormSet->Guid);
  bByoFormsetFlag = TRUE;
  bGotoGuidFormset = FALSE;
  //
  // Send form.
  //
  Status = SendForm (
                 &mPrivateData.FormBrowser2,
                 &FormSet->HiiHandle,
                 1,
                 &FormSet->Guid,
                 1,
                 NULL,
                 &ActionRequest
                 );

  //
  // After SendForm, Remove all Formset info.
  //
  Link = GetFirstNode (&This->ByoFormSetList);
  while (!IsNull (&This->ByoFormSetList, Link)) {
    FormSet = BYO_FORM_BROWSER_FORMSET_FROM_LINK (Link);
    Link = GetNextNode (&This->ByoFormSetList, Link);

    RemoveEntryList(&FormSet->Link);
    FreePool(FormSet);
  }
  gCurrentFormSetLink = NULL;
  gOldFormSet = NULL;
  bByoFormsetFlag = FALSE;
  //
  // exit setup restore resolution
  //
  if(ResChanged){
    PcdSet32 (PcdConOutColumn, Column);
    PcdSet32 (PcdConOutRow, Row);
    PcdSet32 (PcdVideoHorizontalResolution, Horizontal);
    PcdSet32 (PcdVideoVerticalResolution, Vertical);
    ReconnectVga ();
  }
  return Status;
}

/**
  Look for Formset with given Guid in hii database.

**/
BOOLEAN
SetupCheckFormsetWithGuid (
  IN EFI_GUID    *FormsetGuid
  )
{
  if (NULL == FormsetGuid) {
    return FALSE;
  }

  if (NULL == FormSetGuidToHiiHandle(FormsetGuid)) {
    return FALSE;
  } else {
    return TRUE;
  }
}

/**
  Checking Password when it have one.

**/
EFI_STATUS
SetupCheckPassword (
  IN CHAR16 *Title,
  IN CHAR16 *Password
  )
{
  EFI_STATUS Status = EFI_SUCCESS;
  CHAR16          NullCharacter = CHAR_NULL;
  CHAR16          *IncorrectPassword = L"Password is Incorrect";
  UINTN             ScreenSize = 0;
  UINTN             CurrentAttribute;
  EFI_INPUT_KEY    Key;
  //
  // Get Gop Dimension.
  //
  CurrentAttribute = gST->ConOut->Mode->Attribute;
  if (0 == gScreenDimensions.RightColumn || 0 == gScreenDimensions.BottomRow) {
    gST->ConOut->QueryMode (
                 gST->ConOut,
                 gST->ConOut->Mode->Mode,
                 &gScreenDimensions.RightColumn,
                 &gScreenDimensions.BottomRow
                 );
  }

  gST->ConOut->EnableCursor (gST->ConOut, FALSE);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BACKGROUND_BLACK);
  gST->ConOut->ClearScreen(gST->ConOut);

  if (NULL == Password) {
    //
    // Incorrect Password Dialog..
    //
    ScreenSize = PASSWORD_DIALOG_WIDTH;
    CreateMultiStringPopUp (ScreenSize, 4, &NullCharacter, IncorrectPassword,Title, &NullCharacter);
    do {
      WaitForKeyStroke (&Key);
    }while (CHAR_CARRIAGE_RETURN !=Key.UnicodeChar );

    gST->ConOut->SetAttribute (gST->ConOut, CurrentAttribute);
    gST->ConOut->EnableCursor (gST->ConOut, FALSE);
    return EFI_INVALID_PARAMETER;
  }

  ZeroMem(Password, (StrLen(Password) + 1) * sizeof(Password));
  Status = ReadString(NULL,Title,Password);

  gST->ConOut->SetAttribute (gST->ConOut, CurrentAttribute);
  gST->ConOut->EnableCursor (gST->ConOut, FALSE);
  return Status;
}

/**
  Install Byo Formset Manager Protocol.

**/
EFI_STATUS
InstallByoFormsetManagerProtocol (
  VOID
  )
{
  EFI_STATUS    Status = EFI_SUCCESS;
  EFI_HANDLE    Handle = NULL;
  EFI_BYO_FORMSET_MANAGER_PROTOCOL    *FormsetManager = NULL;

//DEBUG((EFI_D_INFO, "InstallByoFormsetManagerProtocol(),\n"));
  Status = gBS->LocateProtocol (
                  &gEfiByoFormsetManagerProtocolGuid,
                  NULL,
                  &FormsetManager
                  );
  if ( ! EFI_ERROR(Status) ) {
    DEBUG((EFI_D_ERROR, "InstallByoFormsetManagerProtocol(), Protocol Already Started.\n"));
    gByoFormsetManager = FormsetManager;
    return EFI_ALREADY_STARTED;
  }

  FormsetManager = AllocateZeroPool (sizeof(EFI_BYO_FORMSET_MANAGER_PROTOCOL));
  if (FormsetManager == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  InitializeListHead (&FormsetManager->ByoFormSetList);
  FormsetManager->Insert = InsertByoFormset;
  FormsetManager->Remove = RemoveByoFormset;
  FormsetManager->Run = RunByoFormset;
  FormsetManager->CheckPassword = SetupCheckPassword;
  FormsetManager->CheckFormset =  SetupCheckFormsetWithGuid;

  Status = gBS->InstallProtocolInterface (
                 &Handle,
                 &gEfiByoFormsetManagerProtocolGuid,
                 EFI_NATIVE_INTERFACE,
                 FormsetManager
                 );

  gByoFormsetManager = FormsetManager;
  return Status;
}

/**
  Initialize Setup Browser driver.

  @param ImageHandle     The image handle.
  @param SystemTable     The system table.

  @retval EFI_SUCCESS    The Setup Browser module is initialized correctly..
  @return Other value if failed to initialize the Setup Browser module.

**/
EFI_STATUS
EFIAPI
InitializeSetup (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS                  Status;
  EFI_HANDLE                  NotifyHandle;
  EFI_INPUT_KEY               DefaultHotKey;
  EFI_STRING                  HelpString;
  //
  // Locate required Hii relative protocols
  //
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **) &mHiiDatabase
                  );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->LocateProtocol (
                  &gEfiHiiStringProtocolGuid,
                  NULL,
                  (VOID **) &mHiiString
                  );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->LocateProtocol (
                  &gEfiHiiConfigRoutingProtocolGuid,
                  NULL,
                  (VOID **) &mHiiConfigRouting
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Publish our HII data
  //
  gHiiHandle = HiiAddPackages (
                 &gSetupBrowserGuid,
                 ImageHandle,
                 SetupBrowserStrings,
                 NULL
                 );
  ASSERT (gHiiHandle != NULL);

  //
  // Initialize Driver private data
  //
  gBannerData = AllocateZeroPool (sizeof (BANNER_DATA));
  ASSERT (gBannerData != NULL);

  //
  // Initialize generic help strings.
  //
  gSaveFailed    = GetToken (STRING_TOKEN (SAVE_FAILED), gHiiHandle);
  gDiscardFailed = GetToken (STRING_TOKEN (DISCARD_FAILED), gHiiHandle);
  gDefaultFailed = GetToken (STRING_TOKEN (DEFAULT_FAILED), gHiiHandle);

  //
  // Install FormBrowser2 protocol
  //
  mPrivateData.Handle = NULL;
  Status = gBS->InstallProtocolInterface (
                  &mPrivateData.Handle,
                  &gEfiFormBrowser2ProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mPrivateData.FormBrowser2
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Install default HotKey F10 for Save
  //
  DefaultHotKey.UnicodeChar = CHAR_NULL;
  HelpString             = GetToken (STRING_TOKEN (FUNCTION_TEN_STRING), gHiiHandle);
  DefaultHotKey.ScanCode = SCAN_F10;
  RegisterHotKey (&DefaultHotKey, BROWSER_ACTION_SUBMIT | BROWSER_ACTION_EXIT, 0, HelpString);
  FreePool (HelpString);
  //
  // Install default HotKey F9 for Reset To Defaults
  //
  DefaultHotKey.ScanCode    = SCAN_F9;
  HelpString                = GetToken (STRING_TOKEN (FUNCTION_NINE_STRING), gHiiHandle);
  RegisterHotKey (&DefaultHotKey, BROWSER_ACTION_DEFAULT, EFI_HII_DEFAULT_CLASS_STANDARD, HelpString);
  FreePool (HelpString);

  //
  // Install FormBrowserEx protocol
  //
  mPrivateData.Handle = NULL;
  Status = gBS->InstallProtocolInterface (
                  &mPrivateData.Handle,
                  &gEfiByoFormBrowserExProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mPrivateData.FormBrowserEx
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Install Byo Formset Manager Protocol.
  //
  InstallByoFormsetManagerProtocol();
  gByoFormSetList = &gByoFormsetManager->ByoFormSetList;

  //
  // Register notify for Form package remove
  //
  Status = mHiiDatabase->RegisterPackageNotify (
                           mHiiDatabase,
                           EFI_HII_PACKAGE_FORMS,
                           NULL,
                           FormsetRemoveNotify,
                           EFI_HII_DATABASE_NOTIFY_REMOVE_PACK,
                           &NotifyHandle
                           );
  ASSERT_EFI_ERROR (Status);

  return Status;
}


/**
  Create a new string in HII Package List.

  @param  String                 The String to be added
  @param  HiiHandle              The package list in the HII database to insert the
                                 specified string.

  @return The output string.

**/
EFI_STRING_ID
NewString (
  IN  CHAR16                   *String,
  IN  EFI_HII_HANDLE           HiiHandle
  )
{
  EFI_STRING_ID  StringId;

  StringId = HiiSetString (HiiHandle, 0, String, NULL);
  ASSERT (StringId != 0);

  return StringId;
}


/**
  Delete a string from HII Package List.

  @param  StringId               Id of the string in HII database.
  @param  HiiHandle              The HII package list handle.

  @retval EFI_SUCCESS            The string was deleted successfully.

**/
EFI_STATUS
DeleteString (
  IN  EFI_STRING_ID            StringId,
  IN  EFI_HII_HANDLE           HiiHandle
  )
{
  CHAR16  NullChar;

  NullChar = CHAR_NULL;
  HiiSetString (HiiHandle, StringId, &NullChar, NULL);
  return EFI_SUCCESS;
}


/**
  Get the string based on the StringId and HII Package List Handle.

  @param  Token                  The String's ID.
  @param  HiiHandle              The package list in the HII database to search for
                                 the specified string.

  @return The output string.

**/
CHAR16 *
GetToken (
  IN  EFI_STRING_ID                Token,
  IN  EFI_HII_HANDLE               HiiHandle
  )
{
  EFI_STRING  String;

  if (HiiHandle == NULL) {
    return NULL;
  }

  String = HiiGetString (HiiHandle, Token, NULL);
  if (String == NULL) {
    String = AllocateCopyPool (sizeof (mUnknownString), mUnknownString);
    ASSERT (String != NULL);
  }
  return (CHAR16 *) String;
}


/**
  Allocate new memory and then copy the Unicode string Source to Destination.

  @param  Dest                   Location to copy string
  @param  Src                    String to copy

**/
VOID
NewStringCpy (
  IN OUT CHAR16       **Dest,
  IN CHAR16           *Src
  )
{
  if (*Dest != NULL) {
    FreePool (*Dest);
  }
  *Dest = AllocateCopyPool (StrSize (Src), Src);
  ASSERT (*Dest != NULL);
}


/**
  Allocate new memory and concatinate Source on the end of Destination.

  @param  Dest                   String to added to the end of.
  @param  Src                    String to concatinate.

**/
VOID
NewStringCat (
  IN OUT CHAR16       **Dest,
  IN CHAR16           *Src
  )
{
  CHAR16  *NewString;
  UINTN   TmpSize;

  if (*Dest == NULL) {
    NewStringCpy (Dest, Src);
    return;
  }

  TmpSize = StrSize (*Dest);
  NewString = AllocateZeroPool (TmpSize + StrSize (Src) - 1);
  ASSERT (NewString != NULL);

  StrCpy (NewString, *Dest);
  StrCat (NewString, Src);

  FreePool (*Dest);
  *Dest = NewString;
}


/**
  Synchronize or restore Storage's Edit copy and Shadow copy.

  @param  Storage          The Storage to be synchronized.
  @param  SyncOrRestore    Sync the buffer to editbuffer or Restore  the
                           editbuffer to buffer
                           if TRUE, copy the editbuffer to the buffer.
                           if FALSE, copy the buffer to the editbuffer.

**/
VOID
SynchronizeStorage (
  IN FORMSET_STORAGE         *Storage,
  IN BOOLEAN                 SyncOrRestore
  )
{
  LIST_ENTRY              *Link;
  NAME_VALUE_NODE         *Node;

  switch (Storage->Type) {
  case EFI_HII_VARSTORE_BUFFER:
  case EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER:
    if (SyncOrRestore) {
      CopyMem (Storage->Buffer, Storage->EditBuffer, Storage->Size);
    } else {
      CopyMem (Storage->EditBuffer, Storage->Buffer, Storage->Size);
    }
    break;

  case EFI_HII_VARSTORE_NAME_VALUE:
    Link = GetFirstNode (&Storage->NameValueListHead);
    while (!IsNull (&Storage->NameValueListHead, Link)) {
      Node = NAME_VALUE_NODE_FROM_LINK (Link);

      if (SyncOrRestore) {
        NewStringCpy (&Node->Value, Node->EditValue);
      } else {
        NewStringCpy (&Node->EditValue, Node->Value);
      }

      Link = GetNextNode (&Storage->NameValueListHead, Link);
    }
    break;

  case EFI_HII_VARSTORE_EFI_VARIABLE:
  default:
    break;
  }
}


/**
  Get Value for given Name from a NameValue Storage.

  @param  Storage                The NameValue Storage.
  @param  Name                   The Name.
  @param  Value                  The retured Value.

  @retval EFI_SUCCESS            Value found for given Name.
  @retval EFI_NOT_FOUND          No such Name found in NameValue storage.

**/
EFI_STATUS
GetValueByName (
  IN FORMSET_STORAGE             *Storage,
  IN CHAR16                      *Name,
  IN OUT CHAR16                  **Value,
  IN GET_SET_QUESTION_VALUE_WITH GetValueFrom
  )
{
  LIST_ENTRY              *Link;
  NAME_VALUE_NODE         *Node;

  if (GetValueFrom != GetSetValueWithEditBuffer && GetValueFrom != GetSetValueWithBuffer) {
    return EFI_INVALID_PARAMETER;
  }

  *Value = NULL;

  Link = GetFirstNode (&Storage->NameValueListHead);
  while (!IsNull (&Storage->NameValueListHead, Link)) {
    Node = NAME_VALUE_NODE_FROM_LINK (Link);

    if (StrCmp (Name, Node->Name) == 0) {
      if (GetValueFrom == GetSetValueWithEditBuffer) {
        NewStringCpy (Value, Node->EditValue);
      } else {
        NewStringCpy (Value, Node->Value);
      }
      return EFI_SUCCESS;
    }

    Link = GetNextNode (&Storage->NameValueListHead, Link);
  }

  return EFI_NOT_FOUND;
}


/**
  Set Value of given Name in a NameValue Storage.

  @param  Storage                The NameValue Storage.
  @param  Name                   The Name.
  @param  Value                  The Value to set.
  @param  Edit                   Whether update editValue or Value.

  @retval EFI_SUCCESS            Value found for given Name.
  @retval EFI_NOT_FOUND          No such Name found in NameValue storage.

**/
EFI_STATUS
SetValueByName (
  IN FORMSET_STORAGE         *Storage,
  IN CHAR16                  *Name,
  IN CHAR16                  *Value,
  IN GET_SET_QUESTION_VALUE_WITH SetValueTo
  )
{
  LIST_ENTRY              *Link;
  NAME_VALUE_NODE         *Node;
  CHAR16                  *Buffer;

  if (SetValueTo != GetSetValueWithEditBuffer && SetValueTo != GetSetValueWithBuffer) {
    return EFI_INVALID_PARAMETER;
  }

  Link = GetFirstNode (&Storage->NameValueListHead);
  while (!IsNull (&Storage->NameValueListHead, Link)) {
    Node = NAME_VALUE_NODE_FROM_LINK (Link);

    if (StrCmp (Name, Node->Name) == 0) {
      if (SetValueTo == GetSetValueWithEditBuffer) {
        Buffer = Node->EditValue;
      } else {
        Buffer = Node->Value;
      }
      if (Buffer != NULL) {
        FreePool (Buffer);
      }
      Buffer = AllocateCopyPool (StrSize (Value), Value);
      ASSERT (Buffer != NULL);
      if (SetValueTo == GetSetValueWithEditBuffer) {
        Node->EditValue = Buffer;
      } else {
        Node->Value = Buffer;
      }
      return EFI_SUCCESS;
    }

    Link = GetNextNode (&Storage->NameValueListHead, Link);
  }

  return EFI_NOT_FOUND;
}


/**
  Convert setting of Buffer Storage or NameValue Storage to <ConfigResp>.

  @param  Buffer                 The Storage to be conveted.
  @param  ConfigResp             The returned <ConfigResp>.
  @param  SingleForm             Whether update data for single form or formset level.

  @retval EFI_SUCCESS            Convert success.
  @retval EFI_INVALID_PARAMETER  Incorrect storage type.

**/
EFI_STATUS
StorageToConfigResp (
  IN VOID                    *Buffer,
  IN CHAR16                  **ConfigResp,
  IN BOOLEAN                 SingleForm
  )
{
  EFI_STATUS  Status;
  EFI_STRING  Progress;
  LIST_ENTRY              *Link;
  NAME_VALUE_NODE         *Node;
  CHAR16                  *ConfigRequest;
  FORMSET_STORAGE         *Storage;
  FORM_BROWSER_CONFIG_REQUEST  *ConfigInfo;

  Status = EFI_SUCCESS;
  if (SingleForm) {
    ConfigInfo    = (FORM_BROWSER_CONFIG_REQUEST *) Buffer;
    Storage       = ConfigInfo->Storage;
    ConfigRequest = ConfigInfo->ConfigRequest;
  } else {
    Storage       = (FORMSET_STORAGE *) Buffer;
    ConfigRequest = Storage->ConfigRequest;
  }

  switch (Storage->Type) {
  case EFI_HII_VARSTORE_BUFFER:
  case EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER:
    Status = mHiiConfigRouting->BlockToConfig (
                                  mHiiConfigRouting,
                                  ConfigRequest,
                                  Storage->EditBuffer,
                                  Storage->Size,
                                  ConfigResp,
                                  &Progress
                                  );
    break;

  case EFI_HII_VARSTORE_NAME_VALUE:
    *ConfigResp = NULL;
    NewStringCat (ConfigResp, Storage->ConfigHdr);

    Link = GetFirstNode (&Storage->NameValueListHead);
    while (!IsNull (&Storage->NameValueListHead, Link)) {
      Node = NAME_VALUE_NODE_FROM_LINK (Link);

      if (StrStr (ConfigRequest, Node->Name) != NULL) {
        NewStringCat (ConfigResp, L"&");
        NewStringCat (ConfigResp, Node->Name);
        NewStringCat (ConfigResp, L"=");
        NewStringCat (ConfigResp, Node->EditValue);
      }
      Link = GetNextNode (&Storage->NameValueListHead, Link);
    }
    break;

  case EFI_HII_VARSTORE_EFI_VARIABLE:
  default:
    Status = EFI_INVALID_PARAMETER;
    break;
  }

  return Status;
}


/**
  Convert <ConfigResp> to settings in Buffer Storage or NameValue Storage.

  @param  Storage                The Storage to receive the settings.
  @param  ConfigResp             The <ConfigResp> to be converted.

  @retval EFI_SUCCESS            Convert success.
  @retval EFI_INVALID_PARAMETER  Incorrect storage type.

**/
EFI_STATUS
ConfigRespToStorage (
  IN FORMSET_STORAGE         *Storage,
  IN CHAR16                  *ConfigResp
  )
{
  EFI_STATUS  Status;
  EFI_STRING  Progress;
  UINTN       BufferSize;
  CHAR16      *StrPtr;
  CHAR16      *Name;
  CHAR16      *Value;

  Status = EFI_SUCCESS;

  switch (Storage->Type) {
  case EFI_HII_VARSTORE_BUFFER:
  case EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER:
    BufferSize = Storage->Size;
    Status = mHiiConfigRouting->ConfigToBlock (
                                  mHiiConfigRouting,
                                  ConfigResp,
                                  Storage->EditBuffer,
                                  &BufferSize,
                                  &Progress
                                  );
    break;

  case EFI_HII_VARSTORE_NAME_VALUE:
    StrPtr = StrStr (ConfigResp, L"PATH");
    if (StrPtr == NULL) {
      break;
    }
    StrPtr = StrStr (ConfigResp, L"&");
    while (StrPtr != NULL) {
      //
      // Skip '&'
      //
      StrPtr = StrPtr + 1;
      Name = StrPtr;
      StrPtr = StrStr (StrPtr, L"=");
      if (StrPtr == NULL) {
        break;
      }
      *StrPtr = 0;

      //
      // Skip '='
      //
      StrPtr = StrPtr + 1;
      Value = StrPtr;
      StrPtr = StrStr (StrPtr, L"&");
      if (StrPtr != NULL) {
        *StrPtr = 0;
      }
      SetValueByName (Storage, Name, Value, GetSetValueWithEditBuffer);
    }
    break;

  case EFI_HII_VARSTORE_EFI_VARIABLE:
  default:
    Status = EFI_INVALID_PARAMETER;
    break;
  }

  return Status;
}


/**
  Get Question's current Value.

  @param  FormSet                FormSet data structure.
  @param  Form                   Form data structure.
  @param  Question               Question to be initialized.
  @param  Cached                 TRUE:  get from Edit copy FALSE: get from original
                                 Storage

  @retval EFI_SUCCESS            The function completed successfully.

**/
EFI_STATUS
GetQuestionValue (
  IN FORM_BROWSER_FORMSET             *FormSet,
  IN FORM_BROWSER_FORM                *Form,
  IN OUT FORM_BROWSER_STATEMENT       *Question,
  IN GET_SET_QUESTION_VALUE_WITH      GetValueFrom
  )
{
  EFI_STATUS          Status;
  BOOLEAN             Enabled;
  BOOLEAN             Pending;
  UINT8               *Dst;
  UINTN               StorageWidth;
  EFI_TIME            EfiTime;
  FORMSET_STORAGE     *Storage;
  EFI_IFR_TYPE_VALUE  *QuestionValue;
  CHAR16              *ConfigRequest;
  CHAR16              *Progress;
  CHAR16              *Result;
  CHAR16              *Value;
  CHAR16              *StringPtr;
  UINTN               Length;
  UINTN               Index;
  UINTN               LengthStr;
  BOOLEAN             IsBufferStorage;
  BOOLEAN             IsString;
  CHAR16              TemStr[5];
  UINT8               DigitUint8;
  UINT8               *TemBuffer;

  Status = EFI_SUCCESS;
  Value  = NULL;
  Result = NULL;

  if (GetValueFrom >= GetSetValueWithMax) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Statement don't have storage, skip them
  //
  if (Question->QuestionId == 0) {
    return Status;
  }

  //
  // Question value is provided by an Expression, evaluate it
  //
  if (Question->ValueExpression != NULL) {
    Status = EvaluateExpression (FormSet, Form, Question->ValueExpression);
    if (!EFI_ERROR (Status)) {
      if (Question->ValueExpression->Result.Type == EFI_IFR_TYPE_BUFFER) {
        ASSERT (Question->HiiValue.Type == EFI_IFR_TYPE_BUFFER && Question->HiiValue.Buffer != NULL);
        if (Question->StorageWidth > Question->ValueExpression->Result.BufferLen) {
          CopyMem (Question->HiiValue.Buffer, Question->ValueExpression->Result.Buffer, Question->ValueExpression->Result.BufferLen);
          Question->HiiValue.BufferLen = Question->ValueExpression->Result.BufferLen;
        } else {
          CopyMem (Question->HiiValue.Buffer, Question->ValueExpression->Result.Buffer, Question->StorageWidth);
          Question->HiiValue.BufferLen = Question->StorageWidth;
        }
        FreePool (Question->ValueExpression->Result.Buffer);
      }
      Question->HiiValue.Type = Question->ValueExpression->Result.Type;
      CopyMem (&Question->HiiValue.Value, &Question->ValueExpression->Result.Value, sizeof (EFI_IFR_TYPE_VALUE));
    }
    return Status;
  }

  //
  // Get question value by read expression.
  //
  if (Question->ReadExpression != NULL && Form->FormType == STANDARD_MAP_FORM_TYPE) {
    Status = EvaluateExpression (FormSet, Form, Question->ReadExpression);
    if (!EFI_ERROR (Status) &&
      ((Question->ReadExpression->Result.Type < EFI_IFR_TYPE_OTHER) || (Question->ReadExpression->Result.Type == EFI_IFR_TYPE_BUFFER))) {
      //
      // Only update question value to the valid result.
      //
      if (Question->ReadExpression->Result.Type == EFI_IFR_TYPE_BUFFER) {
        ASSERT (Question->HiiValue.Type == EFI_IFR_TYPE_BUFFER && Question->HiiValue.Buffer != NULL);
        if (Question->StorageWidth > Question->ReadExpression->Result.BufferLen) {
          CopyMem (Question->HiiValue.Buffer, Question->ReadExpression->Result.Buffer, Question->ReadExpression->Result.BufferLen);
          Question->HiiValue.BufferLen = Question->ReadExpression->Result.BufferLen;
        } else {
          CopyMem (Question->HiiValue.Buffer, Question->ReadExpression->Result.Buffer, Question->StorageWidth);
          Question->HiiValue.BufferLen = Question->StorageWidth;
        }
        FreePool (Question->ReadExpression->Result.Buffer);
      }
      Question->HiiValue.Type = Question->ReadExpression->Result.Type;
      CopyMem (&Question->HiiValue.Value, &Question->ReadExpression->Result.Value, sizeof (EFI_IFR_TYPE_VALUE));
      return EFI_SUCCESS;
    }
  }

  //
  // Question value is provided by RTC
  //
  Storage = Question->Storage;
  QuestionValue = &Question->HiiValue.Value;
  if (Storage == NULL) {
    //
    // It's a Question without storage, or RTC date/time
    //
    if (Question->Operand == EFI_IFR_DATE_OP || Question->Operand == EFI_IFR_TIME_OP) {
      //
      // Date and time define the same Flags bit
      //
      switch (Question->Flags & EFI_QF_DATE_STORAGE) {
      case QF_DATE_STORAGE_TIME:
        Status = gRT->GetTime (&EfiTime, NULL);
        break;

      case QF_DATE_STORAGE_WAKEUP:
        Status = gRT->GetWakeupTime (&Enabled, &Pending, &EfiTime);
        break;

      case QF_DATE_STORAGE_NORMAL:
      default:
        //
        // For date/time without storage
        //
        return EFI_SUCCESS;
      }

      if (EFI_ERROR (Status)) {
        return Status;
      }

      if (Question->Operand == EFI_IFR_DATE_OP) {
        QuestionValue->date.Year  = EfiTime.Year;
        QuestionValue->date.Month = EfiTime.Month;
        QuestionValue->date.Day   = EfiTime.Day;
      } else {
        QuestionValue->time.Hour   = EfiTime.Hour;
        QuestionValue->time.Minute = EfiTime.Minute;
        QuestionValue->time.Second = EfiTime.Second;
      }
    }

    return EFI_SUCCESS;
  }

  //
  // Question value is provided by EFI variable
  //
  StorageWidth = Question->StorageWidth;
  if (Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE) {
    if (Question->BufferValue != NULL) {
      Dst = Question->BufferValue;
    } else {
      Dst = (UINT8 *) QuestionValue;
    }

    Status = gRT->GetVariable (
                     Question->VariableName,
                     &Storage->Guid,
                     NULL,
                     &StorageWidth,
                     Dst
                     );
    //
    // Always return success, even this EFI variable doesn't exist
    //
    return EFI_SUCCESS;
  }

  //
  // Question Value is provided by Buffer Storage or NameValue Storage
  //
  if (Question->BufferValue != NULL) {
    //
    // This Question is password or orderedlist
    //
    Dst = Question->BufferValue;
  } else {
    //
    // Other type of Questions
    //
    Dst = (UINT8 *) &Question->HiiValue.Value;
  }

  if (Storage->Type == EFI_HII_VARSTORE_BUFFER ||
      Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER) {
    IsBufferStorage = TRUE;
  } else {
    IsBufferStorage = FALSE;
  }
  IsString = (BOOLEAN) ((Question->HiiValue.Type == EFI_IFR_TYPE_STRING) ?  TRUE : FALSE);
  if (GetValueFrom == GetSetValueWithEditBuffer || GetValueFrom == GetSetValueWithBuffer ) {
    if (IsBufferStorage) {
      if (GetValueFrom == GetSetValueWithEditBuffer) {
        //
        // Copy from storage Edit buffer
        //
        CopyMem (Dst, Storage->EditBuffer + Question->VarStoreInfo.VarOffset, StorageWidth);
      } else {
        //
        // Copy from storage Edit buffer
        //
        CopyMem (Dst, Storage->Buffer + Question->VarStoreInfo.VarOffset, StorageWidth);
      }
    } else {
      Value = NULL;
      Status = GetValueByName (Storage, Question->VariableName, &Value, GetValueFrom);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      ASSERT (Value != NULL);
      LengthStr = StrLen (Value);
      Status    = EFI_SUCCESS;
      if (IsString) {
        //
        // Convert Config String to Unicode String, e.g "0041004200430044" => "ABCD"
        // Add string tail char L'\0' into Length
        //
        Length    = StorageWidth + sizeof (CHAR16);
        if (Length < ((LengthStr / 4 + 1) * 2)) {
          Status = EFI_BUFFER_TOO_SMALL;
        } else {
          StringPtr = (CHAR16 *) Dst;
          ZeroMem (TemStr, sizeof (TemStr));
          for (Index = 0; Index < LengthStr; Index += 4) {
            StrnCpy (TemStr, Value + Index, 4);
            StringPtr[Index/4] = (CHAR16) StrHexToUint64 (TemStr);
          }
          //
          // Add tailing L'\0' character
          //
          StringPtr[Index/4] = L'\0';
        }
      } else {
        if (StorageWidth < ((LengthStr + 1) / 2)) {
          Status = EFI_BUFFER_TOO_SMALL;
        } else {
          ZeroMem (TemStr, sizeof (TemStr));
          for (Index = 0; Index < LengthStr; Index ++) {
            TemStr[0] = Value[LengthStr - Index - 1];
            DigitUint8 = (UINT8) StrHexToUint64 (TemStr);
            if ((Index & 1) == 0) {
              Dst [Index/2] = DigitUint8;
            } else {
              Dst [Index/2] = (UINT8) ((DigitUint8 << 4) + Dst [Index/2]);
            }
          }
        }
      }

      FreePool (Value);
    }
  } else {
    if (Storage->Type == EFI_HII_VARSTORE_BUFFER || Storage->Type == EFI_HII_VARSTORE_NAME_VALUE) {
      //
      // Request current settings from Configuration Driver
      //
      if (FormSet->ConfigAccess == NULL) {
        return EFI_NOT_FOUND;
      }

      //
      // <ConfigRequest> ::= <ConfigHdr> + <BlockName> ||
      //                   <ConfigHdr> + "&" + <VariableName>
      //
      if (IsBufferStorage) {
        Length = StrLen (Storage->ConfigHdr);
        Length += StrLen (Question->BlockName);
      } else {
        Length = StrLen (Storage->ConfigHdr);
        Length += StrLen (Question->VariableName) + 1;
      }
      ConfigRequest = AllocateZeroPool ((Length + 1) * sizeof (CHAR16));
      ASSERT (ConfigRequest != NULL);

      StrCpy (ConfigRequest, Storage->ConfigHdr);
      if (IsBufferStorage) {
        StrCat (ConfigRequest, Question->BlockName);
      } else {
        StrCat (ConfigRequest, L"&");
        StrCat (ConfigRequest, Question->VariableName);
      }

      Status = FormSet->ConfigAccess->ExtractConfig (
                                        FormSet->ConfigAccess,
                                        ConfigRequest,
                                        &Progress,
                                        &Result
                                        );
      FreePool (ConfigRequest);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      //
      // Skip <ConfigRequest>
      //
      if (IsBufferStorage) {
        Value = StrStr (Result, L"&VALUE");
        if (Value == NULL) {
          FreePool (Result);
          return EFI_NOT_FOUND;
        }
        //
        // Skip "&VALUE"
        //
        Value = Value + 6;
      } else {
        Value = Result + Length;
      }
      if (*Value != '=') {
        FreePool (Result);
        return EFI_NOT_FOUND;
      }
      //
      // Skip '=', point to value
      //
      Value = Value + 1;

      //
      // Suppress <AltResp> if any
      //
      StringPtr = Value;
      while (*StringPtr != L'\0' && *StringPtr != L'&') {
        StringPtr++;
      }
      *StringPtr = L'\0';

      LengthStr = StrLen (Value);
      Status    = EFI_SUCCESS;
      if (!IsBufferStorage && IsString) {
        //
        // Convert Config String to Unicode String, e.g "0041004200430044" => "ABCD"
        // Add string tail char L'\0' into Length
        //
        Length    = StorageWidth + sizeof (CHAR16);
        if (Length < ((LengthStr / 4 + 1) * 2)) {
          Status = EFI_BUFFER_TOO_SMALL;
        } else {
          StringPtr = (CHAR16 *) Dst;
          ZeroMem (TemStr, sizeof (TemStr));
          for (Index = 0; Index < LengthStr; Index += 4) {
            StrnCpy (TemStr, Value + Index, 4);
            StringPtr[Index/4] = (CHAR16) StrHexToUint64 (TemStr);
          }
          //
          // Add tailing L'\0' character
          //
          StringPtr[Index/4] = L'\0';
        }
      } else {
        if (StorageWidth < ((LengthStr + 1) / 2)) {
          Status = EFI_BUFFER_TOO_SMALL;
        } else {
          ZeroMem (TemStr, sizeof (TemStr));
          for (Index = 0; Index < LengthStr; Index ++) {
            TemStr[0] = Value[LengthStr - Index - 1];
            DigitUint8 = (UINT8) StrHexToUint64 (TemStr);
            if ((Index & 1) == 0) {
              Dst [Index/2] = DigitUint8;
            } else {
              Dst [Index/2] = (UINT8) ((DigitUint8 << 4) + Dst [Index/2]);
            }
          }
        }
      }

      if (EFI_ERROR (Status)) {
        FreePool (Result);
        return Status;
      }
    } else if (Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER) {
      TemBuffer = NULL;
      TemBuffer = AllocateZeroPool (Storage->Size);
      if (TemBuffer == NULL) {
        Status = EFI_OUT_OF_RESOURCES;
        return Status;
      }
      Length = Storage->Size;
      Status = gRT->GetVariable (
                       Storage->Name,
                       &Storage->Guid,
                       NULL,
                       &Length,
                       TemBuffer
                       );
      if (EFI_ERROR (Status)) {
        FreePool (TemBuffer);
        return Status;
      }

      CopyMem (Dst, TemBuffer + Question->VarStoreInfo.VarOffset, StorageWidth);

      FreePool (TemBuffer);
    }

    //
    // Synchronize Edit Buffer
    //
    if (IsBufferStorage) {
      CopyMem (Storage->EditBuffer + Question->VarStoreInfo.VarOffset, Dst, StorageWidth);
    } else {
      SetValueByName (Storage, Question->VariableName, Value, GetSetValueWithEditBuffer);
    }

    if (Result != NULL) {
      FreePool (Result);
    }
  }

  return Status;
}


/**
  Save Question Value to edit copy(cached) or Storage(uncached).

  @param  FormSet                FormSet data structure.
  @param  Form                   Form data structure.
  @param  Question               Pointer to the Question.
  @param  Cached                 TRUE:  set to Edit copy FALSE: set to original
                                 Storage

  @retval EFI_SUCCESS            The function completed successfully.

**/
EFI_STATUS
SetQuestionValue (
  IN FORM_BROWSER_FORMSET             *FormSet,
  IN FORM_BROWSER_FORM                *Form,
  IN OUT FORM_BROWSER_STATEMENT       *Question,
  IN GET_SET_QUESTION_VALUE_WITH      SetValueTo
  )
{
  EFI_STATUS          Status;
  BOOLEAN             Enabled;
  BOOLEAN             Pending;
  UINT8               *Src;
  EFI_TIME            EfiTime;
  UINTN               BufferLen;
  UINTN               StorageWidth;
  FORMSET_STORAGE     *Storage;
  EFI_IFR_TYPE_VALUE  *QuestionValue;
  CHAR16              *ConfigResp;
  CHAR16              *Progress;
  CHAR16              *Value;
  UINTN               Length;
  BOOLEAN             IsBufferStorage;
  BOOLEAN             IsString;
  UINT8               *TemBuffer;
  CHAR16              *TemName;
  CHAR16              *TemString;
  UINTN               Index;

  Status = EFI_SUCCESS;

  if (SetValueTo >= GetSetValueWithMax) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Statement don't have storage, skip them
  //
  if (Question->QuestionId == 0) {
    return Status;
  }

  //
  // If Question value is provided by an Expression, then it is read only
  //
  if (Question->ValueExpression != NULL) {
    return Status;
  }

  //
  // Before set question value, evaluate its write expression.
  //
  if (Question->WriteExpression != NULL && Form->FormType == STANDARD_MAP_FORM_TYPE) {
    Status = EvaluateExpression (FormSet, Form, Question->WriteExpression);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  //
  // Question value is provided by RTC
  //
  Storage = Question->Storage;
  QuestionValue = &Question->HiiValue.Value;
  if (Storage == NULL) {
    //
    // It's a Question without storage, or RTC date/time
    //
    if (Question->Operand == EFI_IFR_DATE_OP || Question->Operand == EFI_IFR_TIME_OP) {
      //
      // Date and time define the same Flags bit
      //
      switch (Question->Flags & EFI_QF_DATE_STORAGE) {
      case QF_DATE_STORAGE_TIME:
        Status = gRT->GetTime (&EfiTime, NULL);
        break;

      case QF_DATE_STORAGE_WAKEUP:
        Status = gRT->GetWakeupTime (&Enabled, &Pending, &EfiTime);
        break;

      case QF_DATE_STORAGE_NORMAL:
      default:
        //
        // For date/time without storage
        //
        return EFI_SUCCESS;
      }

      if (EFI_ERROR (Status)) {
        return Status;
      }

      if (Question->Operand == EFI_IFR_DATE_OP) {
        EfiTime.Year  = QuestionValue->date.Year;
        EfiTime.Month = QuestionValue->date.Month;
        EfiTime.Day   = QuestionValue->date.Day;
      } else {
        EfiTime.Hour   = QuestionValue->time.Hour;
        EfiTime.Minute = QuestionValue->time.Minute;
        EfiTime.Second = QuestionValue->time.Second;
      }

      if ((Question->Flags & EFI_QF_DATE_STORAGE) == QF_DATE_STORAGE_TIME) {
        Status = gRT->SetTime (&EfiTime);
      } else {
        Status = gRT->SetWakeupTime (TRUE, &EfiTime);
      }
    }

    return Status;
  }

  //
  // Question value is provided by EFI variable
  //
  StorageWidth = Question->StorageWidth;
  if (Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE) {
    if (Question->BufferValue != NULL) {
      Src = Question->BufferValue;
    } else {
      Src = (UINT8 *) QuestionValue;
    }

    Status = gRT->SetVariable (
                     Question->VariableName,
                     &Storage->Guid,
                     Storage->Attributes,
                     StorageWidth,
                     Src
                     );
    return Status;
  }

  //
  // Question Value is provided by Buffer Storage or NameValue Storage
  //
  if (Question->BufferValue != NULL) {
    Src = Question->BufferValue;
  } else {
    Src = (UINT8 *) &Question->HiiValue.Value;
  }

  if (Storage->Type == EFI_HII_VARSTORE_BUFFER ||
      Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER) {
    IsBufferStorage = TRUE;
  } else {
    IsBufferStorage = FALSE;
  }
  IsString = (BOOLEAN) ((Question->HiiValue.Type == EFI_IFR_TYPE_STRING) ?  TRUE : FALSE);

  if (SetValueTo == GetSetValueWithEditBuffer || SetValueTo == GetSetValueWithBuffer) {
    if (IsBufferStorage) {
      if (SetValueTo == GetSetValueWithEditBuffer) {
        //
        // Copy to storage edit buffer
        //
        CopyMem (Storage->EditBuffer + Question->VarStoreInfo.VarOffset, Src, StorageWidth);
      } else if (SetValueTo == GetSetValueWithBuffer) {
        //
        // Copy to storage edit buffer
        //
        CopyMem (Storage->Buffer + Question->VarStoreInfo.VarOffset, Src, StorageWidth);
      }
    } else {
      if (IsString) {
        //
        // Allocate enough string buffer.
        //
      Value = NULL;
      BufferLen = ((StrLen ((CHAR16 *) Src) * 4) + 1) * sizeof (CHAR16);
      Value = AllocateZeroPool (BufferLen);
      ASSERT (Value != NULL);
      //
      // Convert Unicode String to Config String, e.g. "ABCD" => "0041004200430044"
      //
      TemName = (CHAR16 *) Src;
      TemString = Value;
      for (; *TemName != L'\0'; TemName++) {
        TemString += UnicodeValueToString (TemString, PREFIX_ZERO | RADIX_HEX, *TemName, 4);
      }
    } else {
      BufferLen = StorageWidth * 2 + 1;
      Value = AllocateZeroPool (BufferLen * sizeof (CHAR16));
      ASSERT (Value != NULL);
      //
      // Convert Buffer to Hex String
      //
        TemBuffer = Src + StorageWidth - 1;
        TemString = Value;
        for (Index = 0; Index < StorageWidth; Index ++, TemBuffer --) {
          TemString += UnicodeValueToString (TemString, PREFIX_ZERO | RADIX_HEX, *TemBuffer, 2);
        }
      }

      Status = SetValueByName (Storage, Question->VariableName, Value, SetValueTo);
      FreePool (Value);
    }
  } else if (SetValueTo == GetSetValueWithHiiDriver) {
    if (Storage->Type == EFI_HII_VARSTORE_BUFFER || Storage->Type == EFI_HII_VARSTORE_NAME_VALUE) {
      //
      // <ConfigResp> ::= <ConfigHdr> + <BlockName> + "&VALUE=" + "<HexCh>StorageWidth * 2" ||
      //                <ConfigHdr> + "&" + <VariableName> + "=" + "<string>"
      //
      if (IsBufferStorage) {
        Length = StrLen (Question->BlockName) + 7;
      } else {
        Length = StrLen (Question->VariableName) + 2;
      }
      if (!IsBufferStorage && IsString) {
        Length += (StrLen ((CHAR16 *) Src) * 4);
      } else {
        Length += (StorageWidth * 2);
      }
      ConfigResp = AllocateZeroPool ((StrLen (Storage->ConfigHdr) + Length + 1) * sizeof (CHAR16));
      ASSERT (ConfigResp != NULL);

      StrCpy (ConfigResp, Storage->ConfigHdr);
      if (IsBufferStorage) {
        StrCat (ConfigResp, Question->BlockName);
        StrCat (ConfigResp, L"&VALUE=");
      } else {
        StrCat (ConfigResp, L"&");
        StrCat (ConfigResp, Question->VariableName);
        StrCat (ConfigResp, L"=");
      }

      Value = ConfigResp + StrLen (ConfigResp);

      if (!IsBufferStorage && IsString) {
        //
        // Convert Unicode String to Config String, e.g. "ABCD" => "0041004200430044"
        //
        TemName = (CHAR16 *) Src;
        TemString = Value;
        for (; *TemName != L'\0'; TemName++) {
          TemString += UnicodeValueToString (TemString, PREFIX_ZERO | RADIX_HEX, *TemName, 4);
        }
      } else {
        //
        // Convert Buffer to Hex String
        //
        TemBuffer = Src + StorageWidth - 1;
        TemString = Value;
        for (Index = 0; Index < StorageWidth; Index ++, TemBuffer --) {
          TemString += UnicodeValueToString (TemString, PREFIX_ZERO | RADIX_HEX, *TemBuffer, 2);
        }
      }

      //
      // Convert to lower char.
      //
      for (TemString = Value; *Value != L'\0'; Value++) {
        if (*Value >= L'A' && *Value <= L'Z') {
          *Value = (CHAR16) (*Value - L'A' + L'a');
        }
      }

      //
      // Submit Question Value to Configuration Driver
      //
      if (FormSet->ConfigAccess != NULL) {
        Status = FormSet->ConfigAccess->RouteConfig (
                                          FormSet->ConfigAccess,
                                          ConfigResp,
                                          &Progress
                                          );
        if (EFI_ERROR (Status)) {
          FreePool (ConfigResp);
          return Status;
        }
      }
      FreePool (ConfigResp);

    } else if (Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER) {
      TemBuffer = NULL;
      TemBuffer = AllocateZeroPool(Storage->Size);
      if (TemBuffer == NULL) {
        Status = EFI_OUT_OF_RESOURCES;
        return Status;
      }
      Length = Storage->Size;
      Status = gRT->GetVariable (
                       Storage->Name,
                       &Storage->Guid,
                       NULL,
                       &Length,
                       TemBuffer
                       );

      CopyMem (TemBuffer + Question->VarStoreInfo.VarOffset, Src, StorageWidth);

      Status = gRT->SetVariable (
                       Storage->Name,
                       &Storage->Guid,
                       Storage->Attributes,
                       Storage->Size,
                       TemBuffer
                       );
      FreePool (TemBuffer);
      if (EFI_ERROR (Status)){
        return Status;
      }
    }
    //
    // Sync storage, from editbuffer to buffer.
    //
    CopyMem (Storage->Buffer + Question->VarStoreInfo.VarOffset, Src, StorageWidth);
  }

  return Status;
}


/**
  Perform inconsistent check for a Form.

  @param  FormSet                FormSet data structure.
  @param  Form                   Form data structure.
  @param  Question               The Question to be validated.
  @param  Type                   Validation type: InConsistent or NoSubmit

  @retval EFI_SUCCESS            Form validation pass.
  @retval other                  Form validation failed.

**/
EFI_STATUS
ValidateQuestion (
  IN  FORM_BROWSER_FORMSET            *FormSet,
  IN  FORM_BROWSER_FORM               *Form,
  IN  FORM_BROWSER_STATEMENT          *Question,
  IN  UINTN                           Type
  )
{
  EFI_STATUS              Status;
  LIST_ENTRY              *Link;
  LIST_ENTRY              *ListHead;
  EFI_STRING              PopUp;
  EFI_INPUT_KEY           Key;
  FORM_EXPRESSION         *Expression;

  if (Type == EFI_HII_EXPRESSION_INCONSISTENT_IF) {
    ListHead = &Question->InconsistentListHead;
  } else if (Type == EFI_HII_EXPRESSION_NO_SUBMIT_IF) {
    ListHead = &Question->NoSubmitListHead;
  } else {
    return EFI_UNSUPPORTED;
  }

  Link = GetFirstNode (ListHead);
  while (!IsNull (ListHead, Link)) {
    Expression = FORM_EXPRESSION_FROM_LINK (Link);

    //
    // Evaluate the expression
    //
    Status = EvaluateExpression (FormSet, Form, Expression);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    if ((Expression->Result.Type == EFI_IFR_TYPE_BOOLEAN) && Expression->Result.Value.b) {
      //
      // Condition meet, show up error message
      //
      if (Expression->Error != 0) {
        PopUp = GetToken (Expression->Error, FormSet->HiiHandle);
        do {
          CreateDialog (4, TRUE, 0, NULL, &Key, gEmptyString, PopUp, gPressEnter, gEmptyString);
        } while (Key.UnicodeChar != CHAR_CARRIAGE_RETURN);
        FreePool (PopUp);
      }

      return EFI_NOT_READY;
    }

    Link = GetNextNode (ListHead, Link);
  }

  return EFI_SUCCESS;
}


/**
  Perform NoSubmit check for each Form in FormSet.

  @param  FormSet                FormSet data structure.
  @param  CurrentForm            Current input form data structure.

  @retval EFI_SUCCESS            Form validation pass.
  @retval other                  Form validation failed.

**/
EFI_STATUS
NoSubmitCheck (
  IN  FORM_BROWSER_FORMSET            *FormSet,
  IN  FORM_BROWSER_FORM               *CurrentForm
  )
{
  EFI_STATUS              Status;
  LIST_ENTRY              *Link;
  FORM_BROWSER_STATEMENT  *Question;
  FORM_BROWSER_FORM       *Form;
  LIST_ENTRY              *LinkForm;

  LinkForm = GetFirstNode (&FormSet->FormListHead);
  while (!IsNull (&FormSet->FormListHead, LinkForm)) {
    Form = FORM_BROWSER_FORM_FROM_LINK (LinkForm);
    LinkForm = GetNextNode (&FormSet->FormListHead, LinkForm);

    if (CurrentForm != NULL && CurrentForm != Form) {
      continue;
    }

    Link = GetFirstNode (&Form->StatementListHead);
    while (!IsNull (&Form->StatementListHead, Link)) {
      Question = FORM_BROWSER_STATEMENT_FROM_LINK (Link);

      Status = ValidateQuestion (FormSet, Form, Question, EFI_HII_EXPRESSION_NO_SUBMIT_IF);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      Link = GetNextNode (&Form->StatementListHead, Link);
    }
  }

  return EFI_SUCCESS;
}

/**
  Fill storage's edit copy with settings requested from Configuration Driver.

  @param  FormSet                FormSet data structure.
  @param  ConfigInfo             The config info related to this form.
  @param  SyncOrRestore          Sync the buffer to editbuffer or Restore  the
                                 editbuffer to buffer
                                 if TRUE, copy the editbuffer to the buffer.
                                 if FALSE, copy the buffer to the editbuffer.

  @retval EFI_SUCCESS            The function completed successfully.

**/
EFI_STATUS
SynchronizeStorageForForm (
  IN FORM_BROWSER_FORMSET        *FormSet,
  IN FORM_BROWSER_CONFIG_REQUEST *ConfigInfo,
  IN BOOLEAN                     SyncOrRestore
  )
{
  EFI_STATUS              Status;
  EFI_STRING              Progress;
  EFI_STRING              Result;
  UINTN                   BufferSize;
  LIST_ENTRY              *Link;
  NAME_VALUE_NODE         *Node;
  UINT8                   *Src;
  UINT8                   *Dst;

  Status = EFI_SUCCESS;
  Result = NULL;
  if (FormSet->ConfigAccess == NULL && ConfigInfo->Storage->Type != EFI_HII_VARSTORE_NAME_VALUE) {
    return EFI_NOT_FOUND;
  }

  if (ConfigInfo->ElementCount == 0) {
    //
    // Skip if there is no RequestElement
    //
    return EFI_SUCCESS;
  }

  if (ConfigInfo->Storage->Type == EFI_HII_VARSTORE_BUFFER ||
      (ConfigInfo->Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER)) {
    BufferSize = ConfigInfo->Storage->Size;

    if (SyncOrRestore) {
      Src = ConfigInfo->Storage->EditBuffer;
      Dst = ConfigInfo->Storage->Buffer;
    } else {
      Src = ConfigInfo->Storage->Buffer;
      Dst = ConfigInfo->Storage->EditBuffer;
    }

    Status = mHiiConfigRouting->BlockToConfig(
                                  mHiiConfigRouting,
                                  ConfigInfo->ConfigRequest,
                                  Src,
                                  BufferSize,
                                  &Result,
                                  &Progress
                                  );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Status = mHiiConfigRouting->ConfigToBlock (
                                  mHiiConfigRouting,
                                  Result,
                                  Dst,
                                  &BufferSize,
                                  &Progress
                                  );
    if (Result != NULL) {
      FreePool (Result);
    }
  } else if (ConfigInfo->Storage->Type == EFI_HII_VARSTORE_NAME_VALUE) {
    Link = GetFirstNode (&ConfigInfo->Storage->NameValueListHead);
    while (!IsNull (&ConfigInfo->Storage->NameValueListHead, Link)) {
      Node = NAME_VALUE_NODE_FROM_LINK (Link);

      if (StrStr (ConfigInfo->ConfigRequest, Node->Name) != NULL) {
        if (SyncOrRestore) {
          NewStringCpy (&Node->Value, Node->EditValue);
        } else {
          NewStringCpy (&Node->EditValue, Node->Value);
        }
      }

      Link = GetNextNode (&ConfigInfo->Storage->NameValueListHead, Link);
    }
  }

  return Status;
}

/**
  When discard the question value, call the callback function with Changed type
  to inform the hii driver.

  @param  FormSet                FormSet data structure.
  @param  Form                   Form data structure.

**/
VOID
SendDiscardInfoToDriver (
  IN FORM_BROWSER_FORMSET             *FormSet,
  IN FORM_BROWSER_FORM                *Form
  )
{
  LIST_ENTRY                  *Link;
  FORM_BROWSER_STATEMENT      *Question;
  EFI_STATUS                  Status;
  EFI_HII_VALUE               HiiValue;
  UINT8                       *BufferValue;
  BOOLEAN                     ValueChanged;
  EFI_IFR_TYPE_VALUE          *TypeValue;
  EFI_BROWSER_ACTION_REQUEST  ActionRequest;

  ValueChanged = FALSE;
  BufferValue  = NULL;

  if(!Form->NvUpdateRequired) {
    return;
  }

  Link = GetFirstNode (&Form->StatementListHead);
  while (!IsNull (&Form->StatementListHead, Link)) {
    Question = FORM_BROWSER_STATEMENT_FROM_LINK (Link);
    Link = GetNextNode (&Form->StatementListHead, Link);

    if (Question->Storage == NULL || Question->Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE) {
      continue;
    }

    if (Question->Operand == EFI_IFR_PASSWORD_OP) {
      continue;
    }

    if (Question->BufferValue != NULL) {
      BufferValue = AllocateZeroPool (Question->StorageWidth);
      ASSERT (BufferValue != NULL);
      CopyMem (BufferValue, Question->BufferValue, Question->StorageWidth);
    } else {
      HiiValue.Type = Question->HiiValue.Type;
      CopyMem (&HiiValue.Value, &Question->HiiValue.Value, sizeof (EFI_IFR_TYPE_VALUE));
    }

    Status = GetQuestionValue (FormSet, Form, Question, GetSetValueWithBuffer);
    if (EFI_ERROR (Status)) {
      if (BufferValue != NULL) {
        FreePool (BufferValue);
        BufferValue = NULL;
      }
      continue;
    }

    if (Question->BufferValue != NULL) {
      if (CompareMem (BufferValue, Question->BufferValue, Question->StorageWidth)) {
        ValueChanged = TRUE;
      }
    } else {
      if (CompareMem (&HiiValue.Value, &Question->HiiValue.Value, sizeof (EFI_IFR_TYPE_VALUE))) {
        ValueChanged = TRUE;
      }
    }

    if (BufferValue != NULL) {
      FreePool (BufferValue);
      BufferValue = NULL;
    }

    if (!ValueChanged) {
      continue;
    }

    ValueChanged = FALSE;

    if (Question->HiiValue.Type == EFI_IFR_TYPE_BUFFER) {
      TypeValue = (EFI_IFR_TYPE_VALUE *) Question->BufferValue;
    } else {
      TypeValue = &Question->HiiValue.Value;
    }

    ActionRequest = EFI_BROWSER_ACTION_REQUEST_NONE;
    FormSet->ConfigAccess->Callback (
                             FormSet->ConfigAccess,
                             EFI_BROWSER_ACTION_CHANGED,
                             Question->QuestionId,
                             Question->HiiValue.Type,
                             TypeValue,
                             &ActionRequest
                             );
  }
}

/**
  Discard data based on the input setting scope (Form, FormSet or System).

  @param  FormSet                FormSet data structure.
  @param  Form                   Form data structure.
  @param  SettingScope           Setting Scope for Discard action.

  @retval EFI_SUCCESS            The function completed successfully.
  @retval EFI_UNSUPPORTED        Unsupport SettingScope.

**/
EFI_STATUS
DiscardForm (
  IN FORM_BROWSER_FORMSET             *FormSet,
  IN FORM_BROWSER_FORM                *Form,
  IN BROWSER_SETTING_SCOPE            SettingScope
  )
{
  LIST_ENTRY                   *Link;
  FORMSET_STORAGE              *Storage;
  FORM_BROWSER_CONFIG_REQUEST  *ConfigInfo;
  FORM_BROWSER_FORMSET    *LocalFormSet;

  //
  // Check the supported setting level.
  //
  if (SettingScope >= MaxLevel) {
    return EFI_UNSUPPORTED;
  }

  if (SettingScope == FormLevel && Form->NvUpdateRequired) {
    ConfigInfo = NULL;
    Link = GetFirstNode (&Form->ConfigRequestHead);
    while (!IsNull (&Form->ConfigRequestHead, Link)) {
      ConfigInfo = FORM_BROWSER_CONFIG_REQUEST_FROM_LINK (Link);
      Link = GetNextNode (&Form->ConfigRequestHead, Link);

      if (ConfigInfo->Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE) {
        continue;
      }

      //
      // Skip if there is no RequestElement
      //
      if (ConfigInfo->ElementCount == 0) {
        continue;
      }

      //
      // Prepare <ConfigResp>
      //
      SynchronizeStorageForForm(FormSet, ConfigInfo, FALSE);

      //
      // Call callback with Changed type to inform the driver.
      //
      SendDiscardInfoToDriver (FormSet, Form);
    }

    Form->NvUpdateRequired = FALSE;
  } else if (SettingScope == FormSetLevel && IsNvUpdateRequired(FormSet)) {
    //
    // Discard Buffer storage or Name/Value storage
    //
    Link = GetFirstNode (&FormSet->StorageListHead);
    while (!IsNull (&FormSet->StorageListHead, Link)) {
      Storage = FORMSET_STORAGE_FROM_LINK (Link);
      Link = GetNextNode (&FormSet->StorageListHead, Link);

      if (Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE) {
        continue;
      }

      //
      // Skip if there is no RequestElement
      //
      if (Storage->ElementCount == 0) {
        continue;
      }

      SynchronizeStorage(Storage, FALSE);
    }

    Link = GetFirstNode (&FormSet->FormListHead);
    while (!IsNull (&FormSet->FormListHead, Link)) {
      Form = FORM_BROWSER_FORM_FROM_LINK (Link);
      Link = GetNextNode (&FormSet->FormListHead, Link);

      //
      // Call callback with Changed type to inform the driver.
      //
      SendDiscardInfoToDriver (FormSet, Form);
    }

    UpdateNvInfoInForm (FormSet, FALSE);
  } else if (SettingScope == SystemLevel) {
    //
    // System Level Discard.
    //

    //
    // Discard changed value for each FormSet in the maintain list.
    //
    Link = GetFirstNode (&gBrowserFormSetList);
    while (!IsNull (&gBrowserFormSetList, Link)) {
      LocalFormSet = FORM_BROWSER_FORMSET_FROM_LINK (Link);
      DiscardForm (LocalFormSet, NULL, FormSetLevel);
      Link = GetNextNode (&gBrowserFormSetList, Link);
      if (!IsHiiHandleInBrowserContext (LocalFormSet->HiiHandle)) {
        //
        // Remove maintain backup list after discard except for the current using FormSet.
        //
        if (!IsNvUpdateRequired(LocalFormSet)) {
          RemoveEntryList (&LocalFormSet->Link);
          DestroyFormSet (LocalFormSet);
        }
      }
    }
  }

  //
  // Destroy Modify FormSet.
  //
    Link = GetFirstNode (&gBrowserFormSetList);
    while (!IsNull (&gBrowserFormSetList, Link)) {
      LocalFormSet = FORM_BROWSER_FORMSET_FROM_LINK (Link);
      Link = GetNextNode (&gBrowserFormSetList, Link);
      if (!IsHiiHandleInBrowserContext (LocalFormSet->HiiHandle)) {
        if (IsNvUpdateRequired(LocalFormSet)) {
          RemoveEntryList (&LocalFormSet->Link);
          DestroyFormSet (LocalFormSet);
        }
      }
    }

  return EFI_SUCCESS;
}

/**
  Submit data based on the input Setting level (Form, FormSet or System).

  @param  FormSet                FormSet data structure.
  @param  Form                   Form data structure.
  @param  SettingScope           Setting Scope for Submit action.

  @retval EFI_SUCCESS            The function completed successfully.
  @retval EFI_UNSUPPORTED        Unsupport SettingScope.

**/
EFI_STATUS
SubmitForm (
  IN FORM_BROWSER_FORMSET             *FormSet,
  IN FORM_BROWSER_FORM                *Form,
  IN BROWSER_SETTING_SCOPE            SettingScope
  )
{
  EFI_STATUS              Status;
  LIST_ENTRY              *Link;
  EFI_STRING              ConfigResp;
  EFI_STRING              Progress;
  FORMSET_STORAGE         *Storage;
  UINTN                   BufferSize;
  UINT8                   *TmpBuf;
  FORM_BROWSER_FORMSET    *LocalFormSet;
  FORM_BROWSER_CONFIG_REQUEST  *ConfigInfo;
  BYO_BROWSER_FORMSET    *ByoFormSet = NULL;
  FORM_BROWSER_FORMSET    *CurrentFormSet = NULL;

  //
  // Check the supported setting level.
  //
  if (SettingScope >= MaxLevel) {
    return EFI_UNSUPPORTED;
  }

  if (NULL != gCurrentFormSetLink) {
    ByoFormSet = BYO_FORM_BROWSER_FORMSET_FROM_LINK (gCurrentFormSetLink);
  }
		
  //
  // Validate the Form by NoSubmit check
  //
  Status = EFI_SUCCESS;
  if (SettingScope == FormLevel) {
    Status = NoSubmitCheck (FormSet, Form);
  } else if (SettingScope == FormSetLevel) {
    Status = NoSubmitCheck (FormSet, NULL);
  }
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (SettingScope == FormLevel && Form->NvUpdateRequired) {
    ConfigInfo = NULL;
    Link = GetFirstNode (&Form->ConfigRequestHead);
    while (!IsNull (&Form->ConfigRequestHead, Link)) {
      ConfigInfo = FORM_BROWSER_CONFIG_REQUEST_FROM_LINK (Link);
      Link = GetNextNode (&Form->ConfigRequestHead, Link);

      Storage = ConfigInfo->Storage;
      if (Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE) {
        continue;
      }

      //
      // Skip if there is no RequestElement
      //
      if (ConfigInfo->ElementCount == 0) {
        continue;
      }
      //
      // 1. Prepare <ConfigResp>
      //
      Status = StorageToConfigResp (ConfigInfo, &ConfigResp, TRUE);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      //
      // 2. Set value to hii driver or efi variable.
      //
      if (Storage->Type == EFI_HII_VARSTORE_BUFFER ||
          Storage->Type == EFI_HII_VARSTORE_NAME_VALUE) {
        //
        // Send <ConfigResp> to Configuration Driver
        //
        if (FormSet->ConfigAccess != NULL) {
          Status = FormSet->ConfigAccess->RouteConfig (
                                            FormSet->ConfigAccess,
                                            ConfigResp,
                                            &Progress
                                            );
          if (EFI_ERROR (Status)) {
            FreePool (ConfigResp);
            return Status;
          }
        }
      } else if (Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER) {
        TmpBuf = NULL;
        TmpBuf = AllocateZeroPool(Storage->Size);
        if (TmpBuf == NULL) {
          Status = EFI_OUT_OF_RESOURCES;
          return Status;
        }

        BufferSize = Storage->Size;
        Status = gRT->GetVariable (
                         Storage->Name,
                         &Storage->Guid,
                         NULL,
                         &BufferSize,
                         TmpBuf
                         );
        if (EFI_ERROR (Status)) {
          FreePool (TmpBuf);
          FreePool (ConfigResp);
          return Status;
        }
        ASSERT (BufferSize == Storage->Size);
        Status = mHiiConfigRouting->ConfigToBlock (
                                      mHiiConfigRouting,
                                      ConfigResp,
                                      TmpBuf,
                                      &BufferSize,
                                      &Progress
                                      );
        if (EFI_ERROR (Status)) {
          FreePool (TmpBuf);
          FreePool (ConfigResp);
          return Status;
        }

        Status = gRT->SetVariable (
                         Storage->Name,
                         &Storage->Guid,
                         Storage->Attributes,
                         Storage->Size,
                         TmpBuf
                         );
        FreePool (TmpBuf);
        if (EFI_ERROR (Status)) {
          FreePool (ConfigResp);
          return Status;
        }
      }
      FreePool (ConfigResp);
      //
      // 3. Config success, update storage shadow Buffer, only update the data belong to this form.
      //
      SynchronizeStorageForForm(FormSet, ConfigInfo, TRUE);
    }

    //
    // 4. Update the NV flag.
    //
    Form->NvUpdateRequired = FALSE;
  } else if (SettingScope == FormSetLevel && IsNvUpdateRequired(FormSet)) {
    //
    // Submit Buffer storage or Name/Value storage
    //
    Link = GetFirstNode (&FormSet->StorageListHead);
    while (!IsNull (&FormSet->StorageListHead, Link)) {
      Storage = FORMSET_STORAGE_FROM_LINK (Link);
      Link = GetNextNode (&FormSet->StorageListHead, Link);

      if (Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE) {
        continue;
      }

      //
      // Skip if there is no RequestElement
      //
      if (Storage->ElementCount == 0) {
        continue;
      }

      //
      // 1. Prepare <ConfigResp>
      //
      Status = StorageToConfigResp (Storage, &ConfigResp, FALSE);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      if (Storage->Type == EFI_HII_VARSTORE_BUFFER ||
          Storage->Type == EFI_HII_VARSTORE_NAME_VALUE) {

        //
        // 2. Send <ConfigResp> to Configuration Driver
        //
        if (FormSet->ConfigAccess != NULL) {
          Status = FormSet->ConfigAccess->RouteConfig (
                                            FormSet->ConfigAccess,
                                            ConfigResp,
                                            &Progress
                                            );
          if (EFI_ERROR (Status)) {
            FreePool (ConfigResp);
            return Status;
          }
        }
      } else if (Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER) {
        //
        // 1&2. Set the edit data to the variable.
        //
        TmpBuf = NULL;
        TmpBuf = AllocateZeroPool (Storage->Size);
        if (TmpBuf == NULL) {
          Status = EFI_OUT_OF_RESOURCES;
          return Status;
        }
        BufferSize = Storage->Size;
        Status = gRT->GetVariable (
                       Storage->Name,
                       &Storage->Guid,
                       NULL,
                       &BufferSize,
                       TmpBuf
                       );
        ASSERT (BufferSize == Storage->Size);
        Status = mHiiConfigRouting->ConfigToBlock (
                                      mHiiConfigRouting,
                                      ConfigResp,
                                      TmpBuf,
                                      &BufferSize,
                                      &Progress
                                      );
        if (EFI_ERROR (Status)) {
          FreePool (TmpBuf);
          FreePool (ConfigResp);
          return Status;
        }

        Status = gRT->SetVariable (
                         Storage->Name,
                         &Storage->Guid,
                         Storage->Attributes,
                         Storage->Size,
                         TmpBuf
                         );
        if (EFI_ERROR (Status)) {
          FreePool (TmpBuf);
          FreePool (ConfigResp);
          return Status;
        }
        FreePool (TmpBuf);
      }
      FreePool (ConfigResp);
      //
      // 3. Config success, update storage shadow Buffer
      //
      SynchronizeStorage (Storage, TRUE);
    }

    //
    // 4. Update the NV flag.
    //
    UpdateNvInfoInForm (FormSet, FALSE);
  } else if (SettingScope == SystemLevel) {
    //
    // System Level Save.
    //

    //
    // Save changed value for each FormSet in the maintain list.
    //
    Link = GetFirstNode (&gBrowserFormSetList);
    while (!IsNull (&gBrowserFormSetList, Link)) {
      LocalFormSet = FORM_BROWSER_FORMSET_FROM_LINK (Link);

      if (CompareGuid(&ByoFormSet->Guid, &LocalFormSet->Guid)) {
        //
        // Skip Current Formset.
        //        
        CurrentFormSet = LocalFormSet;
      } else {
        SubmitForm (LocalFormSet, NULL, FormSetLevel);
      }
	  
      Link = GetNextNode (&gBrowserFormSetList, Link);
    }
    //
    //Save Current Formset.
    //
    if (NULL != CurrentFormSet) {
      SubmitForm (CurrentFormSet, NULL, FormSetLevel);
    }
  }

  return EFI_SUCCESS;
}

/**
  Get Question default value from AltCfg string.

  @param  FormSet                The form set.
  @param  Question               The question.
  @param  DefaultId              The default Id.

  @retval EFI_SUCCESS            Question is reset to default value.

**/
EFI_STATUS
GetDefaultValueFromAltCfg (
  IN     FORM_BROWSER_FORMSET             *FormSet,
  IN OUT FORM_BROWSER_STATEMENT           *Question,
  IN     UINT16                           DefaultId
  )
{
  BOOLEAN             IsBufferStorage;
  BOOLEAN             IsString;
  UINTN               Length;
  FORMSET_STORAGE     *Storage;
  CHAR16              *ConfigRequest;
  CHAR16              *Progress;
  CHAR16              *Result;
  CHAR16              *ConfigResp;
  CHAR16              *Value;
  CHAR16              *StringPtr;
  UINTN               LengthStr;
  UINT8               *Dst;
  CHAR16              TemStr[5];
  UINTN               Index;
  UINT8               DigitUint8;
  EFI_STATUS          Status;

  Status        = EFI_NOT_FOUND;
  Length        = 0;
  Dst           = NULL;
  ConfigRequest = NULL;
  Result        = NULL;
  ConfigResp    = NULL;
  Value         = NULL;
  Storage       = Question->Storage;

  if ((Storage == NULL) ||
      (Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE) ||
      (Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER)) {
    return Status;
  }

  //
  // Question Value is provided by Buffer Storage or NameValue Storage
  //
  if (Question->BufferValue != NULL) {
    //
    // This Question is password or orderedlist
    //
    Dst = Question->BufferValue;
  } else {
    //
    // Other type of Questions
    //
    Dst = (UINT8 *) &Question->HiiValue.Value;
  }

  IsBufferStorage = (BOOLEAN) ((Storage->Type == EFI_HII_VARSTORE_BUFFER) ? TRUE : FALSE);
  IsString = (BOOLEAN) ((Question->HiiValue.Type == EFI_IFR_TYPE_STRING) ?  TRUE : FALSE);

  //
  // <ConfigRequest> ::= <ConfigHdr> + <BlockName> ||
  //                   <ConfigHdr> + "&" + <VariableName>
  //
  if (IsBufferStorage) {
    Length  = StrLen (Storage->ConfigHdr);
    Length += StrLen (Question->BlockName);
  } else {
    Length  = StrLen (Storage->ConfigHdr);
    Length += StrLen (Question->VariableName) + 1;
  }
  ConfigRequest = AllocateZeroPool ((Length + 1) * sizeof (CHAR16));
  ASSERT (ConfigRequest != NULL);

  StrCpy (ConfigRequest, Storage->ConfigHdr);
  if (IsBufferStorage) {
    StrCat (ConfigRequest, Question->BlockName);
  } else {
    StrCat (ConfigRequest, L"&");
    StrCat (ConfigRequest, Question->VariableName);
  }

  Status = FormSet->ConfigAccess->ExtractConfig (
                                    FormSet->ConfigAccess,
                                    ConfigRequest,
                                    &Progress,
                                    &Result
                                    );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  //
  // Call ConfigRouting GetAltCfg(ConfigRoute, <ConfigResponse>, Guid, Name, DevicePath, AltCfgId, AltCfgResp)
  //    Get the default configuration string according to the default ID.
  //
  Status = mHiiConfigRouting->GetAltConfig (
                                mHiiConfigRouting,
                                Result,
                                &Storage->Guid,
                                Storage->Name,
                                NULL,
                                &DefaultId,  // it can be NULL to get the current setting.
                                &ConfigResp
                              );

  //
  // The required setting can't be found. So, it is not required to be validated and set.
  //
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  //
  // Skip <ConfigRequest>
  //
  if (IsBufferStorage) {
    Value = StrStr (ConfigResp, L"&VALUE");
    ASSERT (Value != NULL);
    //
    // Skip "&VALUE"
    //
    Value = Value + 6;
  } else {
    Value = StrStr (ConfigResp, Question->VariableName);
    ASSERT (Value != NULL);

    Value = Value + StrLen (Question->VariableName);
  }
  if (*Value != '=') {
    Status = EFI_NOT_FOUND;
    goto Done;
  }
  //
  // Skip '=', point to value
  //
  Value = Value + 1;

  //
  // Suppress <AltResp> if any
  //
  StringPtr = Value;
  while (*StringPtr != L'\0' && *StringPtr != L'&') {
    StringPtr++;
  }
  *StringPtr = L'\0';

  LengthStr = StrLen (Value);
  if (!IsBufferStorage && IsString) {
    StringPtr = (CHAR16 *) Dst;
    ZeroMem (TemStr, sizeof (TemStr));
    for (Index = 0; Index < LengthStr; Index += 4) {
      StrnCpy (TemStr, Value + Index, 4);
      StringPtr[Index/4] = (CHAR16) StrHexToUint64 (TemStr);
    }
    //
    // Add tailing L'\0' character
    //
    StringPtr[Index/4] = L'\0';
  } else {
    ZeroMem (TemStr, sizeof (TemStr));
    for (Index = 0; Index < LengthStr; Index ++) {
      TemStr[0] = Value[LengthStr - Index - 1];
      DigitUint8 = (UINT8) StrHexToUint64 (TemStr);
      if ((Index & 1) == 0) {
        Dst [Index/2] = DigitUint8;
      } else {
        Dst [Index/2] = (UINT8) ((DigitUint8 << 4) + Dst [Index/2]);
      }
    }
  }

Done:
  if (ConfigRequest != NULL){
    FreePool (ConfigRequest);
  }

  if (ConfigResp != NULL) {
    FreePool (ConfigResp);
  }

  if (Result != NULL) {
    FreePool (Result);
  }

  return Status;
}

/**
  Get default Id value used for browser.

  @param  DefaultId              The default id value used by hii.

  @retval Browser used default value.

**/
INTN
GetDefaultIdForCallBack (
  UINTN DefaultId
  )
{
  if (DefaultId == EFI_HII_DEFAULT_CLASS_STANDARD) {
    return EFI_BROWSER_ACTION_DEFAULT_STANDARD;
  } else if (DefaultId == EFI_HII_DEFAULT_CLASS_MANUFACTURING) {
    return EFI_BROWSER_ACTION_DEFAULT_MANUFACTURING;
  } else if (DefaultId == EFI_HII_DEFAULT_CLASS_SAFE) {
    return EFI_BROWSER_ACTION_DEFAULT_SAFE;
  } else if (DefaultId >= EFI_HII_DEFAULT_CLASS_PLATFORM_BEGIN && DefaultId < EFI_HII_DEFAULT_CLASS_PLATFORM_BEGIN + 0x1000) {
    return EFI_BROWSER_ACTION_DEFAULT_PLATFORM + DefaultId - EFI_HII_DEFAULT_CLASS_PLATFORM_BEGIN;
  } else if (DefaultId >= EFI_HII_DEFAULT_CLASS_HARDWARE_BEGIN && DefaultId < EFI_HII_DEFAULT_CLASS_HARDWARE_BEGIN + 0x1000) {
    return EFI_BROWSER_ACTION_DEFAULT_HARDWARE + DefaultId - EFI_HII_DEFAULT_CLASS_HARDWARE_BEGIN;
  } else if (DefaultId >= EFI_HII_DEFAULT_CLASS_FIRMWARE_BEGIN && DefaultId < EFI_HII_DEFAULT_CLASS_FIRMWARE_BEGIN + 0x1000) {
    return EFI_BROWSER_ACTION_DEFAULT_FIRMWARE + DefaultId - EFI_HII_DEFAULT_CLASS_FIRMWARE_BEGIN;
  } else {
    return -1;
  }
}

/**
  Reset Question to its default value.

  @param  FormSet                The form set.
  @param  Form                   The form.
  @param  Question               The question.
  @param  DefaultId              The Class of the default.

  @retval EFI_SUCCESS            Question is reset to default value.

**/
EFI_STATUS
GetQuestionDefault (
  IN FORM_BROWSER_FORMSET             *FormSet,
  IN FORM_BROWSER_FORM                *Form,
  IN FORM_BROWSER_STATEMENT           *Question,
  IN UINT16                           DefaultId
  )
{
  EFI_STATUS              Status;
  LIST_ENTRY              *Link;
  QUESTION_DEFAULT        *Default;
  QUESTION_OPTION         *Option;
  EFI_HII_VALUE           *HiiValue;
  UINT8                   Index;
  EFI_STRING              StrValue;
  EFI_HII_CONFIG_ACCESS_PROTOCOL  *ConfigAccess;
  EFI_BROWSER_ACTION_REQUEST      ActionRequest;
  INTN                            Action;

  Status   = EFI_NOT_FOUND;
  StrValue = NULL;

  //
  // Statement don't have storage, skip them
  //
  if (Question->QuestionId == 0) {
    return Status;
  }

  //
  // There are Five ways to specify default value for a Question:
  //  1, use call back function (highest priority)
  //  2, use ExtractConfig function
  //  3, use nested EFI_IFR_DEFAULT
  //  4, set flags of EFI_ONE_OF_OPTION (provide Standard and Manufacturing default)
  //  5, set flags of EFI_IFR_CHECKBOX (provide Standard and Manufacturing default) (lowest priority)
  //
  HiiValue = &Question->HiiValue;

  //
  // Get Question defaut value from call back function.
  //
  ConfigAccess = FormSet->ConfigAccess;
  Action = GetDefaultIdForCallBack (DefaultId);
  if ((Action > 0) && ((Question->QuestionFlags & EFI_IFR_FLAG_CALLBACK) != 0) && (ConfigAccess != NULL)) {
    ActionRequest = EFI_BROWSER_ACTION_REQUEST_NONE;
    Status = ConfigAccess->Callback (
                             ConfigAccess,
                             Action,
                             Question->QuestionId,
                             HiiValue->Type,
                             &HiiValue->Value,
                             &ActionRequest
                             );
    if (!EFI_ERROR (Status)) {
      return Status;
    }
  }

  //
  // Get default value from altcfg string.
  //
  if (ConfigAccess != NULL) {
    Status = GetDefaultValueFromAltCfg(FormSet, Question, DefaultId);
    if (!EFI_ERROR (Status)) {
        return Status;
    }
  }

  //
  // EFI_IFR_DEFAULT has highest priority
  //
  if (!IsListEmpty (&Question->DefaultListHead)) {
    Link = GetFirstNode (&Question->DefaultListHead);
    while (!IsNull (&Question->DefaultListHead, Link)) {
      Default = QUESTION_DEFAULT_FROM_LINK (Link);

      if (Default->DefaultId == DefaultId) {
        if (Default->ValueExpression != NULL) {
          //
          // Default is provided by an Expression, evaluate it
          //
          Status = EvaluateExpression (FormSet, Form, Default->ValueExpression);
          if (EFI_ERROR (Status)) {
            return Status;
          }

          if (Default->ValueExpression->Result.Type == EFI_IFR_TYPE_BUFFER) {
            ASSERT (HiiValue->Type == EFI_IFR_TYPE_BUFFER && Question->BufferValue != NULL);
            if (Question->StorageWidth > Default->ValueExpression->Result.BufferLen) {
              CopyMem (Question->HiiValue.Buffer, Default->ValueExpression->Result.Buffer, Default->ValueExpression->Result.BufferLen);
              Question->HiiValue.BufferLen = Default->ValueExpression->Result.BufferLen;
            } else {
              CopyMem (Question->HiiValue.Buffer, Default->ValueExpression->Result.Buffer, Question->StorageWidth);
              Question->HiiValue.BufferLen = Question->StorageWidth;
            }
            FreePool (Default->ValueExpression->Result.Buffer);
          }
          HiiValue->Type = Default->ValueExpression->Result.Type;
          CopyMem (&HiiValue->Value, &Default->ValueExpression->Result.Value, sizeof (EFI_IFR_TYPE_VALUE));
        } else {
          //
          // Default value is embedded in EFI_IFR_DEFAULT
          //
          CopyMem (HiiValue, &Default->Value, sizeof (EFI_HII_VALUE));
        }

        if (HiiValue->Type == EFI_IFR_TYPE_STRING) {
          StrValue = HiiGetString (FormSet->HiiHandle, HiiValue->Value.string, NULL);
          if (StrValue == NULL) {
            return EFI_NOT_FOUND;
          }
          if (Question->StorageWidth > StrSize (StrValue)) {
            CopyMem (Question->BufferValue, StrValue, StrSize (StrValue));
          } else {
            CopyMem (Question->BufferValue, StrValue, Question->StorageWidth);
          }
        }

        return EFI_SUCCESS;
      }

      Link = GetNextNode (&Question->DefaultListHead, Link);
    }
  }

  //
  // EFI_ONE_OF_OPTION
  //
  if ((Question->Operand == EFI_IFR_ONE_OF_OP) && !IsListEmpty (&Question->OptionListHead)) {
    if (DefaultId <= EFI_HII_DEFAULT_CLASS_MANUFACTURING)  {
      //
      // OneOfOption could only provide Standard and Manufacturing default
      //
      Link = GetFirstNode (&Question->OptionListHead);
      while (!IsNull (&Question->OptionListHead, Link)) {
        Option = QUESTION_OPTION_FROM_LINK (Link);

        if (((DefaultId == EFI_HII_DEFAULT_CLASS_STANDARD) && ((Option->Flags & EFI_IFR_OPTION_DEFAULT) != 0)) ||
            ((DefaultId == EFI_HII_DEFAULT_CLASS_MANUFACTURING) && ((Option->Flags & EFI_IFR_OPTION_DEFAULT_MFG) != 0))
           ) {
          CopyMem (HiiValue, &Option->Value, sizeof (EFI_HII_VALUE));

          return EFI_SUCCESS;
        }

        Link = GetNextNode (&Question->OptionListHead, Link);
      }
    }
  }

  //
  // EFI_IFR_CHECKBOX - lowest priority
  //
  if (Question->Operand == EFI_IFR_CHECKBOX_OP) {
    if (DefaultId <= EFI_HII_DEFAULT_CLASS_MANUFACTURING)  {
      //
      // Checkbox could only provide Standard and Manufacturing default
      //
      if (((DefaultId == EFI_HII_DEFAULT_CLASS_STANDARD) && ((Question->Flags & EFI_IFR_CHECKBOX_DEFAULT) != 0)) ||
          ((DefaultId == EFI_HII_DEFAULT_CLASS_MANUFACTURING) && ((Question->Flags & EFI_IFR_CHECKBOX_DEFAULT_MFG) != 0))
         ) {
        HiiValue->Value.b = TRUE;
      } else {
        HiiValue->Value.b = FALSE;
      }

      return EFI_SUCCESS;
    }
  }

  //
  // For Questions without default
  //
  Status = EFI_NOT_FOUND;
  switch (Question->Operand) {
  case EFI_IFR_NUMERIC_OP:
    //
    // Take minimum value as numeric default value
    //
    if ((HiiValue->Value.u64 < Question->Minimum) || (HiiValue->Value.u64 > Question->Maximum)) {
      HiiValue->Value.u64 = Question->Minimum;
      Status = EFI_SUCCESS;
    }
    break;

  case EFI_IFR_ONE_OF_OP:
    //
    // Take first oneof option as oneof's default value
    //
    if (ValueToOption (Question, HiiValue) == NULL) {
      Link = GetFirstNode (&Question->OptionListHead);
      if (!IsNull (&Question->OptionListHead, Link)) {
        Option = QUESTION_OPTION_FROM_LINK (Link);
        CopyMem (HiiValue, &Option->Value, sizeof (EFI_HII_VALUE));
        Status = EFI_SUCCESS;
      }
    }
    break;

  case EFI_IFR_ORDERED_LIST_OP:
    //
    // Take option sequence in IFR as ordered list's default value
    //
    Index = 0;
    Link = GetFirstNode (&Question->OptionListHead);
    while (!IsNull (&Question->OptionListHead, Link)) {
      Status = EFI_SUCCESS;
      Option = QUESTION_OPTION_FROM_LINK (Link);

      SetArrayData (Question->BufferValue, Question->ValueType, Index, Option->Value.Value.u64);

      Index++;
      if (Index >= Question->MaxContainers) {
        break;
      }

      Link = GetNextNode (&Question->OptionListHead, Link);
    }
    break;

  default:
    break;
  }

  return Status;
}


/**
  Reset Questions to their initial value or default value in a Form, Formset or System.

  GetDefaultValueScope parameter decides which questions will reset
  to its default value.

  @param  FormSet                FormSet data structure.
  @param  Form                   Form data structure.
  @param  DefaultId              The Class of the default.
  @param  SettingScope           Setting Scope for Default action.
  @param  GetDefaultValueScope   Get default value scope.
  @param  Storage                Get default value only for this storage.
  @param  RetrieveValueFirst     Whether call the retrieve call back to
                                 get the initial value before get default
                                 value.

  @retval EFI_SUCCESS            The function completed successfully.
  @retval EFI_UNSUPPORTED        Unsupport SettingScope.

**/
EFI_STATUS
ExtractDefault (
  IN FORM_BROWSER_FORMSET             *FormSet,
  IN FORM_BROWSER_FORM                *Form,
  IN UINT16                           DefaultId,
  IN BROWSER_SETTING_SCOPE            SettingScope,
  IN BROWSER_GET_DEFAULT_VALUE        GetDefaultValueScope,
  IN FORMSET_STORAGE                  *Storage OPTIONAL,
  IN BOOLEAN                          RetrieveValueFirst
  )
{
  EFI_STATUS              Status;
  LIST_ENTRY              *FormLink;
  LIST_ENTRY              *Link;
  FORM_BROWSER_STATEMENT  *Question;
  FORM_BROWSER_FORMSET    *LocalFormSet;

  Status = EFI_SUCCESS;
  //
  // Check the supported setting level.
  //
  if (SettingScope >= MaxLevel || GetDefaultValueScope >= GetDefaultForMax) {
    return EFI_UNSUPPORTED;
  }

  if (GetDefaultValueScope == GetDefaultForStorage && Storage == NULL) {
    return EFI_UNSUPPORTED;
  }


  if (SettingScope == FormLevel) {
    //
    // Extract Form default
    //
    Link = GetFirstNode (&Form->StatementListHead);
    while (!IsNull (&Form->StatementListHead, Link)) {
      Question = FORM_BROWSER_STATEMENT_FROM_LINK (Link);
      Link = GetNextNode (&Form->StatementListHead, Link);

      //
      // If get default value only for this storage, check the storage first.
      //
      if ((GetDefaultValueScope == GetDefaultForStorage) && (Question->Storage != Storage)) {
        continue;
      }

      //
      // If get default value only for no storage question, just skip the question which has storage.
      //
      if ((GetDefaultValueScope == GetDefaultForNoStorage) && (Question->Storage != NULL)) {
        continue;
      }

      //
      // If Question is disabled, don't reset it to default
      //
      if (Question->Expression != NULL) {
        if (EvaluateExpressionList(Question->Expression, TRUE, FormSet, Form) == ExpressDisable) {
          continue;
        }
      }

      if (RetrieveValueFirst) {
        //
        // Call the Retrieve call back to get the initial question value.
        //
        Status = ProcessRetrieveForQuestion(FormSet->ConfigAccess, Question);
      }

      //
      // If not request to get the initial value or get initial value fail, then get default value.
      //
      if (!RetrieveValueFirst || EFI_ERROR (Status)) {
        Status = GetQuestionDefault (FormSet, Form, Question, DefaultId);
        if (EFI_ERROR (Status)) {
          continue;
        }
      }

      //
      // Synchronize Buffer storage's Edit buffer
      //
      if ((Question->Storage != NULL) &&
          (Question->Storage->Type != EFI_HII_VARSTORE_EFI_VARIABLE)) {
        SetQuestionValue (FormSet, Form, Question, GetSetValueWithEditBuffer);
        //
        // Update Form NV flag.
        //
        Form->NvUpdateRequired = TRUE;
      } else if ((Question->Operand == EFI_IFR_ONE_OF_OP) &&
        (Question->Storage == NULL) &&
        ((Question->QuestionFlags & EFI_IFR_FLAG_CALLBACK) != 0) ) {
        // only for dynamic created One-of.
        SetQuestionValue (FormSet, Form, Question, GetSetValueWithEditBuffer);
      }
    }

  } else if (SettingScope == FormSetLevel) {
    FormLink = GetFirstNode (&FormSet->FormListHead);
    while (!IsNull (&FormSet->FormListHead, FormLink)) {
      Form = FORM_BROWSER_FORM_FROM_LINK (FormLink);
      ExtractDefault (FormSet, Form, DefaultId, FormLevel, GetDefaultValueScope, Storage, RetrieveValueFirst);
      FormLink = GetNextNode (&FormSet->FormListHead, FormLink);
    }
  } else if (SettingScope == SystemLevel) {

    //
    // Preload all Hii formset.
    //
    LoadAllHiiFormset();
    //
    // Set Default Value for each FormSet in the maintain list.
    //
    Link = GetFirstNode (&gBrowserFormSetList);
    while (!IsNull (&gBrowserFormSetList, Link)) {
      LocalFormSet = FORM_BROWSER_FORMSET_FROM_LINK (Link);
      ExtractDefault (LocalFormSet, NULL, DefaultId, FormSetLevel, GetDefaultValueScope, Storage, RetrieveValueFirst);
      Link = GetNextNode (&gBrowserFormSetList, Link);
    }
  }

  return EFI_SUCCESS;
}

/**
  Initialize Question's Edit copy from Storage.

  @param  Selection              Selection contains the information about
                                 the Selection, form and formset to be displayed.
                                 Selection action may be updated in retrieve callback.
                                 If Selection is NULL, only initialize Question value.
  @param  FormSet                FormSet data structure.
  @param  Form                   Form data structure.

  @retval EFI_SUCCESS            The function completed successfully.

**/
EFI_STATUS
LoadFormConfig (
  IN OUT UI_MENU_SELECTION    *Selection,
  IN FORM_BROWSER_FORMSET     *FormSet,
  IN FORM_BROWSER_FORM        *Form
  )
{
  EFI_STATUS                  Status;
  LIST_ENTRY                  *Link;
  FORM_BROWSER_STATEMENT      *Question;
  UINT8                       *BufferValue;
  UINTN                       StorageWidth;

  Link = GetFirstNode (&Form->StatementListHead);
  while (!IsNull (&Form->StatementListHead, Link)) {
    Question = FORM_BROWSER_STATEMENT_FROM_LINK (Link);

    //
    // Initialize local copy of Value for each Question
    //
    Status = GetQuestionValue (FormSet, Form, Question, GetSetValueWithEditBuffer);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    if ((Question->Operand == EFI_IFR_STRING_OP) || (Question->Operand == EFI_IFR_PASSWORD_OP)) {
      HiiSetString (FormSet->HiiHandle, Question->HiiValue.Value.string, (CHAR16*)Question->BufferValue, NULL);
    }

    //
    // Call the Retrieve call back function for all questions.
    //
    if ((FormSet->ConfigAccess != NULL) && (Selection != NULL) &&
        ((Question->QuestionFlags & EFI_IFR_FLAG_CALLBACK) == EFI_IFR_FLAG_CALLBACK)) {
      //
      // Check QuestionValue does exist.
      //
      StorageWidth = Question->StorageWidth;
      if (Question->BufferValue != NULL) {
        BufferValue  = Question->BufferValue;
      } else {
        BufferValue = (UINT8 *) &Question->HiiValue.Value;
      }

      //
      // For efivarstore storage, initial question value first.
      //
      if ((Question->Storage != NULL) && (Question->Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE)) {
        Status = gRT->GetVariable (
                         Question->VariableName,
                         &Question->Storage->Guid,
                         NULL,
                         &StorageWidth,
                         BufferValue
                         );
      }

      Status = ProcessCallBackFunction(Selection, Question, EFI_BROWSER_ACTION_RETRIEVE, TRUE);
    }

    Link = GetNextNode (&Form->StatementListHead, Link);
  }

  return EFI_SUCCESS;
}

/**
  Initialize Question's Edit copy from Storage for the whole Formset.

  @param  Selection              Selection contains the information about
                                 the Selection, form and formset to be displayed.
                                 Selection action may be updated in retrieve callback.
                                 If Selection is NULL, only initialize Question value.
  @param  FormSet                FormSet data structure.

  @retval EFI_SUCCESS            The function completed successfully.

**/
EFI_STATUS
LoadFormSetConfig (
  IN OUT UI_MENU_SELECTION    *Selection,
  IN     FORM_BROWSER_FORMSET *FormSet
  )
{
  EFI_STATUS            Status;
  LIST_ENTRY            *Link;
  FORM_BROWSER_FORM     *Form;

  Link = GetFirstNode (&FormSet->FormListHead);
  while (!IsNull (&FormSet->FormListHead, Link)) {
    Form = FORM_BROWSER_FORM_FROM_LINK (Link);

    //
    // Initialize local copy of Value for each Form
    //
    Status = LoadFormConfig (Selection, FormSet, Form);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Link = GetNextNode (&FormSet->FormListHead, Link);
  }

  return EFI_SUCCESS;
}

/**
  Fill storage's edit copy with settings requested from Configuration Driver.

  @param  FormSet                FormSet data structure.
  @param  Storage                Buffer Storage.

  @retval EFI_SUCCESS            The function completed successfully.

**/
EFI_STATUS
LoadStorage (
  IN FORM_BROWSER_FORMSET    *FormSet,
  IN FORMSET_STORAGE         *Storage
  )
{
  EFI_STATUS  Status;
  EFI_STRING  Progress;
  EFI_STRING  Result;
  CHAR16      *StrPtr;

  if (Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE) {
    return EFI_SUCCESS;
  }

  if (Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER) {
    Status = gRT->GetVariable (
                     Storage->Name,
                     &Storage->Guid,
                     NULL,
                     (UINTN*)&Storage->Size,
                     Storage->EditBuffer
                     );
    return Status;
  }

  if (FormSet->ConfigAccess == NULL) {
    return EFI_NOT_FOUND;
  }

  if (Storage->ElementCount == 0) {
    //
    // Skip if there is no RequestElement
    //
    return EFI_SUCCESS;
  }

  //
  // Request current settings from Configuration Driver
  //
  Status = FormSet->ConfigAccess->ExtractConfig (
                                    FormSet->ConfigAccess,
                                    Storage->ConfigRequest,
                                    &Progress,
                                    &Result
                                    );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Convert Result from <ConfigAltResp> to <ConfigResp>
  //
  StrPtr = StrStr (Result, L"&GUID=");
  if (StrPtr != NULL) {
    *StrPtr = L'\0';
  }

  Status = ConfigRespToStorage (Storage, Result);
  FreePool (Result);
  return Status;
}


/**
  Copy uncommitted data from source Storage to destination Storage.

  @param  Dst                    Target Storage for uncommitted data.
  @param  Src                    Source Storage for uncommitted data.

  @retval EFI_SUCCESS            The function completed successfully.
  @retval EFI_INVALID_PARAMETER  Source and destination Storage is not the same type.

**/
EFI_STATUS
CopyStorage (
  IN OUT FORMSET_STORAGE     *Dst,
  IN FORMSET_STORAGE         *Src
  )
{
  LIST_ENTRY          *Link;
  NAME_VALUE_NODE     *Node;

  if ((Dst->Type != Src->Type) || (Dst->Size != Src->Size)) {
    return EFI_INVALID_PARAMETER;
  }

  switch (Src->Type) {
  case EFI_HII_VARSTORE_BUFFER:
  case EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER:
    CopyMem (Dst->EditBuffer, Src->EditBuffer, Src->Size);
    CopyMem (Dst->Buffer, Src->Buffer, Src->Size);
    break;

  case EFI_HII_VARSTORE_NAME_VALUE:
    Link = GetFirstNode (&Src->NameValueListHead);
    while (!IsNull (&Src->NameValueListHead, Link)) {
      Node = NAME_VALUE_NODE_FROM_LINK (Link);

      SetValueByName (Dst, Node->Name, Node->EditValue, GetSetValueWithEditBuffer);
      SetValueByName (Dst, Node->Name, Node->Value, GetSetValueWithBuffer);

      Link = GetNextNode (&Src->NameValueListHead, Link);
    }
    break;

  case EFI_HII_VARSTORE_EFI_VARIABLE:
  default:
    break;
  }

  return EFI_SUCCESS;
}


/**
  Get current setting of Questions.

  @param  FormSet                FormSet data structure.

  @retval EFI_SUCCESS            The function completed successfully.

**/
EFI_STATUS
InitializeCurrentSetting (
  IN OUT FORM_BROWSER_FORMSET             *FormSet
  )
{
  LIST_ENTRY              *Link;
  LIST_ENTRY              *Link2;
  FORMSET_STORAGE         *Storage;
  FORMSET_STORAGE         *StorageSrc;
  FORMSET_STORAGE         *OldStorage;
  FORM_BROWSER_FORM       *Form;
  FORM_BROWSER_FORM       *Form2;
  EFI_STATUS              Status;

  //
  // Extract default from IFR binary for no storage questions.
  //
  ExtractDefault (FormSet, NULL, EFI_HII_DEFAULT_CLASS_STANDARD, FormSetLevel, GetDefaultForNoStorage, NULL, TRUE);

  //
  // Request current settings from Configuration Driver
  //
  Link = GetFirstNode (&FormSet->StorageListHead);
  while (!IsNull (&FormSet->StorageListHead, Link)) {
    Storage = FORMSET_STORAGE_FROM_LINK (Link);

    OldStorage = NULL;
    if (gOldFormSet != NULL) {
      //
      // Try to find the Storage in backup formset gOldFormSet
      //
      Link2 = GetFirstNode (&gOldFormSet->StorageListHead);
      while (!IsNull (&gOldFormSet->StorageListHead, Link2)) {
        StorageSrc = FORMSET_STORAGE_FROM_LINK (Link2);

        if (StorageSrc->VarStoreId == Storage->VarStoreId) {
          OldStorage = StorageSrc;
          break;
        }

        Link2 = GetNextNode (&gOldFormSet->StorageListHead, Link2);
      }
    }

    if (OldStorage == NULL) {
      //
      // Storage is not found in backup formset, request it from ConfigDriver
      //
      Status = LoadStorage (FormSet, Storage);

      if (EFI_ERROR (Status)) {
        //
        // If get last time changed value failed, extract default from IFR binary
        //
        ExtractDefault (FormSet, NULL, EFI_HII_DEFAULT_CLASS_STANDARD, FormSetLevel, GetDefaultForStorage, Storage, TRUE);
        //
        // ExtractDefault will set the NV flag to TRUE, so need this function to clean the flag
        // in current situation.
        //
        UpdateNvInfoInForm (FormSet, FALSE);

      }

      //
      // Now Edit Buffer is filled with default values(lower priority) or current
      // settings(higher priority), sychronize it to shadow Buffer
      //
      if (!EFI_ERROR (Status)) {
        SynchronizeStorage (Storage, TRUE);
      }
    } else {
      //
      // Storage found in backup formset, use it
      //
      Status = CopyStorage (Storage, OldStorage);
    }

    Link = GetNextNode (&FormSet->StorageListHead, Link);
  }

  //
  // If has old formset, get the old nv update status.
  //
  if (gOldFormSet != NULL) {
      Link = GetFirstNode (&FormSet->FormListHead);
      while (!IsNull (&FormSet->FormListHead, Link)) {
        Form = FORM_BROWSER_FORM_FROM_LINK (Link);

        Link2 = GetFirstNode (&gOldFormSet->FormListHead);
        while (!IsNull (&gOldFormSet->FormListHead, Link2)) {
          Form2 = FORM_BROWSER_FORM_FROM_LINK (Link2);

          if (Form->FormId == Form2->FormId) {
            Form->NvUpdateRequired = Form2->NvUpdateRequired;
            break;
          }

          Link2 = GetNextNode (&gOldFormSet->FormListHead, Link2);
        }
          Link = GetNextNode (&FormSet->FormListHead, Link);
      }
  }

  return EFI_SUCCESS;
}

/**
  Free resources of a storage.

  @param  Storage                Pointer of the storage

**/
VOID
DestroyStorage (
  IN FORMSET_STORAGE   *Storage
  );

VOID
RemoveModifiedStorage (
  VOID
  )
{
  LIST_ENTRY    *Link;
  FORMSET_STORAGE    *Storage;
	
  while (!IsListEmpty (&gModifiedStorgeList)) {
    Link = gModifiedStorgeList.ForwardLink;

    Storage = FORMSET_STORAGE_FROM_LINK (Link);
    RemoveEntryList (&Storage->Link);
    DestroyStorage (Storage);
  }

  return;
}

EFI_STATUS
BackupModifiedStorage (
  IN OUT FORM_BROWSER_FORMSET             *FormSet
  )
{
  LIST_ENTRY    *Link;
  LIST_ENTRY    *BackupLink;
  LIST_ENTRY    *FormsetStorageList;

  FORMSET_STORAGE    *FormsetStorage;
  FORMSET_STORAGE    *BackupStorage;
  FORMSET_STORAGE    *NewStorage;

  if (NULL == FormSet || (!IsNvUpdateRequired(FormSet))) {
    return EFI_NOT_FOUND;
  }

  FormsetStorageList = &FormSet->StorageListHead;
  Link = GetFirstNode (FormsetStorageList);
  while (!IsNull (FormsetStorageList, Link)) {
    FormsetStorage = FORMSET_STORAGE_FROM_LINK (Link);
    Link = GetNextNode (FormsetStorageList, Link);

    if (FormsetStorage->ElementCount == 0) {
      continue;
    }
    //
    // Look for and remove old strorage.
    //
    BackupLink = GetFirstNode (&gModifiedStorgeList);
    while (!IsNull (&gModifiedStorgeList, BackupLink)) {
      BackupStorage = FORMSET_STORAGE_FROM_LINK (BackupLink);

      if (BackupStorage->VarStoreId == FormsetStorage->VarStoreId) {
        RemoveEntryList (&BackupStorage->Link);
        DestroyStorage (BackupStorage);
        break;
      }
      BackupLink = GetNextNode (&gModifiedStorgeList, BackupLink);
    }

    //
    // Copy modifed strorage and insert to Backup list.
    //
    NewStorage = AllocateCopyPool (sizeof (FORMSET_STORAGE), FormsetStorage);
    if (NewStorage != NULL) {
      NewStorage->Signature = FORMSET_STORAGE_SIGNATURE;
      InitializeListHead (&NewStorage->NameValueListHead);
      NewStorage->Buffer = AllocateZeroPool (FormsetStorage->Size);
      NewStorage->EditBuffer = AllocateZeroPool (FormsetStorage->Size);
      CopyStorage (NewStorage, FormsetStorage);

      NewStorage->Name = NULL;
      NewStorage->ConfigRequest = NULL;
      NewStorage->ConfigHdr = NULL;
      NewStorage->SpareStrLen = 0;	  
      InsertTailList (&gModifiedStorgeList, &NewStorage->Link);
    }
  }

  return EFI_SUCCESS;
}

/**
  Synchronize Storage with OldFormset.

  @param  FormSet                FormSet data structure.

  @retval EFI_SUCCESS            The function completed successfully.

**/
EFI_STATUS
SynchronizeWithOldFormset(
  IN OUT FORM_BROWSER_FORMSET             *FormSet
  )
{
  LIST_ENTRY              *Link;
  LIST_ENTRY              *Link2;
  LIST_ENTRY              *BackupLink;
  FORMSET_STORAGE         *Storage;
  FORMSET_STORAGE         *StorageSrc;
  FORMSET_STORAGE         *OldStorage;
  FORMSET_STORAGE         *BackupStorage;
  EFI_STATUS              Status;

  if (gOldFormSet == NULL || gBeLoadDefaultValue) {
    return EFI_NOT_FOUND;
  }

  if (!IsListEmpty (&gModifiedStorgeList)) {
    UpdateNvInfoInForm(FormSet, TRUE);
  }else if (!IsNvUpdateRequired(gOldFormSet)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Synchronize.
  //
  Link = GetFirstNode (&FormSet->StorageListHead);
  while (!IsNull (&FormSet->StorageListHead, Link)) {
    Storage = FORMSET_STORAGE_FROM_LINK (Link);

    if (Storage->ElementCount != 0) {
      //
      // Try to find the Storage in gOldFormSet.
      //
      OldStorage = NULL;
      Link2 = GetFirstNode (&gOldFormSet->StorageListHead);
      while (!IsNull (&gOldFormSet->StorageListHead, Link2)) {
        StorageSrc = FORMSET_STORAGE_FROM_LINK (Link2);
        if (StorageSrc->ElementCount != 0) {	  
          if (StorageSrc->VarStoreId == Storage->VarStoreId) {
            OldStorage = StorageSrc;
            break;
          }
        }
        Link2 = GetNextNode (&gOldFormSet->StorageListHead, Link2);
      }
        
      if (OldStorage != NULL ) {
        Status = CopyStorage (Storage, OldStorage);
        UpdateNvInfoInForm(FormSet, TRUE);
      } else {
        //
        // Need check backup storage list.
        //
        BackupLink = GetFirstNode (&gModifiedStorgeList);
        while (!IsNull (&gModifiedStorgeList, BackupLink)) {
          BackupStorage = FORMSET_STORAGE_FROM_LINK (BackupLink);
          if (BackupStorage->VarStoreId == Storage->VarStoreId) {
            Status = CopyStorage (Storage, BackupStorage);
            UpdateNvInfoInForm(FormSet, TRUE);
            DEBUG((EFI_D_ERROR, "jawen ____, SynchronizeWithOldFormset(), with backup storage:%s-%d.\n", Storage->Name, Storage->VarStoreId));
            break;
          }
          BackupLink = GetNextNode (&gModifiedStorgeList, BackupLink);
        }
      }
    }

    Link = GetNextNode (&FormSet->StorageListHead, Link);
  }

  //
  // Backup strorage from gOldFormSet.
  //
  BackupModifiedStorage (gOldFormSet);

  return EFI_SUCCESS;
}

/**
  Fetch the Ifr binary data of a FormSet.

  @param  Handle                 PackageList Handle
  @param  FormSetGuid            On input, GUID or class GUID of a formset. If not
                                 specified (NULL or zero GUID), take the first
                                 FormSet with class GUID EFI_HII_PLATFORM_SETUP_FORMSET_GUID
                                 found in package list.
                                 On output, GUID of the formset found(if not NULL).
  @param  BinaryLength           The length of the FormSet IFR binary.
  @param  BinaryData             The buffer designed to receive the FormSet.

  @retval EFI_SUCCESS            Buffer filled with the requested FormSet.
                                 BufferLength was updated.
  @retval EFI_INVALID_PARAMETER  The handle is unknown.
  @retval EFI_NOT_FOUND          A form or FormSet on the requested handle cannot
                                 be found with the requested FormId.

**/
EFI_STATUS
GetIfrBinaryData (
  IN  EFI_HII_HANDLE   Handle,
  IN OUT EFI_GUID      *FormSetGuid,
  OUT UINTN            *BinaryLength,
  OUT UINT8            **BinaryData
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *HiiPackageList;
  UINTN                        BufferSize;
  UINT8                        *Package;
  UINT8                        *OpCodeData;
  UINT32                       Offset;
  UINT32                       Offset2;
  UINT32                       PackageListLength;
  EFI_HII_PACKAGE_HEADER       PackageHeader;
  UINT8                        Index;
  UINT8                        NumberOfClassGuid;
  BOOLEAN                      ClassGuidMatch;
  EFI_GUID                     *ClassGuid;
  EFI_GUID                     *ComparingGuid;

  OpCodeData = NULL;
  Package = NULL;
  ZeroMem (&PackageHeader, sizeof (EFI_HII_PACKAGE_HEADER));
  //
  // if FormSetGuid is NULL or zero GUID, return first Setup FormSet in the package list
  //
  if (FormSetGuid == NULL) {
    ComparingGuid = &gZeroGuid;
  } else {
    ComparingGuid = FormSetGuid;
  }

  //
  // Get HII PackageList
  //
  BufferSize = 0;
  HiiPackageList = NULL;
  Status = mHiiDatabase->ExportPackageLists (mHiiDatabase, Handle, &BufferSize, HiiPackageList);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    HiiPackageList = AllocatePool (BufferSize);
    ASSERT (HiiPackageList != NULL);

    Status = mHiiDatabase->ExportPackageLists (mHiiDatabase, Handle, &BufferSize, HiiPackageList);
  }
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ASSERT (HiiPackageList != NULL);

  //
  // Get Form package from this HII package List
  //
  Offset = sizeof (EFI_HII_PACKAGE_LIST_HEADER);
  Offset2 = 0;
  CopyMem (&PackageListLength, &HiiPackageList->PackageLength, sizeof (UINT32));

  ClassGuidMatch = FALSE;
  while (Offset < PackageListLength) {
    Package = ((UINT8 *) HiiPackageList) + Offset;
    CopyMem (&PackageHeader, Package, sizeof (EFI_HII_PACKAGE_HEADER));

    if (PackageHeader.Type == EFI_HII_PACKAGE_FORMS) {
      //
      // Search FormSet in this Form Package
      //
      Offset2 = sizeof (EFI_HII_PACKAGE_HEADER);
      while (Offset2 < PackageHeader.Length) {
        OpCodeData = Package + Offset2;

        if (((EFI_IFR_OP_HEADER *) OpCodeData)->OpCode == EFI_IFR_FORM_SET_OP) {
          //
          // Try to compare against formset GUID
          //
          if (CompareGuid (FormSetGuid, &gZeroGuid) ||
              CompareGuid (ComparingGuid, (EFI_GUID *)(OpCodeData + sizeof (EFI_IFR_OP_HEADER)))) {
            break;
          }

          if (((EFI_IFR_OP_HEADER *) OpCodeData)->Length > OFFSET_OF (EFI_IFR_FORM_SET, Flags)) {
            //
            // Try to compare against formset class GUID
            //
            NumberOfClassGuid = (UINT8) (((EFI_IFR_FORM_SET *) OpCodeData)->Flags & 0x3);
            ClassGuid         = (EFI_GUID *) (OpCodeData + sizeof (EFI_IFR_FORM_SET));
            for (Index = 0; Index < NumberOfClassGuid; Index++) {
              if (CompareGuid (ComparingGuid, ClassGuid + Index)) {
                ClassGuidMatch = TRUE;
                break;
              }
            }
            if (ClassGuidMatch) {
              break;
            }
          } else if (ComparingGuid == &gEfiHiiPlatformSetupFormsetGuid) {
            ClassGuidMatch = TRUE;
            break;
          }
        }

        Offset2 += ((EFI_IFR_OP_HEADER *) OpCodeData)->Length;
      }

      if (Offset2 < PackageHeader.Length) {
        //
        // Target formset found
        //
        break;
      }
    }

    Offset += PackageHeader.Length;
  }

  if (Offset >= PackageListLength) {
    //
    // Form package not found in this Package List
    //
    FreePool (HiiPackageList);
    return EFI_NOT_FOUND;
  }

  if (FormSetGuid != NULL) {
    //
    // Return the FormSet GUID
    //
    CopyMem (FormSetGuid, &((EFI_IFR_FORM_SET *) OpCodeData)->Guid, sizeof (EFI_GUID));
  }

  //
  // To determine the length of a whole FormSet IFR binary, one have to parse all the Opcodes
  // in this FormSet; So, here just simply copy the data from start of a FormSet to the end
  // of the Form Package.
  //
  *BinaryLength = PackageHeader.Length - Offset2;
  *BinaryData = AllocateCopyPool (*BinaryLength, OpCodeData);

  FreePool (HiiPackageList);

  if (*BinaryData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}


/**
  Initialize the internal data structure of a FormSet.

  @param  Handle                 PackageList Handle
  @param  FormSetGuid            On input, GUID or class GUID of a formset. If not
                                 specified (NULL or zero GUID), take the first
                                 FormSet with class GUID EFI_HII_PLATFORM_SETUP_FORMSET_GUID
                                 found in package list.
                                 On output, GUID of the formset found(if not NULL).
  @param  FormSet                FormSet data structure.
  @param  UpdateGlobalVar        Whether need to update the global variable.

  @retval EFI_SUCCESS            The function completed successfully.
  @retval EFI_NOT_FOUND          The specified FormSet could not be found.

**/
EFI_STATUS
InitializeFormSet (
  IN  EFI_HII_HANDLE                   Handle,
  IN OUT EFI_GUID                      *FormSetGuid,
  OUT FORM_BROWSER_FORMSET             *FormSet,
  IN  BOOLEAN                          UpdateGlobalVar
  )
{
  EFI_STATUS                Status;
  EFI_HANDLE                DriverHandle;
  UINT16                    Index;

  Status = GetIfrBinaryData (Handle, FormSetGuid, &FormSet->IfrBinaryLength, &FormSet->IfrBinaryData);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  FormSet->Signature = FORM_BROWSER_FORMSET_SIGNATURE;
  FormSet->HiiHandle = Handle;
  CopyMem (&FormSet->Guid, FormSetGuid, sizeof (EFI_GUID));

  //
  // Retrieve ConfigAccess Protocol associated with this HiiPackageList
  //
  Status = mHiiDatabase->GetPackageListHandle (mHiiDatabase, Handle, &DriverHandle);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  FormSet->DriverHandle = DriverHandle;
  Status = gBS->HandleProtocol (
                  DriverHandle,
                  &gEfiHiiConfigAccessProtocolGuid,
                  (VOID **) &FormSet->ConfigAccess
                  );
  if (EFI_ERROR (Status)) {
    //
    // Configuration Driver don't attach ConfigAccess protocol to its HII package
    // list, then there will be no configuration action required
    //
    FormSet->ConfigAccess = NULL;
  }

  //
  // Parse the IFR binary OpCodes
  //
  Status = ParseOpCodes (FormSet);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // If not need to update the global variable, just return.
  //
  if (!UpdateGlobalVar) {
    return Status;
  }

  RefreshFormsetVariable(FormSet);

  //
  // Match GUID to find out the function key setting. If match fail, use the default setting.
  //
  for (Index = 0; Index < sizeof (gFunctionKeySettingTable) / sizeof (FUNCTIION_KEY_SETTING); Index++) {
    if (CompareGuid (&FormSet->Guid, &(gFunctionKeySettingTable[Index].FormSetGuid))) {
      //gFunctionKeySetting = gFunctionKeySettingTable[Index].KeySetting;
    }
  }

  return EFI_SUCCESS;
}


/**
  Save globals used by previous call to SendForm(). SendForm() may be called from
  HiiConfigAccess.Callback(), this will cause SendForm() be reentried.
  So, save globals of previous call to SendForm() and restore them upon exit.

**/
VOID
SaveBrowserContext (
  VOID
  )
{
  BROWSER_CONTEXT  *Context;

  gBrowserContextCount++;
  if (gBrowserContextCount == 1) {
    //
    // This is not reentry of SendForm(), no context to save
    //
    return;
  }

  Context = AllocatePool (sizeof (BROWSER_CONTEXT));
  ASSERT (Context != NULL);

  Context->Signature = BROWSER_CONTEXT_SIGNATURE;

  //
  // Save FormBrowser context
  //
  Context->BannerData           = gBannerData;
  Context->ClassOfVfr           = gClassOfVfr;
  Context->FunctionKeySetting   = gFunctionKeySetting;
  Context->ResetRequired        = gResetRequired;
  Context->Direction            = gDirection;
  Context->EnterString          = gEnterString;
  Context->EnterCommitString    = gEnterCommitString;
  Context->EnterEscapeString    = gEnterEscapeString;
  Context->EscapeString         = gEscapeString;
  Context->MoveHighlight        = gMoveHighlight;
  Context->MakeSelection        = gMakeSelection;
  Context->DecNumericInput      = gDecNumericInput;
  Context->HexNumericInput      = gHexNumericInput;
  Context->ToggleCheckBox       = gToggleCheckBox;
  Context->PromptForData        = gPromptForData;
  Context->PromptForPassword    = gPromptForPassword;
  Context->PromptForNewPassword = gPromptForNewPassword;
  Context->ConfirmPassword      = gConfirmPassword;
  Context->ConfirmError         = gConfirmError;
  Context->PassowordInvalid     = gPassowordInvalid;
  Context->PressEnter           = gPressEnter;
  Context->EmptyString          = gEmptyString;
  Context->AreYouSure           = gAreYouSure;
  Context->YesResponse          = gYesResponse;
  Context->NoResponse           = gNoResponse;
  Context->MiniString           = gMiniString;
  Context->PlusString           = gPlusString;
  Context->MinusString          = gMinusString;
  Context->AdjustNumber         = gAdjustNumber;
  Context->SaveChanges          = gSaveChanges;
  Context->OptionMismatch       = gOptionMismatch;
  Context->FormSuppress         = gFormSuppress;
  Context->PromptBlockWidth     = gPromptBlockWidth;
  Context->OptionBlockWidth     = gOptionBlockWidth;
  Context->HelpBlockWidth       = gHelpBlockWidth;
  Context->OldFormSet           = gOldFormSet;
  Context->MenuRefreshHead      = gMenuRefreshHead;
  Context->FunctionNineString   = gFunctionNineString;
  Context->FunctionTenString    = gFunctionTenString;
  Context->SaveFailed           = gSaveFailed;

  CopyMem (&Context->ScreenDimensions, &gScreenDimensions, sizeof (gScreenDimensions));
  CopyMem (&Context->MenuOption, &gMenuOption, sizeof (gMenuOption));

  //
  // Insert to FormBrowser context list
  //
  InsertHeadList (&gBrowserContextList, &Context->Link);
}


/**
  Restore globals used by previous call to SendForm().

**/
VOID
RestoreBrowserContext (
  VOID
  )
{
  LIST_ENTRY       *Link;
  BROWSER_CONTEXT  *Context;

  ASSERT (gBrowserContextCount != 0);
  gBrowserContextCount--;
  if (gBrowserContextCount == 0) {
    //
    // This is not reentry of SendForm(), no context to restore
    //
    return;
  }

  ASSERT (!IsListEmpty (&gBrowserContextList));

  Link = GetFirstNode (&gBrowserContextList);
  Context = BROWSER_CONTEXT_FROM_LINK (Link);

  //
  // Restore FormBrowser context
  //
  gBannerData           = Context->BannerData;
  gClassOfVfr           = Context->ClassOfVfr;
  gFunctionKeySetting   = Context->FunctionKeySetting;
  gResetRequired        = Context->ResetRequired;
  gDirection            = Context->Direction;
  gEnterString          = Context->EnterString;
  gEnterCommitString    = Context->EnterCommitString;
  gEnterEscapeString    = Context->EnterEscapeString;
  gEscapeString         = Context->EscapeString;
  gMoveHighlight        = Context->MoveHighlight;
  gMakeSelection        = Context->MakeSelection;
  gDecNumericInput      = Context->DecNumericInput;
  gHexNumericInput      = Context->HexNumericInput;
  gToggleCheckBox       = Context->ToggleCheckBox;
  gPromptForData        = Context->PromptForData;
  gPromptForPassword    = Context->PromptForPassword;
  gPromptForNewPassword = Context->PromptForNewPassword;
  gConfirmPassword      = Context->ConfirmPassword;
  gConfirmError         = Context->ConfirmError;
  gPassowordInvalid     = Context->PassowordInvalid;
  gPressEnter           = Context->PressEnter;
  gEmptyString          = Context->EmptyString;
  gAreYouSure           = Context->AreYouSure;
  gYesResponse          = Context->YesResponse;
  gNoResponse           = Context->NoResponse;
  gMiniString           = Context->MiniString;
  gPlusString           = Context->PlusString;
  gMinusString          = Context->MinusString;
  gAdjustNumber         = Context->AdjustNumber;
  gSaveChanges          = Context->SaveChanges;
  gOptionMismatch       = Context->OptionMismatch;
  gFormSuppress         = Context->FormSuppress;
  gPromptBlockWidth     = Context->PromptBlockWidth;
  gOptionBlockWidth     = Context->OptionBlockWidth;
  gHelpBlockWidth       = Context->HelpBlockWidth;
  gOldFormSet           = Context->OldFormSet;
  gMenuRefreshHead      = Context->MenuRefreshHead;
  gFunctionNineString   = Context->FunctionNineString;
  gFunctionTenString    = Context->FunctionTenString;
  gSaveFailed           = Context->SaveFailed;

  CopyMem (&gScreenDimensions, &Context->ScreenDimensions, sizeof (gScreenDimensions));
  CopyMem (&gMenuOption, &Context->MenuOption, sizeof (gMenuOption));

  //
  // Remove from FormBrowser context list
  //
  RemoveEntryList (&Context->Link);
  gBS->FreePool (Context);
}

/**
  Find the matched FormSet context in the backup maintain list based on HiiHandle.

  @param Handle  The Hii Handle.

  @return the found FormSet context. If no found, NULL will return.

**/
FORM_BROWSER_FORMSET *
GetFormSetFromHiiHandle (
  EFI_HII_HANDLE Handle
  )
{
  LIST_ENTRY           *Link;
  FORM_BROWSER_FORMSET *FormSet;
	  
  if (IsListEmpty(&gBrowserFormSetList)) {
    return NULL;
  }
	  
  Link = GetFirstNode (&gBrowserFormSetList);
  while (!IsNull (&gBrowserFormSetList, Link)) {
    FormSet = FORM_BROWSER_FORMSET_FROM_LINK (Link);
    if (FormSet->HiiHandle == Handle) {
      return FormSet;
    }
    Link = GetNextNode (&gBrowserFormSetList, Link);
  }

  return NULL;
}

/**
  Check whether formset is in gByoFormSetList.

**/
BOOLEAN
IsByoMainFormset (
  EFI_HII_HANDLE Handle
  )
{
  LIST_ENTRY           *Link;
  BYO_BROWSER_FORMSET   *FormSet;

  if (IsListEmpty(gByoFormSetList)) {
    return TRUE;
  }

  Link = GetFirstNode (gByoFormSetList);
  while (!IsNull (gByoFormSetList, Link)) {
    FormSet = BYO_FORM_BROWSER_FORMSET_FROM_LINK (Link);
    if (FormSet->HiiHandle == Handle) {
      return TRUE;
    }
    Link = GetNextNode (gByoFormSetList, Link);
  }
  return FALSE;
}

/**
  Check whether the input HII handle is the FormSet that is being used.

  @param Handle  The Hii Handle.

  @retval TRUE   HII handle is being used.
  @retval FALSE  HII handle is not being used.

**/
BOOLEAN
IsHiiHandleInBrowserContext (
  EFI_HII_HANDLE Handle
  )
{
  LIST_ENTRY       *Link;
  BROWSER_CONTEXT  *Context;
  BYO_BROWSER_FORMSET   *FormSet;
  //
  // HiiHandle is Current FormSet.
  //
  if ((gOldFormSet != NULL) && (gOldFormSet->HiiHandle == Handle)) {
    return TRUE;
  }

  //
  //Check Current Byo Formset.
  //
  if (NULL != gCurrentFormSetLink) {
    FormSet = BYO_FORM_BROWSER_FORMSET_FROM_LINK (gCurrentFormSetLink);
    if (FormSet->HiiHandle == Handle) {
      return TRUE;
    }
  }
  
  //
  // Check whether HiiHandle is in BrowserContext.
  //
  Link = GetFirstNode (&gBrowserContextList);
  while (!IsNull (&gBrowserContextList, Link)) {
    Context = BROWSER_CONTEXT_FROM_LINK (Link);
    if (Context->OldFormSet->HiiHandle == Handle) {
      //
      // HiiHandle is in BrowserContext
      //
      return TRUE;
    }
    Link = GetNextNode (&gBrowserContextList, Link);
  }

  return FALSE;
}

/**
  Find the registered HotKey based on KeyData.

  @param[in] KeyData     A pointer to a buffer that describes the keystroke
                         information for the hot key.

  @return The registered HotKey context. If no found, NULL will return.
**/
BROWSER_HOT_KEY *
GetHotKeyFromRegisterList (
  IN EFI_INPUT_KEY *KeyData
  )
{
  LIST_ENTRY       *Link;
  BROWSER_HOT_KEY  *HotKey;

  Link = GetFirstNode (&gBrowserHotKeyList);
  while (!IsNull (&gBrowserHotKeyList, Link)) {
    HotKey = BROWSER_HOT_KEY_FROM_LINK (Link);
    if (HotKey->KeyData->ScanCode == KeyData->ScanCode) {
      return HotKey;
    }
    Link = GetNextNode (&gBrowserHotKeyList, Link);
  }

  return NULL;
}

/**
  Configure what scope the hot key will impact.
  All hot keys have the same scope. The mixed hot keys with the different level are not supported.
  If no scope is set, the default scope will be FormSet level.
  After all registered hot keys are removed, previous Scope can reset to another level.

  @param[in] Scope               Scope level to be set.

  @retval EFI_SUCCESS            Scope is set correctly.
  @retval EFI_INVALID_PARAMETER  Scope is not the valid value specified in BROWSER_SETTING_SCOPE.
  @retval EFI_UNSPPORTED         Scope level is different from current one that the registered hot keys have.

**/
EFI_STATUS
EFIAPI
SetScope (
  IN BROWSER_SETTING_SCOPE Scope
  )
{
  if (Scope >= MaxLevel) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // When no hot key registered in system or on the first setting,
  // Scope can be set.
  //
  if (mBrowserScopeFirstSet || IsListEmpty (&gBrowserHotKeyList)) {
    gBrowserSettingScope  = Scope;
    mBrowserScopeFirstSet = FALSE;
  } else if (Scope != gBrowserSettingScope) {
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

/**
  Register the hot key with its browser action, or unregistered the hot key.
  Only support hot key that is not printable character (control key, function key, etc.).
  If the action value is zero, the hot key will be unregistered if it has been registered.
  If the same hot key has been registered, the new action and help string will override the previous ones.

  @param[in] KeyData     A pointer to a buffer that describes the keystroke
                         information for the hot key. Its type is EFI_INPUT_KEY to
                         be supported by all ConsoleIn devices.
  @param[in] Action      Action value that describes what action will be trigged when the hot key is pressed.
  @param[in] DefaultId   Specifies the type of defaults to retrieve, which is only for DEFAULT action.
  @param[in] HelpString  Help string that describes the hot key information.
                         Its value may be NULL for the unregistered hot key.

  @retval EFI_SUCCESS            Hot key is registered or unregistered.
  @retval EFI_INVALID_PARAMETER  KeyData is NULL or HelpString is NULL on register.
  @retval EFI_NOT_FOUND          KeyData is not found to be unregistered.
  @retval EFI_UNSUPPORTED        Key represents a printable character. It is conflicted with Browser.
**/
EFI_STATUS
EFIAPI
RegisterHotKey (
  IN EFI_INPUT_KEY *KeyData,
  IN UINT32        Action,
  IN UINT16        DefaultId,
  IN EFI_STRING    HelpString OPTIONAL
  )
{
  BROWSER_HOT_KEY  *HotKey;

  //
  // Check input parameters.
  //
  if (KeyData == NULL || KeyData->UnicodeChar != CHAR_NULL ||
     (Action != BROWSER_ACTION_UNREGISTER && HelpString == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check whether the input KeyData is in BrowserHotKeyList.
  //
  HotKey = GetHotKeyFromRegisterList (KeyData);

  //
  // Unregister HotKey
  //
  if (Action == BROWSER_ACTION_UNREGISTER) {
    if (HotKey != NULL) {
      //
      // The registered HotKey is found.
      // Remove it from List, and free its resource.
      //
      RemoveEntryList (&HotKey->Link);
      FreePool (HotKey->KeyData);
      FreePool (HotKey->HelpString);
      return EFI_SUCCESS;
    } else {
      //
      // The registered HotKey is not found.
      //
      return EFI_NOT_FOUND;
    }
  }

  //
  // Register HotKey into List.
  //
  if (HotKey == NULL) {
    //
    // Create new Key, and add it into List.
    //
    HotKey = AllocateZeroPool (sizeof (BROWSER_HOT_KEY));
    ASSERT (HotKey != NULL);
    HotKey->Signature = BROWSER_HOT_KEY_SIGNATURE;
    HotKey->KeyData   = AllocateCopyPool (sizeof (EFI_INPUT_KEY), KeyData);
    InsertTailList (&gBrowserHotKeyList, &HotKey->Link);
  }

  //
  // Fill HotKey information.
  //
  HotKey->Action     = Action;
  HotKey->DefaultId  = DefaultId;
  if (HotKey->HelpString != NULL) {
    FreePool (HotKey->HelpString);
  }
  HotKey->HelpString = AllocateCopyPool (StrSize (HelpString), HelpString);

  return EFI_SUCCESS;
}

/**
  Register Exit handler function.
  When more than one handler function is registered, the latter one will override the previous one.
  When NULL handler is specified, the previous Exit handler will be unregistered.

  @param[in] Handler      Pointer to handler function.

**/
VOID
EFIAPI
RegiserExitHandler (
  IN EXIT_HANDLER Handler
  )
{
  ExitHandlerFunction = Handler;
  return;
}

/**
  Check whether the browser data has been modified.

  @retval TRUE        Browser data is modified.
  @retval FALSE       No browser data is modified.

**/
BOOLEAN
EFIAPI
IsBrowserDataModified (
  VOID
  )
{
  LIST_ENTRY              *Link;
  FORM_BROWSER_FORMSET    *FormSet;

  if (gCurrentSelection == NULL) {
    return FALSE;
  }

  switch (gBrowserSettingScope) {
    case FormLevel:
      return gCurrentSelection->Form->NvUpdateRequired;

    case FormSetLevel:
      return IsNvUpdateRequired (gCurrentSelection->FormSet);

    case SystemLevel:
      Link = GetFirstNode (&gBrowserFormSetList);
      while (!IsNull (&gBrowserFormSetList, Link)) {
        FormSet = FORM_BROWSER_FORMSET_FROM_LINK (Link);
        if (IsNvUpdateRequired (FormSet)) {
          return TRUE;
        }
        Link = GetNextNode (&gBrowserFormSetList, Link);
      }
      return FALSE;

    default:
      return FALSE;
  }
}

/**
  Execute the action requested by the Action parameter.

  @param[in] Action     Execute the request action.
  @param[in] DefaultId  The default Id info when need to load default value. Only used when Action is BROWSER_ACTION_DEFAULT.

  @retval EFI_SUCCESS              Execute the request action succss.
  @retval EFI_INVALID_PARAMETER    The input action value is invalid.

**/
EFI_STATUS
EFIAPI
ExecuteAction (
  IN UINT32        Action,
  IN UINT16        DefaultId
  )
{
  EFI_STATUS    Status;
  CHAR16    *Title;

  if (gCurrentSelection == NULL) {
    return EFI_NOT_READY;
  }
  Status = EFI_SUCCESS;
  //
  // Executet the discard action.
  //
  if ((Action & BROWSER_ACTION_DISCARD) != 0) {

    if ((Action & BROWSER_ACTION_EXIT) != 0) {
      Title = gAreYouSureExitWithOutSaving;
    } else {
      Title = gAreYouSureDiscardChanges;
    }

    if (CreateConfirmDialog (NULL, 3, gEmptyString, Title, gEmptyString)) {
      Status = DiscardForm (gCurrentSelection->FormSet, gCurrentSelection->Form, gBrowserSettingScope);
      RemoveModifiedStorage ();	  
      gBeLoadDefaultValue = FALSE;
      LoadAllHiiFormset();
	  
      if (EFI_ERROR (Status)) {
        return Status;
      }
    } else {
      Status = EFI_NOT_READY;
      return Status;
    }
  }

  //
  // Executet the default action.
  //
  if ((Action & BROWSER_ACTION_DEFAULT) != 0) {
    if (CreateConfirmDialog (NULL, 3, gEmptyString, gAreYouSureLoadDefault, gEmptyString)) {
      mSynchronizeWithOldFormset = FALSE;

      if (NULL != gOldFormSet){
        Status = ExtractDefault (gOldFormSet, NULL, 0, FormSetLevel, GetDefaultForAll,NULL, FALSE);
        gOldFormSet = NULL;
      }
      Status = ExtractDefault (gCurrentSelection->FormSet, gCurrentSelection->Form, DefaultId, gBrowserSettingScope, GetDefaultForAll, NULL, FALSE);
      RemoveModifiedStorage ();
      gBeLoadDefaultValue = TRUE;

      if (EFI_ERROR (Status)) {
        return Status;
      }
    } else {
      Status = EFI_NOT_READY;
      return Status;
    }
  }

  if ((Action & BROWSER_ACTION_SAVE_USER_DEFAULT) != 0) {
    if (CreateConfirmDialog (NULL, 3, gEmptyString, gAreYouSureSaveUserDefault, gEmptyString)) {
      RemoveModifiedStorage ();
      gBeLoadDefaultValue = FALSE;
      LoadAllHiiFormset();
      Status = EFI_SUCCESS;
    } else {
      Status = EFI_NOT_READY;
    }
    return Status;
  }

  if ((Action & BROWSER_ACTION_LOAD_USER_DEFAULT) != 0) {
    if (CreateConfirmDialog (NULL, 3, gEmptyString, gAreYouSureLoadUserDefault, gEmptyString)) {
      UpdateAllFormsetNvInfo(TRUE);
      RemoveModifiedStorage();
      Status = EFI_SUCCESS;
    } else {
      Status = EFI_NOT_READY;
    }
    return Status;
  }

  //
  // Executet the submit action.
  //
  if ((Action & BROWSER_ACTION_SUBMIT) != 0) {

    if ((Action & BROWSER_ACTION_EXIT) != 0) {
      Title = gAreYouSureExitWithSaving;
    } else {
      Title = gAreYouSureSaveChanges;
    }

    if (CreateConfirmDialog (NULL, 3, gEmptyString, Title, gEmptyString)) {
      Status = SubmitForm (gCurrentSelection->FormSet, gCurrentSelection->Form, gBrowserSettingScope);
      RemoveModifiedStorage ();
      gBeLoadDefaultValue = FALSE;
      if (EFI_ERROR (Status)) {
        return Status;
      }
    } else {
      Status = EFI_NOT_READY;
      return Status;
    }
  }

  //
  // Executet the reset action.
  //
  if ((Action & BROWSER_ACTION_RESET) != 0) {
    gResetRequired = TRUE;
  }

  //
  // Executet the exit action.
  //
  if ((Action & BROWSER_ACTION_EXIT) != 0) {
    if (gBrowserSettingScope == SystemLevel) {
      if (ExitHandlerFunction != NULL) {
        DEBUG((EFI_D_ERROR, "ExecuteAction(), ExitHandlerFunction(),\n"));
        ExitHandlerFunction ();
      }
    }
    gExitRequired = TRUE;
  }

  return Status;
}

/**
  This is only fake function that will be made in platform setup.
**/
VOID
FakeOemPlatformReset (VOID)
{
  DEBUG((EFI_D_ERROR, "FakeOemPlatformReset(), This is a EfiResetCold, please make new function in plafform setup.\n"));
  gRT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);
  return;
}
