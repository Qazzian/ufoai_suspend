;
; This installer script is for distributing ufo with compressed zip archives
; that contains the data - use installer.nsi if you don't want an installer
; with zip files
; Note: use the archives.sh bash script in base/ folder to generate the data
; archive files
;

; Script generated by the HM NIS Edit Script Wizard.

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "UFO:Alien Invasion"
!define SHORT_PRODUCT_NAME "UFO:AI"
!define PRODUCT_NAME_DEDICATED "UFO:Alien Invasion Dedicated Server"
!define PRODUCT_VERSION "2.2-dev"
!define PRODUCT_PUBLISHER "UFO:AI Team"
!define PRODUCT_WEB_SITE "http://ufoai.sf.net"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\ufo.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

;SetCompressor bzip2
SetCompressor lzma

; MUI 1.67 compatible ------
!include "MUI.nsh"
!include "LogicLib.nsh"

ShowInstDetails "nevershow"
ShowUninstDetails "nevershow"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "..\..\..\build\projects\ufo.ico"
!define MUI_UNICON "..\..\..\build\projects\ufo.ico"

; Language Selection Dialog Settings
!define MUI_LANGDLL_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
!define MUI_LANGDLL_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "NSIS:Language"

!define MUI_WELCOMEFINISHPAGE_BITMAP "..\..\..\build\installer.bmp"

!macro MUI_PAGE_DIRECTORY_CUSTOM

  !verbose push
  !verbose ${MUI_VERBOSE}

  !insertmacro MUI_PAGE_INIT

  !insertmacro MUI_SET MUI_${MUI_PAGE_UNINSTALLER_PREFIX}DIRECTORYPAGE

  !insertmacro MUI_DEFAULT MUI_DIRECTORYPAGE_TEXT_TOP ""
  !insertmacro MUI_DEFAULT MUI_DIRECTORYPAGE_TEXT_DESTINATION ""

  PageEx ${MUI_PAGE_UNINSTALLER_FUNCPREFIX}directory

    PageCallbacks ${MUI_PAGE_UNINSTALLER_FUNCPREFIX}mui.DirectoryPre_${MUI_UNIQUEID} ${MUI_PAGE_UNINSTALLER_FUNCPREFIX}mui.DirectoryShow_${MUI_UNIQUEID} ${MUI_PAGE_UNINSTALLER_FUNCPREFIX}mui.DirectoryLeave_${MUI_UNIQUEID}

    Caption " "

    DirText "${MUI_DIRECTORYPAGE_TEXT_TOP}" "${MUI_DIRECTORYPAGE_TEXT_DESTINATION}"

    !ifdef MUI_DIRECTORYPAGE_VARIABLE
      DirVar "${MUI_DIRECTORYPAGE_VARIABLE}"
    !endif

    !ifdef MUI_DIRECTORYPAGE_VERIFYONLEAVE
      DirVerify leave
      PageCallbacks "" "" dirLeave
    !endif

  PageExEnd

  !insertmacro MUI_FUNCTION_DIRECTORYPAGE ${MUI_PAGE_UNINSTALLER_FUNCPREFIX}mui.DirectoryPre_${MUI_UNIQUEID} ${MUI_PAGE_UNINSTALLER_FUNCPREFIX}mui.DirectoryShow_${MUI_UNIQUEID} ${MUI_PAGE_UNINSTALLER_FUNCPREFIX}mui.DirectoryLeave_${MUI_UNIQUEID}

  !undef MUI_DIRECTORYPAGE_TEXT_TOP
  !undef MUI_DIRECTORYPAGE_TEXT_DESTINATION
  !insertmacro MUI_UNSET MUI_DIRECTORYPAGE_BGCOLOR
  !insertmacro MUI_UNSET MUI_DIRECTORYPAGE_VARIABLE
  !insertmacro MUI_UNSET MUI_DIRECTORYPAGE_VERIFYONLEAVE

  !verbose pop

!macroend

Var GAMEFLAGS
Var MAPFLAGS
Var GAMETEST
Var MAPTEST
Var GAMEICONFLAGS
Var MAPICONFLAGS
Var NONEMPTY

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!define MUI_LICENSEPAGE_CHECKBOX
!insertmacro MUI_PAGE_LICENSE "..\..\..\COPYING"
!define MUI_COMPONENTSPAGE_SMALLDESC
!insertmacro MUI_PAGE_COMPONENTS
; Directory page
!define MUI_DIRECTORYPAGE_VERIFYONLEAVE
!insertmacro MUI_PAGE_DIRECTORY_CUSTOM
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
;!define MUI_FINISHPAGE_RUN "$INSTDIR\ufo.exe"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "German"

; Reserve files
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

; MUI end ------

Name "${SHORT_PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "ufoai-${PRODUCT_VERSION}-win32.exe"
InstallDir "$PROGRAMFILES\UFOAI-${PRODUCT_VERSION}"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

Function .onInit
  !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

SectionGroup /e "Game" SECGROUP01
  Section "Game Files" SEC01
    SetOverwrite ifnewer
    SetOutPath "$INSTDIR"
    File /nonfatal "..\..\..\src\docs\tex\*.pdf"
    File "..\..\..\contrib\*.dll"
    File "..\..\..\*.dll"
    File "..\..\..\*.exe"
    File "..\..\..\.gamedir"
    SetOutPath "$INSTDIR\base"
    File "..\..\..\base\*.dll"
    File "..\..\..\base\*.pk3"
    SetOutPath "$INSTDIR\base\i18n"
    SetOutPath "$INSTDIR\base\i18n\cs\LC_MESSAGES"
    File "..\..\..\base\i18n\cs\LC_MESSAGES\*.mo"
    SetOutPath "$INSTDIR\base\i18n\da\LC_MESSAGES"
    File "..\..\..\base\i18n\da\LC_MESSAGES\*.mo"
    SetOutPath "$INSTDIR\base\i18n\de\LC_MESSAGES"
    File "..\..\..\base\i18n\de\LC_MESSAGES\*.mo"
    SetOutPath "$INSTDIR\base\i18n\en\LC_MESSAGES"
    File "..\..\..\base\i18n\en\LC_MESSAGES\*.mo"
    SetOutPath "$INSTDIR\base\i18n\es\LC_MESSAGES"
    File "..\..\..\base\i18n\es\LC_MESSAGES\*.mo"
    SetOutPath "$INSTDIR\base\i18n\es_ES\LC_MESSAGES"
    File "..\..\..\base\i18n\es_ES\LC_MESSAGES\*.mo"
    SetOutPath "$INSTDIR\base\i18n\est\LC_MESSAGES"
    File "..\..\..\base\i18n\est\LC_MESSAGES\*.mo"
    SetOutPath "$INSTDIR\base\i18n\fi\LC_MESSAGES"
    File "..\..\..\base\i18n\fi\LC_MESSAGES\*.mo"
    SetOutPath "$INSTDIR\base\i18n\fr\LC_MESSAGES"
    File "..\..\..\base\i18n\fr\LC_MESSAGES\*.mo"
    SetOutPath "$INSTDIR\base\i18n\it\LC_MESSAGES"
    File "..\..\..\base\i18n\it\LC_MESSAGES\*.mo"
    SetOutPath "$INSTDIR\base\i18n\pl\LC_MESSAGES"
    File "..\..\..\base\i18n\pl\LC_MESSAGES\*.mo"
    SetOutPath "$INSTDIR\base\i18n\pt_BR\LC_MESSAGES"
    File "..\..\..\base\i18n\pt_BR\LC_MESSAGES\*.mo"
    SetOutPath "$INSTDIR\base\i18n\ru\LC_MESSAGES"
    File "..\..\..\base\i18n\ru\LC_MESSAGES\*.mo"
    SetOutPath "$INSTDIR\base\i18n\sv\LC_MESSAGES"
    File "..\..\..\base\i18n\sv\LC_MESSAGES\*.mo"
    SetOutPath "$INSTDIR\base\i18n\slo\LC_MESSAGES"
    File "..\..\..\base\i18n\slo\LC_MESSAGES\*.mo"
  
  ;======================================================================
  ; to let the game start up
  ;======================================================================
    SetOutPath "$INSTDIR"
  
  SectionEnd
  
  Section "Game Shortcuts" SEC01B
    CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}\"
    CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk" "$INSTDIR\ufo.exe" "+set vid_fullscreen 1 + set snd_init 1" "$INSTDIR\ufo.exe" 0
    CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME} (safe-mode).lnk" "$INSTDIR\ufo.exe" "+set vid_fullscreen 1 +set gl_mode 6 +set snd_init 0" "$INSTDIR\ufo.exe" 0
    CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME} (safe-mode windowed).lnk" "$INSTDIR\ufo.exe" "+set vid_fullscreen 0 +set gl_mode 6 +set snd_init 0" "$INSTDIR\ufo.exe" 0
    CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME_DEDICATED}.lnk" "$INSTDIR\ufo_ded.exe" "" "$INSTDIR\ufo.exe_ded" 0
    CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\ufo.exe"
    CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
    CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk" "$INSTDIR\uninst.exe"
  SectionEnd
SectionGroupEnd

SectionGroup /e "Mapping" SECGROUP02
  Section "Mapping Tools" SEC02
    SetOutPath "$INSTDIR\base\maps"
    File "..\..\..\base\maps\*.map"
    File "..\..\..\base\maps\Makefile"
    File "..\..\..\base\maps\Makefile.win"
    File "..\..\..\base\maps\compile.p*"
    SetOutPath "$INSTDIR\base\maps\b"
    File "..\..\..\base\maps\b\*.map"
    SetOutPath "$INSTDIR\base\maps\countryd"
    File "..\..\..\base\maps\countryd\*.map"
    SetOutPath "$INSTDIR\base\maps\countryn"
    File "..\..\..\base\maps\countryn\*.map"
    SetOutPath "$INSTDIR\base\maps\frozend"
    File "..\..\..\base\maps\frozend\*.map"
    SetOutPath "$INSTDIR\base\maps\frozenn"
    File "..\..\..\base\maps\frozenn\*.map"
    SetOutPath "$INSTDIR\base\maps\icen"
    File "..\..\..\base\maps\icen\*.map"
    SetOutPath "$INSTDIR\base\maps\iced"
    File "..\..\..\base\maps\iced\*.map"
    SetOutPath "$INSTDIR\base\maps\orientald"
    File "..\..\..\base\maps\orientald\*.map"
    SetOutPath "$INSTDIR\base\maps\orientaln"
    File "..\..\..\base\maps\orientaln\*.map"
    SetOutPath "$INSTDIR\base\maps\tropicd"
    File "..\..\..\base\maps\tropicd\*.map"
    SetOutPath "$INSTDIR\base\maps\tropicn"
    File "..\..\..\base\maps\tropicn\*.map"
    SetOutPath "$INSTDIR\base\maps\villaged"
    File "..\..\..\base\maps\villaged\*.map"
    SetOutPath "$INSTDIR\base\maps\villagen"
    File "..\..\..\base\maps\villagen\*.map"
    SetOutPath "$INSTDIR\base\pics"
    File "..\..\..\base\pics\colormap.pcx"
    SetOutPath "$INSTDIR\tools"
    File "..\..\tools\*.ms"
    File "..\..\tools\*.qe4"
    File "..\..\tools\*.def"
    ; EULA
    File "..\..\..\contrib\*.doc"
    File "..\..\..\contrib\*.exe"
    File "..\..\..\ufo2map.exe"
  SectionEnd

  Section "Mapping Tools Shortcuts" SEC02B
    CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}\"
    CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\MAP-Editor.lnk" "$INSTDIR\tools\q3radiant.exe" "" "$INSTDIR\tools\q3radiant.exe" 0
  SectionEnd
SectionGroupEnd

Section "Source Code" SEC03
  SetOverwrite ifnewer
  SetOutPath "$INSTDIR\build"
  File "..\..\..\build\*.bmp"
  SetOutPath "$INSTDIR\build\projects"
  File "..\..\..\build\projects\*.cbp"
  File "..\..\..\build\projects\*.dev"
  File "..\..\..\build\projects\*.sln"
  File "..\..\..\build\projects\*.dsp"
  File "..\..\..\build\projects\*.dsw"
  File "..\..\..\build\projects\*.dev"
  File "..\..\..\build\projects\*.ico"
  File "..\..\..\build\projects\*.workspace"
  File "..\..\..\build\projects\*.vcproj"
  SetOutPath "$INSTDIR\src\client"
  File "..\..\client\*.h"
  File "..\..\client\*.c"
  SetOutPath "$INSTDIR\src\docs"
  File "..\..\docs\*.txt"
  File "..\..\docs\readme.linux"
  File "..\..\docs\readme.solaris"
  SetOutPath "$INSTDIR\src\docs\tex"
  File "..\..\docs\tex\*.tex"
  SetOutPath "$INSTDIR\src\docs\tex\chapters"
  File "..\..\docs\tex\chapters\*.tex"
  SetOutPath "$INSTDIR\src\docs\tex\images"
  File "..\..\docs\tex\images\*.jpg"
  SetOutPath "$INSTDIR\src\docs\tex\licenses"
  File "..\..\docs\tex\licenses\*.tex"
  SetOutPath "$INSTDIR\src\game"
  File "..\..\game\*.def"
  File "..\..\game\*.c"
  File "..\..\game\*.h"
  SetOutPath "$INSTDIR\src\ports"
  SetOutPath "$INSTDIR\src\ports\irix"
  File "..\..\ports\irix\*.c"
  SetOutPath "$INSTDIR\src\ports\unix"
  File "..\..\ports\unix\*.h"
  File "..\..\ports\unix\*.c"
  SetOutPath "$INSTDIR\src\ports\win32"
  File "..\..\ports\win32\*.h"
  File "..\..\ports\win32\*.c"
  SetOutPath "$INSTDIR\src\ports\macosx"
;  File "..\..\ports\macosx\*.h"
  File "..\..\ports\macosx\*.m"
  File "..\..\ports\macosx\*.c"
  SetOutPath "$INSTDIR\src\ports\linux"
  File "..\..\ports\linux\*.h"
  File "..\..\ports\linux\*.c"
  File "..\..\ports\linux\*.xbm"
  File "..\..\ports\linux\*.png"
  SetOutPath "$INSTDIR\src\ports\null"
  File "..\..\ports\null\*.c"
  SetOutPath "$INSTDIR\src\ports\solaris"
  File "..\..\ports\solaris\*.c"
;  File "..\..\ports\solaris\*.h"
  SetOutPath "$INSTDIR\src\po"
  File "..\..\po\*.po"
  File "..\..\po\*.pot"
  File "..\..\po\FINDUFOS"
  File "..\..\po\Makefile"
  File "..\..\po\Makevars"
  File "..\..\po\POTFILES.in"
  File "..\..\po\ufopo.pl"
  File "..\..\po\remove-potcdate.sin"
  SetOutPath "$INSTDIR\src\qcommon"
  File "..\..\qcommon\*.c"
  File "..\..\qcommon\*.h"
  SetOutPath "$INSTDIR\src\ref_gl"
  File "..\..\ref_gl\*.h"
  File "..\..\ref_gl\*.c"
  File "..\..\ref_gl\*.def"
  SetOutPath "$INSTDIR\src\server"
  File "..\..\server\*.h"
  File "..\..\server\*.c"

;======================================================================
; tools
;======================================================================
  SetOutPath "$INSTDIR\src\tools"
  File "..\..\tools\*.pl"
  File "..\..\tools\*.ms"
  SetOutPath "$INSTDIR\src\tools\blender"
  File "..\..\tools\blender\*.py"
  SetOutPath "$INSTDIR\src\tools\gtkradiant"
  File "..\..\tools\gtkradiant\*.diff"
  SetOutPath "$INSTDIR\src\tools\masterserver"
  File "..\..\tools\masterserver\*.c"
  SetOutPath "$INSTDIR\src\tools\gtkradiant\games"
  File "..\..\tools\gtkradiant\games\*.game"
  SetOutPath "$INSTDIR\src\tools\gtkradiant\plugin"
  SetOutPath "$INSTDIR\src\tools\gtkradiant\plugin\ufoai"
  File "..\..\tools\gtkradiant\plugin\ufoai\*.cpp"
  File "..\..\tools\gtkradiant\plugin\ufoai\*.h"
  SetOutPath "$INSTDIR\src\tools\gtkradiant\ufoai.game"
  File "..\..\tools\gtkradiant\ufoai.game\*.xml"
  File "..\..\tools\gtkradiant\ufoai.game\*.xlink"
  SetOutPath "$INSTDIR\src\tools\gtkradiant\ufoai.game\base"
  File "..\..\tools\gtkradiant\ufoai.game\base\*.def"
  SetOutPath "$INSTDIR\src\tools\qdata"
  File "..\..\tools\qdata\*.c"
  File "..\..\tools\qdata\*.h"
  SetOutPath "$INSTDIR\src\tools\masterserver"
  File "..\..\tools\masterserver\*.c"
  SetOutPath "$INSTDIR\src\tools\ufo2map"
  File "..\..\tools\ufo2map\*.h"
  File "..\..\tools\ufo2map\*.c"
  SetOutPath "$INSTDIR\src\tools\campaign_editor"
; TODO
  SetOutPath "$INSTDIR\src\tools\ufo2map\common"
  File "..\..\tools\ufo2map\common\*.h"
  File "..\..\tools\ufo2map\common\*.c"

  SetOutPath "$INSTDIR\src"
  File "..\..\CMakeLists.txt"
  SetOutPath "$INSTDIR"
SectionEnd

Section -AdditionalIcons
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\ufo.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\ufo.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd

; Section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01}  "The game and its data. You need this to play."
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01B} "Shortcuts for the game."
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC02}  "Mapping (and modelling) tools and map source files."
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC02B} "Shortcuts for the mapping tools."
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC03}  "C-Source code for UFO:Alien Invasion."
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Function .onVerifyInstDir
  IfFileExists $INSTDIR\*.* Invalid Valid
  Invalid:
  StrCmp $INSTDIR "C:" Break
  StrCmp $INSTDIR "C:\" Break
  StrCmp $INSTDIR $PROGRAMFILES Break
  StrCmp $INSTDIR $PROGRAMFILES Break
  StrCpy $NONEMPTY 1
  Goto End
  Valid:
  StrCpy $NONEMPTY 0
  Goto End
  Break:
  Abort
  End:
FunctionEnd

Function dirLeave
  GetInstDirError $0
  ${Switch} $0
    ${Case} 0
      ${Break}
    ${Case} 1
      MessageBox MB_OK "$INSTDIR is not a valid installation path!"
      Abort
      ${Break}
    ${Case} 2
      MessageBox MB_OK "Not enough free space!"
      Abort
      ${Break}
  ${EndSwitch}
  ${If} $NONEMPTY == 1
    MessageBox MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON2 "The destination folder already exists!$\r$\nAre you sure you want to install into that directory?" IDYES +2
    Abort
  ${EndIf}
FunctionEnd

Function .onSelChange
  ; This will ensure that you can't install the shortcuts without installing the target files
  SectionGetFlags ${SEC01} $GAMEFLAGS
  SectionGetFlags ${SEC02} $MAPFLAGS
  IntOP $GAMETEST $GAMEFLAGS & ${SF_SELECTED} ; tests the activation bit
  IntOP $MAPTEST $MAPFLAGS & ${SF_SELECTED} ; tests the activation bit

  IntCmp $GAMETEST 1 GameSelected
    SectionGetFlags ${SEC01B} $GAMEICONFLAGS
    IntOp $GAMEICONFLAGS $GAMEICONFLAGS & 510 ; Forces to zero the activation bit
    SectionSetFlags ${SEC01B} $GAMEICONFLAGS

  GameSelected:
  IntCmp $MAPTEST 1 done
    SectionGetFlags ${SEC02B} $MAPICONFLAGS
    IntOp $MAPICONFLAGS $MAPICONFLAGS & 510 ; Forces to zero the activation bit
    SectionSetFlags ${SEC02B} $MAPICONFLAGS

  done:
FunctionEnd

Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) successfully deinstalled."
FunctionEnd

Function un.onInit
!insertmacro MUI_UNGETLANGUAGE
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure that you want to remove $(^Name) and all its data?" IDYES +2
  Abort
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Do you also want to delete your configuration Files and saved games?" IDNO +2
  RMDIR /r "$APPDATA\UFOAI"
FunctionEnd

; This uninstaller is unsafe - if a user installs this in the root of C: for example, the uninstall will wipe the entire partition.
Section Uninstall
  RMDIR /r $INSTDIR
  RMDIR $INSTDIR
  RMDir /r "$SMPROGRAMS\${PRODUCT_NAME}"
  Delete "$DESKTOP\${PRODUCT_NAME}.lnk"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd
