@echo off
REM stolen from lagarith

Set RegQry=HKLM\Hardware\Description\System\CentralProcessor\0
REM set OLDDIR=%CD%
pushd %~dp0%

REG.exe Query %RegQry% > checkOS.txt
 
Find /i "x86" < CheckOS.txt > StringCheck.txt
 
If %ERRORLEVEL% == 0 (
	del StringCheck.txt
	del CheckOS.txt 
	Echo "32 Bit Operating system detected, installing 32 bit OpenEncode version"
	copy openencode.inf %windir%\system32\
	copy OpenEncode32\OPENENCODEVFW.DLL %windir%\system32\

	cd /d %windir%\system32\
	rundll32 setupapi.dll,InstallHinfSection DefaultInstall 0 %windir%\system32\OpenEncode.inf
) ELSE (
	del StringCheck.txt
	del CheckOS.txt 

	echo ===
	echo === 64 Bit Operating System detected, 
	echo === but installing only 32 bit OpenEncode version.
	echo === Edit install.bat if you need 64 bit too.
	echo ===

	copy OpenEncode.inf %windir%\system32\
	REM ### Remove REM from next line if you need 64bits too and 2 more lines below ###
	REM copy OpenEncode64\OPENENCODEVFW.DLL %windir%\system32\
	
	REM Because something weird with windows, you have to run this from within syswow64 dir
	copy OpenEncode.inf %windir%\SysWOW64\
	copy OpenEncode32\OPENENCODEVFW.DLL %windir%\SysWOW64\
	REM Probably confuses 32bit uninstaller. Add "64" suffix to INF and DLL?
	REM ### Remove REM from next 2 lines if you need 64bits too ###
	REM cd /d %windir%\system32\
	REM rundll32 setupapi.dll,InstallHinfSection DefaultInstall 0 %windir%\system32\OpenEncode.inf

	cd /d %windir%\SysWOW64\
	rundll32 setupapi.dll,InstallHinfSection DefaultInstall 0 %windir%\SYSWOW64\OpenEncode.inf
)

popd
pause
