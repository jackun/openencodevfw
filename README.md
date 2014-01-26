openencodevfw
=============

VFW encoder for AMD VCE h264 encoder. Usable with Virtualdub, Dxtory etc.

https://github.com/jackun/openencodevfw/archive/master.zip


https://dl.dropboxusercontent.com/u/235773/OpenEncodeVFW-bin.7z with sample confs.

Extra settings are saved to registry under `HKCU\Software\OpenEncodeVFW`

Installing
============

 * Unpack the archive somewhere, right click on `install.bat` and click `Run as Administrator`.

If it complains about missing files, try the more manual version:

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


Some setting descriptions
=======

 * `Fixed QP` basically keeps picture quality constant across all frames. Unpredictable (usually) file sizes.
 * `CBR` keeps constant bitrate so picture quality gets worse if there is frequently fast motion in video and bitrate is too low or wastes harddrive space if frame could have been compressed more.
 * `VBR` uses variable bitrate, tries to keep in target bitrate but rises bitrate a little bit if needed or lowers if frame can be compressed more.
 * [CABAC](http://en.wikipedia.org/wiki/Context-adaptive_binary_arithmetic_coding) is more efficient and resource intensive encoding option.
 * `Search range` is motion vector range. Specifies how wide the codec looks for moved pixels so it can just say that these pixels moved to x,y and just save that. Higher (max 36?) is better and more resource intensive encoding option.
 * `Profiles` / `levels`: start from http://en.wikipedia.org/wiki/H.264/MPEG-4_AVC#Profiles . Colorspace is limited to Y'UV422.
 * `Send FPS` sets encoder framerate properties to video framerate. Should be more efficient, but not all framerates are supported by encoder. Untick to treat all videos as having 30 fps.
 * `Speedy Math` tries to speed up OpenCL floating point math by making it less accurate, but should be good enough.

Probably not very accurate descriptions :P
