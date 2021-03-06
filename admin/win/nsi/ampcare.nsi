;ownCloud installer script.

!define APPLICATION_SHORTNAME "ampcare"
!define APPLICATION_NAME "AMPcare"
!define APPLICATION_VENDOR "Awesome Technologies Innovationslabor GmbH"
!define APPLICATION_EXECUTABLE "ampcare.exe"
!define APPLICATION_CMD_EXECUTABLE "ampcare.exe"
!define APPLICATION_DOMAIN "amp.care"
!define APPLICATION_LICENSE ""
!define WIN_SETUP_BITMAP_PATH "$%PROJECT_PATH%\desktop\admin\win\nsi"
!define CRASHREPORTER_EXECUTABLE "ampcare_crash_reporter"
!define BUILD_PLATFORM "$%ARCH%"

;-----------------------------------------------------------------------------
; Some installer script options (comment-out options not required)
;-----------------------------------------------------------------------------
!if "" != ""
  !define OPTION_LICENSE_AGREEMENT
!endif
!define OPTION_UAC_PLUGIN_ENHANCED
;!define OPTION_SECTION_SC_SHELL_EXT
!define OPTION_SECTION_SC_START_MENU
!define OPTION_SECTION_SC_DESKTOP
!define OPTION_SECTION_SC_QUICK_LAUNCH
!define OPTION_FINISHPAGE
!define OPTION_FINISHPAGE_LAUNCHER

;-----------------------------------------------------------------------------
; Some paths.
;-----------------------------------------------------------------------------
!define QT_PATH  "$%QT_PATH%"
!define QT_DLL_PATH "${QT_PATH}\bin"
!define SQLITE_DLL_PATH "${QT_PATH}/plugins/sqldrivers"
!define IMAGEFORMATS_DLL_PATH "${QT_PATH}/plugins/imageformats"
!define PLATFORMS_DLL_PATH "${QT_PATH}/plugins/platforms"
!define PROJECT_PATH "$%PROJECT_PATH%"
!define BUILD_PATH "${PROJECT_PATH}\desktop\build"
!define LIBS_PATH "${PROJECT_PATH}\libs"
!define INSTALL_PATH "${PROJECT_PATH}\install"
!define SOURCE_PATH "${PROJECT_PATH}\desktop"
!define OPENSSL10_PATH "$%OPENSSL10_PATH%"
!define OPENSSL11_PATH "$%OPENSSL11_PATH%"
!define OPENSSL11_LIBCRYPTO_PATH "$%OPENSSL11_LIBCRYPTO_PATH%"
!define OPENSSL_DLL_PATH "$%OPENSSL_DLL_PATH%"

!define CSYNC_LIBRARY_DIR ""
!define CSYNC_CONFIG_DIR ""

!define NSI_PATH "${SOURCE_PATH}/admin/win/nsi"

;-----------------------------------------------------------------------------
; Installer version
;-----------------------------------------------------------------------------

!getdllversion "${INSTALL_PATH}\bin\ampcare.exe" expv_
!define VER_MAJOR "${expv_1}"
!define VER_MINOR "${expv_2}"
!define VER_PATCH "${expv_3}"
!define VER_BUILD "${expv_4}"
!define VER_YEAR "${AMP_VERSION_YEAR}"
!define VER_MONTH "${AMP_VERSION_MONTH}"
!define VER_REV "${AMP_VERSION_REV}"
!define VERSION "${VER_YEAR}.${VER_MONTH}-${VER_REV}"
Var InstallRunIfSilent
Var NoAutomaticUpdates

;-----------------------------------------------------------------------------
; Installer build timestamp.
;-----------------------------------------------------------------------------
!define /date BUILD_TIME "${MIRALL_VERSION_SUFFIX} Built from Git revision ${GIT_REVISION} on %Y/%m/%d at %I:%M %p"
!define /date BUILD_TIME_FILENAME "%Y%m%d"

;-----------------------------------------------------------------------------
; Initial installer setup and definitions.
;-----------------------------------------------------------------------------

!define INSTALLER_FILENAME "${APPLICATION_NAME}-${VERSION}-${MIRALL_VERSION_SUFFIX}-${BUILD_TIME_FILENAME}-${BUILD_PLATFORM}.exe"
Name "AMPcare"
BrandingText "${APPLICATION_NAME} ${VERSION} - ${BUILD_TIME}"
OutFile "${PROJECT_PATH}\install\${INSTALLER_FILENAME}"
InstallDir "$PROGRAMFILES\AMPcare"
InstallDirRegKey HKCU "Software\${APPLICATION_VENDOR}\${APPLICATION_NAME}" ""
InstType Standard
InstType Full
InstType Minimal
CRCCheck On
SetCompressor /SOLID lzma
RequestExecutionLevel user ;Now using the UAC plugin.
ReserveFile NSIS.InstallOptions.ini
ReserveFile "${NSISDIR}\Plugins\x86-unicode\InstallOptions.dll"

;-----------------------------------------------------------------------------
; Include some required header files.
;-----------------------------------------------------------------------------
!include LogicLib.nsh ;Used by APPDATA uninstaller.
!include MUI2.nsh ;Used by APPDATA uninstaller.
!include InstallOptions.nsh ;Required by MUI2 to support old MUI_INSTALLOPTIONS.
!include Memento.nsh ;Remember user selections.
!include WinVer.nsh ;Windows version detection.
!include WordFunc.nsh  ;Used by VersionCompare macro function.
!include FileFunc.nsh  ;Used to read out parameters
!include UAC.nsh ;Used by the UAC elevation to install as user or admin.
!include nsProcess.nsh ;Used to kill the running process
!include Library.nsh ;Used by the COM registration for shell extensions
!include x64.nsh ;Used to determine the right arch for the shell extensions

;-----------------------------------------------------------------------------
; Memento selections stored in registry.
;-----------------------------------------------------------------------------
!define MEMENTO_REGISTRY_ROOT HKLM
!define MEMENTO_REGISTRY_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPLICATION_NAME}"

;-----------------------------------------------------------------------------
; Modern User Interface (MUI) definitions and setup.
;-----------------------------------------------------------------------------
!define MUI_ABORTWARNING
!define MUI_ICON ${NSI_PATH}\installer.ico
!define MUI_UNICON ${NSI_PATH}\installer.ico
!define MUI_WELCOMEFINISHPAGE_BITMAP ${WIN_SETUP_BITMAP_PATH}\welcome_amp.bmp
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP  ${WIN_SETUP_BITMAP_PATH}\page_header_amp.bmp
!define MUI_COMPONENTSPAGE_SMALLDESC
; We removed this, h1 issue 191687
;!define MUI_FINISHPAGE_LINK "${APPLICATION_DOMAIN}"
;!define MUI_FINISHPAGE_LINK_LOCATION "http://${APPLICATION_DOMAIN}"
!define MUI_FINISHPAGE_NOREBOOTSUPPORT
!ifdef OPTION_FINISHPAGE_RELEASE_NOTES
   !define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
   !define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\NOTES.txt"
   !define MUI_FINISHPAGE_SHOWREADME_TEXT $MUI_FINISHPAGE_SHOWREADME_TEXT_STRING
!endif
!ifdef OPTION_FINISHPAGE_LAUNCHER
   !define MUI_FINISHPAGE_NOAUTOCLOSE
   !define MUI_FINISHPAGE_RUN
   !define MUI_FINISHPAGE_RUN_FUNCTION "LaunchApplication"
!endif

;-----------------------------------------------------------------------------
; Page macros.
;-----------------------------------------------------------------------------
!insertmacro MUI_PAGE_WELCOME
!ifdef OPTION_LICENSE_AGREEMENT
   !insertmacro MUI_PAGE_LICENSE "${APPLICATION_LICENSE}"
!endif
Page custom PageReinstall PageLeaveReinstall
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!ifdef OPTION_FINISHPAGE
   !insertmacro MUI_PAGE_FINISH
!endif
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;-----------------------------------------------------------------------------
; Other MUI macros.
;-----------------------------------------------------------------------------
!insertmacro MUI_LANGUAGE "English"

!include ${source_path}\admin\win\nsi\l10n\languages.nsh
!include ${source_path}\admin\win\nsi\l10n\declarations.nsh

; Set version strings with english locale
VIProductVersion "${VER_MAJOR}.${VER_MINOR}.${VER_PATCH}.${VER_BUILD}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "${APPLICATION_NAME}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "${APPLICATION_VENDOR}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${VERSION}"

!macro SETLANG un
   Function ${un}SetLang
      # load the selected language file
      !include "${source_path}/admin/win/nsi/l10n\English.nsh"
      StrCmp $LANGUAGE ${LANG_GERMAN} German EndLanguageCmp
      German:
      !include "${source_path}/admin/win/nsi/l10n\German.nsh"
      EndLanguageCmp:
   FunctionEnd
!macroend

!insertmacro SETLANG ""
!insertmacro SETLANG "un."

; Usage: ${If} ${HasSection} SectionName
!macro _HasSection _a _b _t _f
   ReadRegDWORD $_LOGICLIB_TEMP "${MEMENTO_REGISTRY_ROOT}" "${MEMENTO_REGISTRY_KEY}" "MementoSection_${_b}"
   IntCmpU $_LOGICLIB_TEMP 0 ${_f} ${_t}
!macroend
!define HasSection `"" HasSection`

##############################################################################
#                                                                            #
#   FINISH PAGE LAUNCHER FUNCTIONS                                           #
#                                                                            #
##############################################################################

Function LaunchApplication
   !insertmacro UAC_AsUser_ExecShell "" "$INSTDIR\${APPLICATION_EXECUTABLE}" "" "" ""
FunctionEnd

##############################################################################
#                                                                            #
#   PROCESS HANDLING FUNCTIONS AND MACROS                                    #
#                                                                            #
##############################################################################

!macro CheckForProcess processName gotoWhenFound gotoWhenNotFound
   ${nsProcess::FindProcess} ${processName} $R0
   StrCmp $R0 0 ${gotoWhenFound} ${gotoWhenNotFound}
!macroend

!macro ConfirmEndProcess processName
   MessageBox MB_YESNO|MB_ICONEXCLAMATION \
     $ConfirmEndProcess_MESSAGEBOX_TEXT \
     /SD IDYES IDYES process_${processName}_kill IDNO process_${processName}_ended
   process_${processName}_kill:
      DetailPrint $ConfirmEndProcess_KILLING_PROCESSES_TEXT
      ${nsProcess::KillProcess} ${processName} $R0
      Sleep 1500
      StrCmp $R0 "1" process_${processName}_ended
      DetailPrint $ConfirmEndProcess_KILL_NOT_FOUND_TEXT
   process_${processName}_ended:
!macroend

!macro CheckAndConfirmEndProcess processName
   !insertmacro CheckForProcess ${processName} 0 no_process_${processName}_to_end
   !insertmacro ConfirmEndProcess ${processName}
   no_process_${processName}_to_end:
!macroend

Function EnsureOwncloudShutdown
   !insertmacro CheckAndConfirmEndProcess "${APPLICATION_EXECUTABLE}"
FunctionEnd

Function InstallRedistributables
   ${If} ${RunningX64}
      ExecWait '"$OUTDIR\vcredist_x64.exe" /install /quiet'
   ${Else}
      ExecWait '"$OUTDIR\vcredist_x86.exe" /install /quiet'
   ${EndIf}
   Delete "$OUTDIR\vcredist_x86.exe"
   Delete "$OUTDIR\vcredist_x64.exe"
FunctionEnd

##############################################################################
#                                                                            #
#   RE-INSTALLER FUNCTIONS                                                   #
#                                                                            #
##############################################################################

Function PageReinstall
   ReadRegStr $R0 HKLM "Software\${APPLICATION_VENDOR}\${APPLICATION_NAME}" ""
   StrCmp $R0 "" 0 +2
   Abort

   ;Detect version
   ReadRegDWORD $R0 HKLM "Software\${APPLICATION_VENDOR}\${APPLICATION_NAME}" "VersionYear"
   IntCmp $R0 ${VER_YEAR} month_check new_version older_version
   month_check:
      ReadRegDWORD $R0 HKLM "Software\${APPLICATION_VENDOR}\${APPLICATION_NAME}" "VersionMonth"
      IntCmp $R0 ${VER_MONTH} rev_check new_version older_version
   rev_check:
      ReadRegDWORD $R0 HKLM "Software\${APPLICATION_VENDOR}\${APPLICATION_NAME}" "VersionRev"
      IntCmp $R0 ${VER_REV} build_check new_version older_version
   build_check:
      ReadRegDWORD $R0 HKLM "Software\${APPLICATION_VENDOR}\${APPLICATION_NAME}" "VersionBuild"
      IntCmp $R0 ${VER_BUILD} same_version new_version older_version

   new_version:
      !insertmacro INSTALLOPTIONS_WRITE "NSIS.InstallOptions.ini" "Field 1" "Text" $PageReinstall_NEW_Field_1
      !insertmacro INSTALLOPTIONS_WRITE "NSIS.InstallOptions.ini" "Field 2" "Text" $PageReinstall_NEW_Field_2
      !insertmacro INSTALLOPTIONS_WRITE "NSIS.InstallOptions.ini" "Field 3" "Text" $PageReinstall_NEW_Field_3
      !insertmacro MUI_HEADER_TEXT $PageReinstall_NEW_MUI_HEADER_TEXT_TITLE $PageReinstall_NEW_MUI_HEADER_TEXT_SUBTITLE
      StrCpy $R0 "1"
      Goto reinst_start

   older_version:
      !insertmacro INSTALLOPTIONS_WRITE "NSIS.InstallOptions.ini" "Field 1" "Text" $PageReinstall_OLD_Field_1
      !insertmacro INSTALLOPTIONS_WRITE "NSIS.InstallOptions.ini" "Field 2" "Text" $PageReinstall_NEW_Field_2
      !insertmacro INSTALLOPTIONS_WRITE "NSIS.InstallOptions.ini" "Field 3" "Text" $PageReinstall_NEW_Field_3
      !insertmacro MUI_HEADER_TEXT $PageReinstall_NEW_MUI_HEADER_TEXT_TITLE $PageReinstall_NEW_MUI_HEADER_TEXT_SUBTITLE
      StrCpy $R0 "1"
      Goto reinst_start

   same_version:
      !insertmacro INSTALLOPTIONS_WRITE "NSIS.InstallOptions.ini" "Field 1" "Text" $PageReinstall_SAME_Field_1
      !insertmacro INSTALLOPTIONS_WRITE "NSIS.InstallOptions.ini" "Field 2" "Text" $PageReinstall_SAME_Field_2
      !insertmacro INSTALLOPTIONS_WRITE "NSIS.InstallOptions.ini" "Field 3" "Text" $PageReinstall_SAME_Field_3
      !insertmacro MUI_HEADER_TEXT $PageReinstall_NEW_MUI_HEADER_TEXT_TITLE $PageReinstall_SAME_MUI_HEADER_TEXT_SUBTITLE
      StrCpy $R0 "2"

   reinst_start:
      !insertmacro INSTALLOPTIONS_DISPLAY "NSIS.InstallOptions.ini"
FunctionEnd

Function PageLeaveReinstall
   !insertmacro INSTALLOPTIONS_READ $R1 "NSIS.InstallOptions.ini" "Field 2" "State"
   StrCmp $R0 "1" 0 +2
   StrCmp $R1 "1" reinst_uninstall reinst_done
   StrCmp $R0 "2" 0 +3
   StrCmp $R1 "1" reinst_done reinst_uninstall
   reinst_uninstall:
      ReadRegStr $R1 ${MEMENTO_REGISTRY_ROOT} "${MEMENTO_REGISTRY_KEY}" "UninstallString"
      HideWindow
      ClearErrors
      ExecWait '$R1 _?=$INSTDIR'
      IfErrors no_remove_uninstaller
      IfFileExists "$INSTDIR\${APPLICATION_EXECUTABLE}" no_remove_uninstaller
      Delete $R1
      RMDir $INSTDIR
   no_remove_uninstaller:
      StrCmp $R0 "2" 0 +3
      Quit
      BringToFront
   reinst_done:
FunctionEnd

##############################################################################
#                                                                            #
#   INSTALLER SECTIONS                                                       #
#                                                                            #
##############################################################################
Section "${APPLICATION_NAME}" SEC_APPLICATION
   SectionIn 1 2 3 RO
   SetDetailsPrint listonly

   SetDetailsPrint textonly
   DetailPrint $SEC_APPLICATION_DETAILS
   SetDetailsPrint listonly
   SetOutPath "$INSTDIR"

    File /r "${INSTALL_PATH}\bin\*"

; exclude system file list
    File /r "${INSTALL_PATH}\config\AMPcare\sync-exclude.lst"
    File "${INSTALL_PATH}\bin\ampcare\ocsync.dll"

; icon
    File /oname=ampcare.ico "${NSI_PATH}\installer.ico"

; dependencies
    File /r "${LIBS_PATH}\*"

; extra dll's
    File "${OPENSSL10_PATH}\bin\libeay32.dll"
    File "${OPENSSL10_PATH}\bin\ssleay32.dll"
    File "${OPENSSL11_LIBCRYPTO_PATH}"
    File "${OPENSSL_DLL_PATH}\msvcr120.dll"

; translations TODO put the translations under the folder translations
   SetOutPath "$INSTDIR\i18n"
    File /r "${INSTALL_PATH}\i18n\*"
SectionEnd

!ifdef OPTION_SECTION_SC_SHELL_EXT
   ${MementoSection} $OPTION_SECTION_SC_SHELL_EXT_SECTION SEC_SHELL_EXT
      SectionIn 1 2
      SetDetailsPrint textonly
      DetailPrint $OPTION_SECTION_SC_SHELL_EXT_DetailPrint
      Call InstallRedistributables
      CreateDirectory "$INSTDIR\shellext"
      !define LIBRARY_COM
      !define LIBRARY_SHELL_EXTENSION
      !define LIBRARY_IGNORE_VERSION
      ${If} ${RunningX64}
         !define LIBRARY_X64
         !insertmacro InstallLib DLL NOTSHARED REBOOT_PROTECTED "${SOURCE_PATH}\binary\shell_integration\windows\Release\x64\OCUtil_x64.dll" "$INSTDIR\shellext\OCUtil_x64.dll" "$INSTDIR\shellext"
         !insertmacro InstallLib REGDLL NOTSHARED REBOOT_PROTECTED "${SOURCE_PATH}\binary\shell_integration\windows\Release\x64\OCOverlays_x64.dll" "$INSTDIR\shellext\OCOverlays_x64.dll" "$INSTDIR\shellext"
         !insertmacro InstallLib REGDLL NOTSHARED REBOOT_PROTECTED "${SOURCE_PATH}\binary\shell_integration\windows\Release\x64\OCContextMenu_x64.dll" "$INSTDIR\shellext\OCContextMenu_x64.dll" "$INSTDIR\shellext"
         !undef LIBRARY_X64
     ${Else}
         !insertmacro InstallLib DLL NOTSHARED REBOOT_PROTECTED "${SOURCE_PATH}\binary\shell_integration\windows\Release\Win32\OCUtil_x86.dll" "$INSTDIR\shellext\OCUtil_x86.dll" "$INSTDIR\shellext"
         !insertmacro InstallLib REGDLL NOTSHARED REBOOT_PROTECTED "${SOURCE_PATH}\binary\shell_integration\windows\Release\Win32\OCOverlays_x86.dll" "$INSTDIR\shellext\OCOverlays_x86.dll" "$INSTDIR\shellext"
         !insertmacro InstallLib REGDLL NOTSHARED REBOOT_PROTECTED "${SOURCE_PATH}\binary\shell_integration\windows\Release\Win32\OCContextMenu_x86.dll" "$INSTDIR\shellext\OCContextMenu_x86.dll" "$INSTDIR\shellext"
      ${Endif}
      !undef LIBRARY_COM
      !undef LIBRARY_SHELL_EXTENSION
      !undef LIBRARY_IGNORE_VERSION
   ${MementoSectionEnd}
!endif

SectionGroup $SectionGroup_Shortcuts

!ifdef OPTION_SECTION_SC_START_MENU
   ${MementoSection} $OPTION_SECTION_SC_START_MENU_SECTION SEC_START_MENU
      SectionIn 1 2 3
      SetDetailsPrint textonly
      DetailPrint $OPTION_SECTION_SC_START_MENU_DetailPrint
      SetDetailsPrint listonly
      SetShellVarContext all
      CreateShortCut "$SMPROGRAMS\${APPLICATION_NAME}.lnk" "$INSTDIR\${APPLICATION_EXECUTABLE}" "" "$INSTDIR\ampcare.exe" 0
      SetShellVarContext current
   ${MementoSectionEnd}
!endif

!ifdef OPTION_SECTION_SC_DESKTOP
   ${MementoSection} $OPTION_SECTION_SC_DESKTOP_SECTION SEC_DESKTOP
      SectionIn 1 2
      SetDetailsPrint textonly
      DetailPrint $OPTION_SECTION_SC_DESKTOP_DetailPrint
      SetDetailsPrint listonly
      SetShellVarContext all
      CreateShortCut "$DESKTOP\${APPLICATION_NAME}.lnk" "$INSTDIR\${APPLICATION_EXECUTABLE}" "" "$INSTDIR\ampcare.exe" 0
      SetShellVarContext current
   ${MementoSectionEnd}
!endif

!ifdef OPTION_SECTION_SC_QUICK_LAUNCH
   ${MementoSection} $OPTION_SECTION_SC_QUICK_LAUNCH_SECTION SEC_QUICK_LAUNCH
      SectionIn 1 2
      SetDetailsPrint textonly
      DetailPrint $OPTION_SECTION_SC_QUICK_LAUNCH_DetailPrint
      SetShellVarContext all
      SetDetailsPrint listonly
      CreateShortCut "$QUICKLAUNCH\${APPLICATION_NAME}.lnk" "$INSTDIR\${APPLICATION_EXECUTABLE}" "" "$INSTDIR\ampcare.exe" 0
      SetShellVarContext current
   ${MementoSectionEnd}
!endif

SectionGroupEnd

${MementoSectionDone}

; Installer section descriptions
;--------------------------------
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${SEC_APPLICATION} $OPTION_SECTION_SC_APPLICATION_Desc
!insertmacro MUI_DESCRIPTION_TEXT ${SEC_START_MENU} $OPTION_SECTION_SC_START_MENU_Desc
!insertmacro MUI_DESCRIPTION_TEXT ${SEC_DESKTOP} $OPTION_SECTION_SC_DESKTOP_Desc
!insertmacro MUI_DESCRIPTION_TEXT ${SEC_QUICK_LAUNCH} $OPTION_SECTION_SC_QUICK_LAUNCH_Desc
!insertmacro MUI_FUNCTION_DESCRIPTION_END


Section -post

   ;Uninstaller file.
   SetDetailsPrint textonly
   DetailPrint $UNINSTALLER_FILE_Detail
   SetDetailsPrint listonly
   WriteUninstaller $INSTDIR\uninstall.exe

   ;Registry keys required for installer version handling and uninstaller.
   SetDetailsPrint textonly
   DetailPrint $UNINSTALLER_REGISTRY_Detail
   SetDetailsPrint listonly

   ;Version numbers used to detect existing installation version for comparison.
   WriteRegStr HKLM "Software\${APPLICATION_VENDOR}\${APPLICATION_NAME}" "" $INSTDIR
   WriteRegDWORD HKLM "Software\${APPLICATION_VENDOR}\${APPLICATION_NAME}" "VersionYear" "${VER_YEAR}"
   WriteRegDWORD HKLM "Software\${APPLICATION_VENDOR}\${APPLICATION_NAME}" "VersionMonth" "${VER_MONTH}"
   WriteRegDWORD HKLM "Software\${APPLICATION_VENDOR}\${APPLICATION_NAME}" "VersionRev" "${VER_REV}"
   WriteRegDWORD HKLM "Software\${APPLICATION_VENDOR}\${APPLICATION_NAME}" "VersionBuild" "${VER_BUILD}"

   ;Add or Remove Programs entry.
   WriteRegExpandStr ${MEMENTO_REGISTRY_ROOT} "${MEMENTO_REGISTRY_KEY}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
   WriteRegExpandStr ${MEMENTO_REGISTRY_ROOT} "${MEMENTO_REGISTRY_KEY}" "InstallLocation" "$INSTDIR"
   WriteRegStr ${MEMENTO_REGISTRY_ROOT} "${MEMENTO_REGISTRY_KEY}" "DisplayName" "${APPLICATION_NAME}"
   WriteRegStr ${MEMENTO_REGISTRY_ROOT} "${MEMENTO_REGISTRY_KEY}" "Publisher" "${APPLICATION_VENDOR}"
   WriteRegStr ${MEMENTO_REGISTRY_ROOT} "${MEMENTO_REGISTRY_KEY}" "DisplayIcon" "$INSTDIR\Uninstall.exe,0"
   WriteRegStr ${MEMENTO_REGISTRY_ROOT} "${MEMENTO_REGISTRY_KEY}" "DisplayVersion" "${VERSION}"
   WriteRegDWORD ${MEMENTO_REGISTRY_ROOT} "${MEMENTO_REGISTRY_KEY}" "VersionYear" "${VER_YEAR}"
   WriteRegDWORD ${MEMENTO_REGISTRY_ROOT} "${MEMENTO_REGISTRY_KEY}" "VersionMonth" "${VER_MONTH}.${VER_REV}"
   WriteRegStr ${MEMENTO_REGISTRY_ROOT} "${MEMENTO_REGISTRY_KEY}" "URLInfoAbout" "http://${APPLICATION_DOMAIN}/"
   WriteRegStr ${MEMENTO_REGISTRY_ROOT} "${MEMENTO_REGISTRY_KEY}" "HelpLink" "http://${APPLICATION_DOMAIN}/"
   WriteRegDWORD ${MEMENTO_REGISTRY_ROOT} "${MEMENTO_REGISTRY_KEY}" "NoModify" "1"
   WriteRegDWORD ${MEMENTO_REGISTRY_ROOT} "${MEMENTO_REGISTRY_KEY}" "NoRepair" "1"


   SetDetailsPrint textonly
   DetailPrint $UNINSTALLER_FINISHED_Detail
SectionEnd

##############################################################################
#                                                                            #
#   UNINSTALLER SECTION                                                      #
#                                                                            #
##############################################################################

Function un.EnsureOwncloudShutdown
   !insertmacro CheckAndConfirmEndProcess "${APPLICATION_EXECUTABLE}"
FunctionEnd

Section Uninstall
   IfFileExists "$INSTDIR\${APPLICATION_EXECUTABLE}" owncloud_installed
      MessageBox MB_YESNO $UNINSTALL_MESSAGEBOX /SD IDYES IDYES owncloud_installed
      Abort $UNINSTALL_ABORT
   owncloud_installed:

   ; Delete Navigation Pane entries added for Windows 10.
   ; On 64bit Windows, the client will be writing to the 64bit registry.
   ${If} ${RunningX64}
      SetRegView 64
   ${EndIf}
   StrCpy $0 0
   loop:
      ; Look at every registered explorer namespace for HKCU and check if it was added by our application
      ; (we write to a custom "ApplicationName" value there).
      EnumRegKey $1 HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\Desktop\NameSpace" $0
      StrCmp $1 "" done

      ReadRegStr $R0 HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\Desktop\NameSpace\$1" "ApplicationName"
      StrCmp $R0 "${APPLICATION_NAME}" deleteClsid
      ; Increment the index when not deleting the enumerated key.
      IntOp $0 $0 + 1
      goto loop

      deleteClsid:
         DetailPrint "Removing Navigation Pane CLSID $1"
         ; Should match FolderMan::updateCloudStorageRegistry
         DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\Desktop\NameSpace\$1"
         DeleteRegKey HKCU "Software\Classes\CLSID\$1"
         DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\HideDesktopIcons\NewStartPanel" $1
         goto loop
   done:
   ; Go back to the 32bit registry.
   SetRegView lastused

   ;Delete registry keys.
   DeleteRegValue HKLM "Software\${APPLICATION_VENDOR}\${APPLICATION_NAME}" "VersionYear"
   DeleteRegValue HKLM "Software\${APPLICATION_VENDOR}\${APPLICATION_NAME}" "VersionMonth"
   DeleteRegValue HKLM "Software\${APPLICATION_VENDOR}\${APPLICATION_NAME}" "VersionRevision"
   DeleteRegValue HKLM "Software\${APPLICATION_VENDOR}\${APPLICATION_NAME}" ""
   DeleteRegKey HKLM "Software\${APPLICATION_VENDOR}\${APPLICATION_NAME}"

   DeleteRegKey HKCR "${APPLICATION_NAME}"

   ;Shell extension
   !ifdef OPTION_SECTION_SC_SHELL_EXT
      !define LIBRARY_COM
      !define LIBRARY_SHELL_EXTENSION
      !define LIBRARY_IGNORE_VERSION
      ${If} ${HasSection} SEC_SHELL_EXT
        DetailPrint "Uninstalling x64 overlay DLLs"
        !define LIBRARY_X64
        !insertmacro UnInstallLib REGDLL NOTSHARED REBOOT_PROTECTED "$INSTDIR\shellext\OCContextMenu_x64.dll"
        !insertmacro UnInstallLib REGDLL NOTSHARED REBOOT_PROTECTED "$INSTDIR\shellext\OCOverlays_x64.dll"
        !insertmacro UnInstallLib DLL NOTSHARED REBOOT_PROTECTED "$INSTDIR\shellext\OCUtil_x64.dll"
        !undef LIBRARY_X64
        DetailPrint "Uninstalling x86 overlay DLLs"
        !insertmacro UnInstallLib REGDLL NOTSHARED REBOOT_PROTECTED "$INSTDIR\shellext\OCContextMenu_x86.dll"
        !insertmacro UnInstallLib REGDLL NOTSHARED REBOOT_PROTECTED "$INSTDIR\shellext\OCOverlays_x86.dll"
        !insertmacro UnInstallLib DLL NOTSHARED REBOOT_PROTECTED "$INSTDIR\shellext\OCUtil_x86.dll"
      ${EndIf}
      !undef LIBRARY_COM
      !undef LIBRARY_SHELL_EXTENSION
      !undef LIBRARY_IGNORE_VERSION
  !endif

   ;Start menu shortcut
   !ifdef OPTION_SECTION_SC_START_MENU
      SetShellVarContext all
      ${If} ${HasSection} SEC_START_MENU
         Delete "$SMPROGRAMS\${APPLICATION_NAME}.lnk"
      ${EndIf}
      SetShellVarContext current
   !endif

   ;Desktop shortcut.
   !ifdef OPTION_SECTION_SC_DESKTOP
      ${If} ${HasSection} SEC_DESKTOP
         SetShellVarContext all
         ${If} ${FileExists} "$DESKTOP\${APPLICATION_NAME}.lnk"
            Delete "$DESKTOP\${APPLICATION_NAME}.lnk"
         ${EndIf}
         SetShellVarContext current
      ${EndIf}
   !endif

   ;Quick Launch shortcut.
   !ifdef OPTION_SECTION_SC_QUICK_LAUNCH
      ${If} ${HasSection} SEC_QUICK_LAUNCH
         SetShellVarContext all
         ${If} ${FileExists} "$QUICKLAUNCH\${APPLICATION_NAME}.lnk"
            Delete "$QUICKLAUNCH\${APPLICATION_NAME}.lnk"
         ${EndIf}
         SetShellVarContext current
      ${EndIf}
   !endif

   ;Remove all the Program Files.
   RMDir /r $INSTDIR

   DeleteRegKey ${MEMENTO_REGISTRY_ROOT} "${MEMENTO_REGISTRY_KEY}"

   SetDetailsPrint textonly
   DetailPrint $UNINSTALLER_FINISHED_Detail
SectionEnd

##############################################################################
#                                                                            #
#   NSIS Installer Event Handler Functions                                   #
#                                                                            #
##############################################################################

Function .onInit
   SetOutPath $INSTDIR

   ${GetParameters} $R0
   ${GetOptions} $R0 "/launch" $R0
   ${IfNot} ${Errors}
      StrCpy $InstallRunIfSilent "yes"
   ${EndIf}

   ${GetParameters} $R0
   ${GetOptions} $R0 "/noautoupdate" $R0
   ${IfNot} ${Errors}
      StrCpy $NoAutomaticUpdates "yes"
   ${EndIf}


   !insertmacro INSTALLOPTIONS_EXTRACT "NSIS.InstallOptions.ini"

   ; uncomment this line if you want to see the language selection
   ;!insertmacro MUI_LANGDLL_DISPLAY

   Call SetLang

   ; Remove Quick Launch option from Windows 7, as no longer applicable - usually.
   ${IfNot} ${AtMostWinVista}
      SectionSetText ${SEC_QUICK_LAUNCH} $INIT_NO_QUICK_LAUNCH
      SectionSetFlags ${SEC_QUICK_LAUNCH} ${SF_RO}
      SectionSetInstTypes ${SEC_QUICK_LAUNCH} 0
   ${EndIf}

   ; Some people might have a shortcut called 'ownCloud' pointing elsewhere, see #356
   ; Unselect item and adjust text
   ${If} ${FileExists} "$DESKTOP\${APPLICATION_NAME}.lnk"
      SectionSetText ${SEC_DESKTOP} $INIT_NO_DESKTOP
      Push $0
      SectionSetFlags ${SEC_DESKTOP} 0
      SectionSetInstTypes ${SEC_DESKTOP} 0
      Pop $0
   ${EndIf}

   ${MementoSectionRestore}

   UAC_TryAgain:
      !insertmacro UAC_RunElevated
      ${Switch} $0
      ${Case} 0
          ${IfThen} $1 = 1 ${|} Quit ${|} ;we are the outer process, the inner process has done its work, we are done
          ${IfThen} $3 <> 0 ${|} ${Break} ${|} ;we are admin, let the show go on
          ${If} $1 = 3 ;RunAs completed successfully, but with a non-admin user
             MessageBox mb_YesNo|mb_ICONEXCLAMATION|MB_TOPMOST|MB_SETFOREGROUND $UAC_INSTALLER_REQUIRE_ADMIN /SD IDNO IDYES UAC_TryAgain IDNO 0
          ${EndIf}
          ;fall-through and die
      ${Case} 1223
         MessageBox MB_ICONSTOP|MB_TOPMOST|MB_SETFOREGROUND $UAC_INSTALLER_REQUIRE_ADMIN
         Quit
      ${Case} 1062
         MessageBox MB_ICONSTOP|MB_TOPMOST|MB_SETFOREGROUND $UAC_ERROR_LOGON_SERVICE
         Quit
      ${Default}
         MessageBox MB_ICONSTOP "$UAC_ERROR_ELEVATE $0"
         Abort
         Quit
      ${EndSwitch}

   ;Prevent multiple instances.
   System::Call 'kernel32::CreateMutexA(i 0, i 0, t "${APPLICATION_SHORTNAME}Installer") i .r1 ?e'
   Pop $R0
   StrCmp $R0 0 +3
      MessageBox MB_OK|MB_ICONEXCLAMATION $INIT_INSTALLER_RUNNING
      Abort

   ;Use available InstallLocation when possible. This is useful in the uninstaller
   ;via re-install, which would otherwise use a default location - a bug.
   ReadRegStr $R0 ${MEMENTO_REGISTRY_ROOT} "${MEMENTO_REGISTRY_KEY}" "InstallLocation"
   StrCmp $R0 "" SkipSetInstDir
   StrCpy $INSTDIR $R0
   SkipSetInstDir:

   ;Shutdown ${APPLICATION_NAME} in case Add/Remove re-installer option used.
   Call EnsureOwncloudShutdown

   ReadRegStr $R0 ${MEMENTO_REGISTRY_ROOT} "${MEMENTO_REGISTRY_KEY}" "InstallLocation"
   ${If} ${Silent}
   ${AndIf} $R0 != ""
       ExecWait '"$R0\Uninstall.exe" /S _?=$R0'
   ${EndIf}
FunctionEnd

Function .onInstSuccess
   ${MementoSectionSave}

   ${If} $NoAutomaticUpdates == "yes"
      WriteRegDWORD HKLM "Software\${APPLICATION_VENDOR}\${APPLICATION_NAME}" "skipUpdateCheck" "1"
   ${EndIf}

   ; TODO: Only needed to when updating from 2.1.{0,1}. Remove in due time.
   Delete /REBOOTOK $INSTDIR\bearer\qgenericbearer.dll
   Delete /REBOOTOK $INSTDIR\bearer\qnativewifibearer.dll
   RMDir /REBOOTOK $INSTDIR\bearer

   ${If} ${Silent}
   ${AndIf} $InstallRunIfSilent == "yes"
     Call LaunchApplication
   ${EndIf}
FunctionEnd

Function .onInstFailed
FunctionEnd

##############################################################################
#                                                                            #
#   NSIS Uninstaller Event Handler Functions                                 #
#                                                                            #
##############################################################################

Function un.onInit
   Call un.SetLang

   UAC_TryAgain:
      !insertmacro UAC_RunElevated
      ${Switch} $0
      ${Case} 0
          ${IfThen} $1 = 1 ${|} Quit ${|} ;we are the outer process, the inner process has done its work, we are done
          ${IfThen} $3 <> 0 ${|} ${Break} ${|} ;we are admin, let the show go on
          ${If} $1 = 3 ;RunAs completed successfully, but with a non-admin user
             MessageBox mb_YesNo|mb_ICONEXCLAMATION|MB_TOPMOST|MB_SETFOREGROUND $UAC_UNINSTALLER_REQUIRE_ADMIN /SD IDNO IDYES UAC_TryAgain IDNO 0
          ${EndIf}
          ;fall-through and die
      ${Case} 1223
         MessageBox MB_ICONSTOP|MB_TOPMOST|MB_SETFOREGROUND $UAC_UNINSTALLER_REQUIRE_ADMIN
         Quit
      ${Case} 1062
         MessageBox MB_ICONSTOP|MB_TOPMOST|MB_SETFOREGROUND $UAC_ERROR_LOGON_SERVICE
         Quit
      ${Default}
         MessageBox MB_ICONSTOP "$UAC_ERROR_ELEVATE $0"
         Abort
         Quit
      ${EndSwitch}

   ;Prevent multiple instances.
   System::Call 'kernel32::CreateMutexA(i 0, i 0, t "${APPLICATION_SHORTNAME}Uninstaller") i .r1 ?e'
   Pop $R0
   StrCmp $R0 0 +3
      MessageBox MB_OK|MB_ICONEXCLAMATION $INIT_UNINSTALLER_RUNNING
      Abort

   ;Shutdown ${APPLICATION_NAME} in order to remove locked files.
   Call un.EnsureOwncloudShutdown
FunctionEnd

Function un.onUnInstSuccess
FunctionEnd

Function un.onUnInstFailed
FunctionEnd
