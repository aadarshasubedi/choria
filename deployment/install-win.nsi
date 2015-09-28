; Script generated by the HM NIS Edit Script Wizard.

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "choria"
!define PRODUCT_VERSION "0.4.2"
!define PRODUCT_PUBLISHER "Alan Witkowski"
!define PRODUCT_WEB_SITE "https://github.com/jazztickets/choria"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\choria.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "..\LICENSE"
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\choria.exe"
; !define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\readme.txt"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; MUI end ------

RequestExecutionLevel admin
Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "choria-${PRODUCT_VERSION}-setup.exe"
InstallDir "$PROGRAMFILES\choria"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

Section "MainSection" SEC01
  SetShellVarContext all
  SetOutPath "$INSTDIR"
  File "..\CHANGELOG"
  File "..\LICENSE"
  File "..\README"
  File "..\bin\Release\choria.exe"
  CreateDirectory "$SMPROGRAMS\choria"
  CreateShortCut "$SMPROGRAMS\choria\choria.lnk" "$INSTDIR\choria.exe"
  CreateShortCut "$SMPROGRAMS\choria\choria Server.lnk" "$INSTDIR\choria.exe" "-host"
  CreateShortCut "$SMPROGRAMS\choria\choria Map Editor.lnk" "$INSTDIR\choria.exe" "-mapeditor"
  CreateShortCut "$DESKTOP\choria.lnk" "$INSTDIR\choria.exe"
  CreateShortCut "$SMPROGRAMS\choria\choria Server.lnk" "$INSTDIR\choria.exe" "-host"
  CreateShortCut "$SMPROGRAMS\choria\choria Map Editor.lnk" "$INSTDIR\choria.exe" "-mapeditor"
  File "..\working\Irrlicht.dll"
  File /r "..\working\database"
  File /r "..\working\fonts"
  File /r "..\working\maps"
  File /r "..\working\textures"
SectionEnd

Section -AdditionalIcons
  SetShellVarContext all
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\choria\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\choria\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\choria.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\choria.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd

Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  SetShellVarContext all

  Delete "$SMPROGRAMS\choria\Uninstall.lnk"
  Delete "$SMPROGRAMS\choria\Website.lnk"
  Delete "$DESKTOP\choria.lnk"
  Delete "$SMPROGRAMS\choria\choria.lnk"
  Delete "$SMPROGRAMS\choria\choria Server.lnk"
  Delete "$SMPROGRAMS\choria\choria Map Editor.lnk"
  Delete "$INSTDIR\choria Server.lnk"
  Delete "$INSTDIR\choria Map Editor.lnk"

  RMDir "$SMPROGRAMS\choria"
  RMDir /r "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd
