openencodevfw
=============

VFW encoder for AMD VCE h264 encoder

https://github.com/jackun/openencodevfw/archive/master.zip

Installing
============

 * Unpack the archive somewhere, open command prompt as administrator by typing `cmd` to start menu or "Metro" and press SHIFT+CTRL+Enter or right click on the icon and click `Run as Administrator`. 
 * Go to unpacked folder by typing into opened command prompt `cd some\where\OpenEncodeVFW-bin`. 
 * Type `install.bat` and press enter to run the installer.

Uninstalling
============
If uninstaller fails its job, manually remove these registry keys:

	HKLM\SYSTEM\CurrentControlSet\Control\MediaResources\icm\VIDC.H264
	HKLM\Software\Microsoft\Windows NT\CurrentVersion\drivers.desc\OPENENCODEVFW.DLL
	HKLM\Software\Microsoft\Windows NT\CurrentVersion\Drivers32\VIDC.H264
	
and `OPENENCODEVFW.DLL` in %WINDIR%\syswow64 or %WINDIR%\system32
