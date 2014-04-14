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
	echo === 64 Bit Operating System detected.
	echo ===

	REM (With how currently INF is set up) setupapi seems to look for DLLs in the same folder as INF.
	REM So just copy DLLs (aka do the installer's work twice :P) and run INF from dest. dir
	REM Copy INF for uninstaller.
	copy OpenEncode.inf %windir%\system32\
	copy OpenEncode64\OPENENCODEVFW.DLL %windir%\system32\
	
	copy OpenEncode.inf %windir%\SysWOW64\
	copy OpenEncode32\OPENENCODEVFW.DLL %windir%\SysWOW64\
	
	rundll32 setupapi.dll,InstallHinfSection DefaultInstall 0 %windir%\System32\OpenEncode.inf
	
	REM Because Windows-On-Windows, you have to run this from within syswow64 dir
	REM so that windows would know it is 32bit version.
	cd /d %windir%\SysWOW64\
	rundll32 setupapi.dll,InstallHinfSection DefaultInstall 0 %windir%\SYSWOW64\OpenEncode.inf
)

popd
pause
