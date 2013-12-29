@echo off
REM stolen from lagarith

Set RegQry=HKLM\Hardware\Description\System\CentralProcessor\0
set OLDDIR=%CD%

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
	REM No 64bit in the package yet
	REM copy OpenEncode64\OPENENCODEVFW.DLL %windir%\system32\
	
	REM Because something weird with windows, you have to run this from within syswow64 dir
	copy OpenEncode.inf %windir%\SysWOW64\
	copy OpenEncode32\OPENENCODEVFW.DLL %windir%\SysWOW64\

	REM Don't do this, probably confuses 32bit uninstaller. Use different INF.
	REM cd /d %windir%\system32\
	REM rundll32 setupapi.dll,InstallHinfSection DefaultInstall 0 %windir%\system32\OpenEncode.inf

	cd /d %windir%\SysWOW64\
	rundll32 setupapi.dll,InstallHinfSection DefaultInstall 0 %windir%\SYSWOW64\OpenEncode.inf
)

chdir /d %OLDDIR%
pause
