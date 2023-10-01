#NoTrayIcon

#Region
#AutoIt3Wrapper_Icon=icon.ico
#AutoIt3Wrapper_Outfile=מילון הבייניש הגדול מאת פיתוחי חותם.exe
#AutoIt3Wrapper_Compression=4
#AutoIt3Wrapper_UseX64=n
#AutoIt3Wrapper_Res_Comment=Pituchey Hotam | פיתוחי חותם
#AutoIt3Wrapper_Res_Description=מילון הבייניש הגדול
#AutoIt3Wrapper_Res_Fileversion=1.0.0.0
#AutoIt3Wrapper_Res_ProductName=מילון הבייניש הגדול
#AutoIt3Wrapper_Res_ProductVersion=1.0.0.0
#AutoIt3Wrapper_Res_CompanyName=Pituchey Hotam
#AutoIt3Wrapper_Res_LegalCopyright=Pituchey Hotam | פיתוחי חותם
#AutoIt3Wrapper_Res_Language=1037
#AutoIt3Wrapper_Res_Field=ProductName|מילון הבייניש הגדול
#AutoIt3Wrapper_Res_Field=ProductVersion|1.0.0.0
#AutoIt3Wrapper_Res_Field=CompanyName|Pituchey Hotam ~ פיתוחי חותם
#EndRegion

#include <IE.au3>
#include <INet.au3>
#include <WinAPIFiles.au3>
#include <Misc.au3>

If _Singleton("Pituchey-Hotam-TBBD", 1) = 0 Then
	Exit
EndIf

Local $sServerVersionUrl = "https://raw.githubusercontent.com/Pituchey-Hotam/The-Big-Beiinish-Dictionary/data/version.txt"
Local $slastUpdateHebrewDateUrl = "https://raw.githubusercontent.com/Pituchey-Hotam/The-Big-Beiinish-Dictionary/data/last-update-hebrew-date.txt"
Local $sTheDictionaryUrl = "https://raw.githubusercontent.com/Pituchey-Hotam/The-Big-Beiinish-Dictionary/data/TBBD.dic"

Local $sMasterRegistryPath = "HKEY_CURRENT_USER\SOFTWARE\Microsoft\Shared Tools\Proofing Tools\1.0\Custom Dictionaries"
Local $sConfigDirPath = @AppDataDir & "\PitucheyHotam"
Local $sFilePath = $sConfigDirPath & "\מילון הבייניש הגדול - מאת פיתוחי חותם.dic"
Local $sConfigPath = $sConfigDirPath & "\config.TBBD.ini"

Local $sOriginalFileVersion = IniRead($sConfigPath, "General", "current_version", "0.0.0")
Local $sServerFileVersion = _INetGetSource($sServerVersionUrl)

Local $hGUI = GUICreate("מילון הבייניש הגדול - גרסה " & $sOriginalFileVersion, 400, 150, -1, -1, BitOr(12582912, -2147483648, 524288), 0x400000) ;BitOr($WS_CAPTION, $WS_POPUP, $WS_SYSMENU)
Global $idButtonChangeStatus = GUICtrlCreateButton("התקן את המילון", 130, 30, 150, 40)
Global $idButtonReloadDict = GUICtrlCreateButton("עדכן את המילון", 280, 50, 90, 20)
Global $idLabelStatus = GUICtrlCreateLabel("סטטוס: לא מותקן", 165, 10)
Global $idLabelLastUpdate = GUICtrlCreateLabel("תאריך העדכון האחרון" & @CRLF & IniRead($sConfigPath, "General", "last_update_date", "<התוכנה לא הותקנה>"), 5, 35, 120, 30)
GUICtrlCreateLabel("מילון הבייניש הגדול מאת פיתוחי חותם - גרסה " & $sOriginalFileVersion, 55, 80)
Local $idMenuFile = GUICtrlCreateMenu("&קובץ")
Local $idMenuAbout = GUICtrlCreateMenu("&אודות")
Local $idButton_About = GUICtrlCreateMenuItem("&אודות (F2)", $idMenuAbout)
Local $idButton_Site = GUICtrlCreateMenuItem("&אתר האינטרנט של התוכנה", $idMenuFile)
Local $idButton_Updates = GUICtrlCreateMenuItem("&לקבלת עדכונים על פיתוחים נוספים", $idMenuFile)
Local $idButton_JoinUs = GUICtrlCreateMenuItem("&הצטרפות לפיתוחי חותם", $idMenuFile)
Local $idButton_ExitMenu = GUICtrlCreateMenuItem("&יציאה (ESC)", $idMenuFile)
GUICtrlCreateLabel("מייל תמיכה: pituchey-hotam@yehudae.net", 105, 105)
GUICtrlCreateLabel("בס""ד", 5, 10)
GUISetState(@SW_SHOW, $hGUI)

GUICtrlSetState($idButtonReloadDict, 32)

If _CheckIfTheDictIsInstalled() Then
	GUICtrlSetState($idButtonReloadDict, 16)
	GUICtrlSetData($idButtonChangeStatus, "הסר התקנה")
	GUICtrlSetData($idLabelStatus, "סטטוס: מותקן")
EndIf

HotKeySet("{ESC}", "_Exit")

While 1
	Switch GUIGetMsg()
		Case -3, $idButton_ExitMenu ;$GUI_EVENT_CLOSE
			ExitLoop
		Case $idButton_Site
			ShellExecute("https://l.yehudae.net/6mPtxy")
		Case $idButton_Updates
			ShellExecute("https://l.yehudae.net/W0BdXG")
		Case $idButton_JoinUs
			ShellExecute("https://l.yehudae.net/IOpKTA")
		Case $idButton_About
			MsgBox(BitOR(0x40, 0x100000), "מילון הבייניש הגדול", "התוכנה נוצרה באמצעות פרוייקט פיתוחי חותם", 10)
			; ShellExecute("https://l.yehudae.net/Cw5JYi")
		Case $idButtonReloadDict
			GUICtrlSetState($idButtonReloadDict, 128) ;$GUI_DISABLE
			GUICtrlSetState($idButtonChangeStatus, 128) ;$GUI_DISABLE
			_UpdateDict()
			GUICtrlSetState($idButtonReloadDict, 64) ;$GUI_ENABLE
			GUICtrlSetState($idButtonChangeStatus, 64) ;$GUI_ENABLE
		Case $idButtonChangeStatus
			GUICtrlSetState($idButtonReloadDict, 128) ;$GUI_DISABLE
			GUICtrlSetState($idButtonChangeStatus, 128) ;$GUI_DISABLE
			_ChangeInstallStatus()
			GUICtrlSetState($idButtonReloadDict, 64) ;$GUI_ENABLE
			GUICtrlSetState($idButtonChangeStatus, 64) ;$GUI_ENABLE
	EndSwitch
WEnd
GUIDelete($hGUI)




Func _Install()
	If _CheckIfTheDictIsInstalled() Then
		Return
	EndIf

	DirCreate($sConfigDirPath)
	IniWrite($sConfigPath, "General", "current_version", $sServerFileVersion)
	IniWrite($sConfigPath, "General", "last_update_date", _INetGetSource($slastUpdateHebrewDateUrl))

	Local $hDownload = InetGet($sTheDictionaryUrl, $sFilePath, $INET_FORCERELOAD, $INET_DOWNLOADBACKGROUND)
	Do
		Sleep(250)
	Until InetGetInfo($hDownload, $INET_DOWNLOADCOMPLETE)

	Local $aRegexRes = 0
	Local $iMax = 0
	Local $sName = ""
	Local $sRegisryValue = ""

	For $i = 1 To 100 Step 1
		$sRegisryValue = RegEnumVal($sMasterRegistryPath, $i)
		If @error <> 0 Then ExitLoop
		$aRegexRes = StringRegExp($sRegisryValue, "^(\d+)_(.+)_state$", 2)
		If UBound($aRegexRes) = 3 Then
			If $aRegexRes[1] > $iMax Then
				$iMax = $aRegexRes[1]
				$sName = $aRegexRes[2]
			EndIf
		EndIf
	Next

	Local $sPrefix = ($iMax + 1) & "_" & $sName

	RegWrite($sMasterRegistryPath, $sPrefix & "", "REG_SZ", $sFilePath)
	RegWrite($sMasterRegistryPath, $sPrefix & "_external", "REG_BINARY", 1)
	RegWrite($sMasterRegistryPath, $sPrefix & "_roamed", "REG_BINARY", 0)
	RegWrite($sMasterRegistryPath, $sPrefix & "_state", "REG_BINARY", 1)
	RegWrite($sMasterRegistryPath, $sPrefix & "_culturetag", "REG_SZ", "he-IL")

	RegWrite($sMasterRegistryPath, "PitucheyHotem_TBBD_ID", "REG_SZ", ($iMax + 1))

	$sOriginalFileVersion = $sServerFileVersion
EndFunc

Func _UnInstall()
	If Not _CheckIfTheDictIsInstalled() Then _
		Return

	Local $sName = ""
	Local $aRegexRes = 0

	For $i = 1 To 100 Step 1
		Local $sFilePath = RegEnumVal($sMasterRegistryPath, $i)
		If @error <> 0 Then ExitLoop
		$aRegexRes = StringRegExp($sFilePath, "^(\d+)_(.+)_state$", 2)
		If UBound($aRegexRes) = 3 Then
			$sName = $aRegexRes[2]
			ExitLoop
		EndIf
	Next

	Local $sPrefix = (RegRead($sMasterRegistryPath, "PitucheyHotem_TBBD_ID")) & "_" & $sName

	RegDelete($sMasterRegistryPath, $sPrefix & "")
	RegDelete($sMasterRegistryPath, $sPrefix & "_external")
	RegDelete($sMasterRegistryPath, $sPrefix & "_roamed")
	RegDelete($sMasterRegistryPath, $sPrefix & "_state")
	RegDelete($sMasterRegistryPath, $sPrefix & "_culturetag")

	RegDelete($sMasterRegistryPath, "PitucheyHotem_TBBD_ID")

	FileDelete($sFilePath)
	FileDelete($sConfigPath)

	$sOriginalFileVersion = "0.0.0"
EndFunc

Func _CheckIfTheDictIsInstalled()
	Return RegRead($sMasterRegistryPath, "PitucheyHotem_TBBD_ID") <> 0
EndFunc

Func _UpdateLastUpdateText()
	GUICtrlSetData($idLabelLastUpdate, "תאריך העדכון האחרון" & @CRLF & IniRead($sConfigPath, "General", "last_update_date", "<התוכנה לא הותקנה>"))
EndFunc

Func _ChangeInstallStatus()
	If _CheckIfTheDictIsInstalled() Then
		_UnInstall()
		GUICtrlSetState($idButtonReloadDict, 32)
		GUICtrlSetData($idButtonChangeStatus, "התקן את המילון")
		GUICtrlSetData($idLabelStatus, "סטטוס: לא מותקן")
		_UpdateLastUpdateText()
	Else
		_Install()
		GUICtrlSetState($idButtonReloadDict, 16)
		GUICtrlSetData($idButtonChangeStatus, "הסר התקנה")
		GUICtrlSetData($idLabelStatus, "סטטוס: מותקן")
		_UpdateLastUpdateText()
	EndIf
EndFunc

Func _CheckIfExistUpdate()
	If $sOriginalFileVersion < $sServerFileVersion Then
		Return True
	Else
		return False
	EndIf
EndFunc

Func _UpdateDict()
	If _CheckIfExistUpdate() Then
		FileDelete($sFilePath)

		Local $hDownload = InetGet($sTheDictionaryUrl, $sFilePath, $INET_FORCERELOAD, $INET_DOWNLOADBACKGROUND)
		Do
			Sleep(250)
		Until InetGetInfo($hDownload, $INET_DOWNLOADCOMPLETE)

		IniWrite($sConfigPath, "General", "current_version", $sServerFileVersion)
		IniWrite($sConfigPath, "General", "last_update_date", _INetGetSource($slastUpdateHebrewDateUrl))
		_UpdateLastUpdateText()
		MsgBox(BitOR(0x40, 0x100000), "מילון הבייניש הגדול", "המילון עודכן לגרסה " & $sServerFileVersion & " שיצאה ב" & IniRead($sConfigPath, "General", "last_update_date", "***"), 10)
		$sOriginalFileVersion = $sServerFileVersion
	Else
		MsgBox(BitOR(0x40, 0x100000), "מילון הבייניש הגדול", "אין עדכונים חדשים...", 10)
	EndIf
EndFunc

Func _Exit()
	Exit
EndFunc
