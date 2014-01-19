openencodevfw
=============

VFW for AMD VCE h264 encoder

Binaries: https://dl.dropboxusercontent.com/u/235773/OpenEncodeVFW-bin.7z

Installing
============

Unpack the archive somewhere, open command prompt as administrator and go to unpacked folder `cd some\where\OpenEncodeVFW-bin`. Run the `install.bat`

Uninstalling
============
If uninstaller fails its job, manually remove these registry keys:

	HKLM\SYSTEM\CurrentControlSet\Control\MediaResources\icm\VIDC.H264
	HKLM\Software\Microsoft\Windows NT\CurrentVersion\drivers.desc\OPENENCODEVFW.DLL
	HKLM\Software\Microsoft\Windows NT\CurrentVersion\Drivers32\VIDC.H264
	
and `OPENENCODEVFW.DLL` in %WINDIR%\syswow64 or %WINDIR%\system32
