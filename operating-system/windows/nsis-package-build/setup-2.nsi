!define PRODUCT_NAME "TRMS"
!define PRODUCT_VERSION "3.0"
!define PRODUCT_PUBLISHER "����Ϭ��Ƽ����޹�˾"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\Monitor.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
;!define AssetCode $R0

SetCompressor lzma

; ------ MUI  ------
!include "MUI.nsh"
;---------x64 detect---
!include "x64.nsh"  

; MUI 
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; welcome page
!insertmacro MUI_PAGE_WELCOME
;Page custom AssetCodePage AssetCodePageLeave
; 
!insertmacro MUI_PAGE_DIRECTORY
; 
!insertmacro MUI_PAGE_INSTFILES
; 
!insertmacro MUI_PAGE_FINISH
;!insertmacro MUI_FINISHPAGE_REBOOTSUPPORT

; 
!insertmacro MUI_UNPAGE_INSTFILES

; 
!insertmacro MUI_LANGUAGE "SimpChinese"

; 
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS
; ------ MUI  ------


;Var AssetCode

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "Trms_Setup${TIMESTAMP}.exe"
InstallDir "C:\Trms\Monitor"
InstallDirRegKey HKLM "${PRODUCT_UNINST_KEY}" "UninstallString"
ShowInstDetails show
ShowUnInstDetails show
RequestExecutionLevel admin

Section "MainSection" SEC01
  CreateDirectory "C:\Trms"
  
  SetOutPath "$INSTDIR"


  ${If} ${RunningX64}   
        ${EnableX64FSRedirection}
  ${EndIf}    
 
  ;��װǰ����ж�ط���
  nsExec::Exec "$INSTDIR\srvuninstall.bat"
  DeleteRegKey HKCU "Software\pcmant"
  sleep 3000 
  SetOverwrite on

  ;VC runtime dll lib setup
  File "files\vcredist_x86.exe"
  
  ;Qt ��ƽ̨���
  SetOutPath "$INSTDIR\platforms"
  File /r "files\platforms\*.*"
  
  ;����ģ��
  ;SetOutPath "$INSTDIR"
  SetOutPath "C:\Windows\System32"
  File "files\libcurl.dll"
  File "files\Qt5Core.dll"
  File "files\Qt5Widgets.dll"
  File "files\Qt5Gui.dll"
  File "files\Qt5Xml.dll"
  
  ;����ģ��
  SetOutPath "$INSTDIR"
  File "files\Update.exe"
  File "files\Update.exe.txt"
  File "files\MonitorService.exe"
  File "files\TRMSMonitor.exe"
  
  ;�����ļ�
  File "files\config.xml"
  File "files\config.xml.txt"
  
  ;����װģ��
  File "files\srv.bat"
  File "files\srv.bat.txt"
  File "files\srvuninstall.bat"
  File "files\srvuninstall.bat.txt"
  
  ;������
  SetOutPath "$INSTDIR\plugins"
  File "files\plugins\ComputerInfo.dll"

  ;ע������ã���ʱ����
  WriteRegStr HKCU "Software\pcmant" "AssetCode" ""
  WriteRegStr HKCU "Software\pcmant" "UserName" ""
  WriteRegStr HKCU "Software\pcmant" "PassWord" ""
  
  ;�Ȱ�װVC runtime lib ��װ��������
  SetOutPath "$INSTDIR"
  nsExec::Exec "$INSTDIR\vcredist_x86.exe /quiet /norestart"
  nsExec::Exec "$INSTDIR\srv.bat"

  ;ɾ�� VC runtime lib setup
  Delete "$INSTDIR\vcredist_x86.exe"

  ${If} ${RunningX64}   
        ${DisableX64FSRedirection}
  ${EndIf}    
  
SectionEnd

Section -AdditionalIcons
  SetOutPath $INSTDIR
  CreateShortCut "$SMPROGRAMS\Monitor\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\Update.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\Update.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd

/******************************
 *  ж��ģ��  *
 ******************************/

Section Uninstall

   ${If} ${RunningX64}   
        ${EnableX64FSRedirection}
  ${EndIf}    
  
  ;ж�ط���
  nsExec::Exec "$INSTDIR\srvuninstall.bat"
  sleep 3000
  
  Delete "$INSTDIR\srv.bat"
  Delete "$INSTDIR\srv.bat.txt"
  Delete "$INSTDIR\srvuninstall.bat"
  Delete "$INSTDIR\srvuninstall.bat.txt"
  
  Delete "$INSTDIR\uninst.exe"
  
  ;ж�ز��
  Delete "$INSTDIR\plugins\ComputerInfo.dll"
  
  ;ж�س���ģ��
  Delete "$INSTDIR\TRMSMonitor.exe"
  Delete "$INSTDIR\MonitorService.exe"
  Delete "$INSTDIR\Update.exe"
  Delete "$INSTDIR\Update.exe.txt"

  ;�����ļ�
  Delete "$INSTDIR\config.xml"
  Delete "$INSTDIR\config.xml.txt"

  ;ж��������
  Delete "C:\Windows\System32\libcurl.dll"
  Delete "C:\Windows\System32\Qt5Core.dll"
  Delete "C:\Windows\System32\Qt5Widgets.dll"
  Delete "C:\Windows\System32\Qt5Gui.dll"
  Delete "C:\Windows\System32\Qt5Xml.dll"
  
  

  Delete "$SMPROGRAMS\Monitor\Uninstall.lnk"
  Delete "$DESKTOP\Monitor.lnk"
  Delete "$SMPROGRAMS\Monitor\Monitor.lnk"

  RMDir /r "$SMPROGRAMS\Monitor"
  RMDir /r "$INSTDIR\plugins"
  RMDir /r "$INSTDIR\platforms"
  RMDir /r "$INSTDIR\logs"

  RMDir /r "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  DeleteRegKey HKCU "Software\pcmant"
  SetAutoClose true

  ${If} ${RunningX64}   
        ${DisableX64FSRedirection}
  ${EndIf}    

SectionEnd



Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "�Ƿ�Ҫ��ȫж�� $(^Name)" IDYES +2
  Abort
FunctionEnd

Function .onInit
  SetSilent silent
FunctionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) �Ѿ�ж�سɹ�"
FunctionEnd

Function AssetCodePage
  !insertmacro MUI_HEADER_TEXT "" ""
  ReserveFile "asset.ini"
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "asset.ini"
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "asset.ini"
FunctionEnd

Function AssetCodePageLeave
  ;!insertmacro MUI_INSTALLOPTIONS_READ $AssetCode "asset.ini" "Field 2" "State"
  ;StrCmp $AssetCode "" 0 +3
  ;MessageBox MB_OK ""
  ;Abort
FunctionEnd

