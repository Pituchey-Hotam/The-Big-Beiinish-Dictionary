
;==============================================================================================================
; Script Name:      Au3toCmd.au3
; Description:      Creates a CMD file from any AU3 file.
;                   The CMD file will contain the compiled version (A3X) of the AU3 input file
;                   and the AUTOIT3.EXE file as BASE64 data.
;                   This avoids the problem with the false positives of the virus scanners on EXE files.
;                   To avoid the short-term flashing of the CMD window, a shortcut is created on the desktop
;                   that runs in a minimized window. You may delete or move it.
;
; Syntax:           Au3toCmd (input-file)
;                   Default:  none
; Parameter:        Name of AU3 file (optional)
;                       can be set by using SHIFT+F8 in SciTE4AutoIt3 Editor.
;                       if parameter is empty, a FileOpenDialog prompts you.
;
; The execxution environment can be set by using the #AutoIt3Wrapper directives in the source code.
;     #AutoIt3Wrapper_Version=Prod/P/0/Beta/B/1
;     #AutoIt3Wrapper_UseX64=Y/1/N/0
;     #AutoIt3Wrapper_Icon=C:\users\...\anyname.ico
;
; IMPORTANT -- IMPORTANT -- IMPORTANT
; If you add the following line in the SciTEUser.properties file, you can run Au3toCmd via F7 (build) in the SciTE Editor:
; command.build.*.au3=start "anytitle" "C:\Users\...path to your...\Au3toCmd.au3" "$(FileNameExt)"
;
; Example:          Au3toCmd c:\testdir\testfile.au3
;
; Author:           Exit   ( http://www.autoitscript.com/forum/user/45639-exit )
; SourceCode:       http://www.autoitscript.com/forum/index.php?showtopic=201562  Version: 2022.09.01
; COPYLEFT:         © 2020 Freeware by "Exit"
;                   ALL WRONGS RESERVED
;==============================================================================================================

#Region ;**** Directives created by AutoIt3Wrapper_GUI ****
#AutoIt3Wrapper_Version=Prod
#AutoIt3Wrapper_UseX64=N
#AutoIt3Wrapper_AU3Check_Parameters=-d -w 1 -w 2 -w 3 -w 4 -w 5 -w 6 -w 7
#EndRegion ;**** Directives created by AutoIt3Wrapper_GUI ****

Global $_a2c_Debug = 0 ;  change to '1' for debugging informations on output console

#include <File.au3>

Global $sSourcepath, $sSourceData, $sTargetpath, $aPathSplit, $sDrive, $sDir, $sFileName, $sExtension, $sIconPath = "", $iIconNumber = 0, $sRDir
Exit _Main()

Func _Main()
	__DebugInfo()
	If Not _Sourcepath() Then Return SetError(1, 0, 0)
	If Not _IconPath() Then Return SetError(2, 0, 0)
	If Not _Targetpath() Then Return SetError(3, 0, 0)
EndFunc   ;==>_Main

Func _Sourcepath()
	If $cmdline[0] > 0 Then $sSourcepath = $cmdline[1]
	If Not FileExists($sSourcepath) Then
		Beep(1000, 80)
		$sSourcepath = FileOpenDialog(" Enter AU3 Inputfile for Au3toCmd Application ", "", "Autoit Files(*.au3)", 3)
		If @error Then Return SetError(5, MsgBox(16 + 262144, Default, "Error: No Inputfile given", 3), 0)
	EndIf
	$sSourcepath = _PathFull($sSourcepath)
	$aPathSplit = _PathSplit($sSourcepath, $sDrive, $sDir, $sFileName, $sExtension)
	FileChangeDir($sDrive & $sDir)
	$sSourceData = FileRead($sSourcepath)
	__CW("Sourcepath: " & $sSourcepath & @LF)
	Return 1
EndFunc   ;==>_Sourcepath

Func _IconPath()
	Local $aTemp = StringRegExp(@CRLF & $sSourceData, "(?m)\n(\s*)#AutoIt3Wrapper_Icon=(.*)", 3)
	If IsArray($aTemp) And FileExists($aTemp[UBound($aTemp) - 1]) Then
		$sIconPath = $aTemp[UBound($aTemp) - 1]
	ElseIf FileExists($sDrive & $sDir & $sFileName & ".ico") Then
		$sIconPath = $sDrive & $sDir & $sFileName & ".ico"
	Else
		$sIconPath = @WindowsDir & "\system32\shell32.dll"
;~ 		$iIconNumber = 71
		$iIconNumber = 132
	EndIf
	__CW("IconNumber: " & $iIconNumber & "  IconPath: " & $sIconPath & @CRLF)
	Return 1
EndFunc   ;==>_IconPath

Func _Targetpath()
	Local $sA3Dir, $sA3Ver, $hTargetpath, $sTargetdir, $t, $th, $x64 = 0, $beta = 0, $svdate
	$sTargetpath = $sDrive & $sDir & $sFileName & ".cmd"
	$sTargetdir = $sDrive & $sDir
	FileDelete($sTargetpath)
	$beta = StringRegExp(@CRLF & $sSourceData, "(?m)\n(\s*)#AutoIt3Wrapper_Version=[bByY1]", 0)
	$sA3Dir = "C:\Users\Yehuda\Documents\Softwares\AutoIt3\" ;RegRead("HKLM\SOFTWARE" & ((@OSArch = 'X64') ? "\Wow6432Node" : "") & "\AutoIt v3\AutoIt", ($beta ? "beta" : "") & "InstallDir")
	$sA3Ver = "3.0.0" ;RegRead("HKLM\SOFTWARE" & ((@OSArch = 'X64') ? "\Wow6432Node" : "") & "\AutoIt v3\AutoIt", ($beta ? "beta" : "") & "Version")
	__CW("Regread A3Dir: " & $sA3Dir & "  Version: " & $sA3Ver & @CRLF)
	If Not (FileExists($sA3Dir & "\autoit3.exe") And FileExists($sA3Dir & "\au3check.exe") And FileExists($sA3Dir & "\Aut2Exe\Aut2exe.exe")) Then Return SetError(9, MsgBox(16 + 262144, Default, "Error: Autoit " & ($beta ? "Beta Version " : "") & "not installed on this system.", 0), 0)
	$x64 = StringRegExp(@CRLF & $sSourceData, "(?m)\n(\s*)#AutoIt3Wrapper_UseX64=[yY1]", 0)
	__CW("X64: >" & $x64 & "<  beta: >" & $beta & "< A3Dir: " & $sA3Dir & @LF)
	If ShellExecuteWait($sA3Dir & "\au3check.exe", ' -q "' & $sSourcepath & '"', "", "", @SW_HIDE) Then Return SetError(10, MsgBox(16 + 262144, Default, "Error: Input file """ & $sSourcepath & """ has Errors according to Au3Check.exe. ", 0), 0)
	If ShellExecuteWait($sA3Dir & "\Aut2Exe\Aut2exe" & ($x64 ? '_x64' : '') & ".exe", "/In """ & $sSourcepath & """ /out """ & $sTargetpath & ".sa3x""  " & ($x64 ? '/X64' : ' ')) Then Return SetError(11, MsgBox(16 + 262144, Default, "Error : Cannot create target file """ & $sTargetpath & ".sa3x"" ", 0), 0)
	$svdate = "_" & StringLeft(FileGetTime($sA3Dir & "\Autoit3" & ($x64 ? '_x64' : '') & ".exe", 0, 1), 8)
	FileCopy($sA3Dir & "\Autoit3" & ($x64 ? '_x64' : '') & ".exe", $sTargetpath & ".sprog")
	$hTargetpath = FileOpen($sTargetpath, $FO_APPEND)
	FileWriteLine($hTargetpath, '@if not DEFINED _ (set _=g&start "" /min "%~f0" %*&exit)else (%_%oto %_%)')
	RunWait(@ComSpec & " /c " & 'certutil -encode -f "' & $sTargetpath & '.sa3x" "' & $sTargetpath & '.a3x"', "", @SW_HIDE)
	$t = FileRead($sTargetpath & '.a3x')
	FileDelete($sTargetpath & '.sa3x')
	FileDelete($sTargetpath & '.a3x')
	FileWriteLine($hTargetpath, $t)
	FileWriteLine($hTargetpath, _
			':g ' _
			 & @CRLF & 'cd /D %~dp0' _
			 & @CRLF & 'set r=%appdata%\Au3toCmd\' _
			 & @CRLF & 'set r1=%appdata%\Au3toCmdTmp\' _
			 & @CRLF & 'set n=%~n0' _
			 & @CRLF & 'set ver=' & $sA3Ver & $svdate _
			 & @CRLF & 'set x64=' & ($x64 ? "_64" : "") _
			 & @CRLF & 'if not exist "%r%exe\%ver%\AutoIt3%x64%.exe" (' _
			 & @CRLF & '  mkdir "%r%exe\%ver%\"' _
			 & @CRLF & '  more +5 %0 >~~' _
			 & @CRLF & '  certutil -decode -f ~~ "%r%exe\%ver%\AutoIt3%x64%.exe"' _
			 & @CRLF & '  del ~~' _
			 & @CRLF & ')' _
			 & @CRLF & 'set hash=' & _hash($sTargetdir) _
			 & @CRLF & 'call :ts %0 "%r%a3x\%hash%\%n%.a3x"' _
			 & @CRLF & 'if %t1% geq %t2% (' _
			 & @CRLF & 'mkdir "%r%a3x\%hash%\"' _
			 & @CRLF & 'certutil -decode -f %0 "%r%a3x\%hash%\%n%.a3x"' _
			 & @CRLF & ')' _
			 & @CRLF & 'set wdir=%~dp0' _
			 & @CRLF & 'if not "%wdir: =%"=="%wdir%" set wdir=%~sdp0' _
			 & @CRLF & 'wmic process call create ''"%r%exe\%ver%\AutoIt3%x64%.exe" "%r%a3x\%hash%\%n%.a3x" "%*"'' ,"%wdir%" ' _
			 & @CRLF & 'if not exist "%r%%date:~6,4%" (' _
			 & @CRLF & '  dir /B /S "%r%*.*" >~ & timeout /T 5 & del /S /Q "%r%*.exe" "%r%*.a3x" "%r%olddir.txt" ' _
			 & @CRLF & '  xcopy %r%*.* %r1% /S & rmdir /S /Q %r% & xcopy %r1%*.* %r% /S /Y & rmdir /S /Q %r1% ' _
			 & @CRLF & '  mkdir "%r%%date:~6,4%" & copy ~ "%r%%date:~6,4%\olddir.txt" & del ~' _
			 & @CRLF & ')' _
			 & @CRLF & 'rem pause' _
			 & @CRLF & 'exit' _
			 & @CRLF & ':ts t1 t2' _
			 & @CRLF & 'set t1=%~t1' _
			 & @CRLF & 'set t2=%~t2' _
			 & @CRLF & 'set t1=%t1:~3,2%%t1:~0,2%%t1:~11,2%%t1:~14,2%' _
			 & @CRLF & 'set t2=%t2:~3,2%%t2:~0,2%%t2:~11,2%%t2:~14,2%' _
			 & @CRLF & 'goto :eof' _
			)
	RunWait(@ComSpec & " /c " & 'certutil -encode -f "' & $sTargetpath & '.sprog" "' & $sTargetpath & '.prog"', "", @SW_HIDE)
	$t = FileRead($sTargetpath & '.prog')
	FileDelete($sTargetpath & '.sprog')
	FileDelete($sTargetpath & '.prog')
	FileWriteLine($hTargetpath, $t)
	FileClose($hTargetpath)

	$t = StringReplace(FileRead($sTargetpath), @CRLF, @LF, 0, 2)
	$th = FileOpen($sTargetpath, 2)
	FileWrite($th, $t)
	FileClose($th)

	If Not FileCreateShortcut($sTargetpath, @DesktopDir & "\" & $sFileName & ".lnk", $sDrive & $sDir, "", "", $sIconPath, "", $iIconNumber, 7) Then Return SetError(16, MsgBox(16 + 262144, Default, "Unable to create shortcut", 0), 0)
	__CW("$sTargetpath: " & $sTargetpath & @LF)
	MsgBox(64 + 262144, Default, $sTargetpath & "     and " & @CRLF & @DesktopDir & "\" & $sFileName & ".lnk  created." & @CRLF & @CRLF & "X64 Mode=" & ($x64 ? "Yes" : "No") & "      Beta Mode=" & ($beta ? "Yes" : "No"), 0)
	Return 1
EndFunc   ;==>_Targetpath

Func __CW($sText)
	If Not Eval("_a2c_Debug") Then Return
;~  FileWriteLine(@ScriptFullPath & ".console.txt", $sText)
	ConsoleWrite($sText & @CRLF)
EndFunc   ;==>__CW

Func __DebugInfo()
	FileDelete(@ScriptFullPath & ".console.txt")
	If @Compiled Then $_a2c_Debug = 0
	If Not Eval("_a2c_Debug") Then Return
	__CW("============ Start of DebugInfo ===============")
	__CW("Au3toCmd Version: " & (StringRegExp(FileRead(@ScriptFullPath), "(?i)Version: (.*)", 1))[0])
	__CW("@ScriptFullPath: " & @ScriptFullPath)
	__CW("@AutoItExe: " & @AutoItExe)
	__CW("@AutoItVersion: " & @AutoItVersion)
	__CW("@AutoItX64: " & @AutoItX64)
	__CW("@CPUArch: " & @CPUArch)
	__CW("@OSArch: " & @OSArch)
	__CW("@OSBuild: " & @OSBuild)
	__CW("@OSLang: " & @OSLang)
	__CW("@OSType: " & @OSType)
	__CW("@OSVersion: " & @OSVersion)
	__CW("@OSServicePack: " & @OSServicePack)
	__CW("@UserName: " & @UserName)
	__CW("@UserProfileDir: " & @UserProfileDir)
	__CW("============ End of DebugInfo ===============")
EndFunc   ;==>__DebugInfo

Func _hash($data = @ScriptDir)
	Local $shc = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	Local $ahc = StringSplit($shc, "")
	Local $adir = StringSplit($data, "")
	Local $hash = $ahc[Mod($adir[0], $ahc[0]) + 1]
	Local $sum1 = 0, $sum2 = 0, $sum3 = 0
	For $i = 1 To $adir[0]
		$sum1 += Asc($adir[$i])
		$sum2 += Mod(Asc($adir[$i]) * $i, $ahc[0])
		$sum3 += Mod(Asc($adir[$i]) * Asc($adir[$i - 1]) * $i, $ahc[0])
	Next
	$hash &= $ahc[Mod($sum1, $ahc[0]) + 1]
	$hash &= $ahc[Mod($sum2, $ahc[0]) + 1]
	$hash &= $ahc[Mod($sum3, $ahc[0]) + 1]
	ConsoleWrite(" Hash>" & $hash & "<   " & $adir[0] & "  " & $data & @CRLF)
	Return $hash
EndFunc   ;==>_hash

; End of Au3toCmd.au3 script
