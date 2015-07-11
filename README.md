# OpenEncodeVFW

VFW encoder for AMD VCE h264 encoder. Usable with Virtualdub, Dxtory etc.

https://github.com/jackun/openencodevfw/archive/master.zip

Extra settings are saved to registry under `HKCU\Software\OpenEncodeVFW`

**As OpenEncode has been deprecated by AMD for a long time already, it appears the support has been finally dropped from Catalyst 15.7**

**You may need to install [MSVC++ 2013 runtimes](http://www.microsoft.com/en-us/download/details.aspx?id=40784).**
Last [MSVC++2010 commit](https://github.com/jackun/openencodevfw/tree/d6c7c53b61af9447b30d6d6d86be8725801d0fb7).

**NOTE: You need to install x86 version for 32bit codec even if your Windows is 64 bit.**

**NOTE: VCE on cards/APUs prior to Tonga only go up to 1080p and solid 1080p60 recording can not be guaranteed (yet) unfortunately.**

## Compatible hardware

AMD's GCN based cards and APUs.
From [AMD's blog](http://developer.amd.com/community/blog/2014/02/19/introducing-video-coding-engine-vce/):

| VCE Version | Product Family | Distinguishing Features |
| :---------: |:--------------:| -----------------------|
| VCE 1.0     | Radeon HD 7900 series/Radeon R9 280X dGPU | First release: AVC – I,P and DEM |
|             | Radeon HD 7800 series dGPU | |
|             | Radeon R9 270X/270 dGPU    | |
|             | Radeon HD 7700 series/Radeon R7 250X dGPU | |
|             | A10 – 58XX (and other variations) APU | |
|             | A10 – 68XX APU | |
| | | |
| VCE 2.0     | Radeon R9 390x/390/290x/290 dGPU | SVC (temporal) + B-pictures + DEM improvements |
|             | Radeon R7 260X/260 dGPU | |
|             | A10 – 7850K APU         | |
|             | A4-5350, A4-3850, or E1-2650 APU | |
|             | A4-1200/A6-1450 APU | |
| | | |
| VCE 3.0     | Radeon R9 Fury/285 dGPU | 4K |


## Installing

 * Unpack the archive somewhere, right click on `install.bat` and click `Run as Administrator`.

If it complains about missing files, try the more manual version:

 * Unpack the archive somewhere, open command prompt as administrator by typing `cmd` to start menu or "Metro" and press SHIFT+CTRL+Enter or right click on the icon and click `Run as Administrator`. 
 * Go to unpacked folder by typing into opened command prompt `cd some\where\OpenEncodeVFW-bin`. 
 * Type `install.bat` and press enter to run the installer.

## Uninstalling

If uninstaller fails its job, manually remove these registry keys:

	HKLM\SYSTEM\CurrentControlSet\Control\MediaResources\icm\VIDC.H264
	HKLM\Software\Microsoft\Windows NT\CurrentVersion\drivers.desc\OPENENCODEVFW.DLL
	HKLM\Software\Microsoft\Windows NT\CurrentVersion\Drivers32\VIDC.H264
	HKLM\Software\Wow6432Node\Microsoft\Windows NT\CurrentVersion\drivers.desc\OPENENCODEVFW.DLL
	HKLM\Software\Wow6432Node\Microsoft\Windows NT\CurrentVersion\Drivers32\VIDC.H264
	
and `OPENENCODEVFW.DLL` in %WINDIR%\syswow64 or %WINDIR%\system32

## Recommended usage
 * 32 bit input format
 * width/height multiples of 2

## Some setting descriptions

 * `Fixed QP` basically keeps picture quality constant across all frames.
 * `CBR` keeps constant bitrate so picture quality gets worse if there is frequently fast motion in video and bitrate is too low or wastes harddrive space if frame could have been compressed more. Seems to fluctuate too much though.
 * `VBR` uses variable bitrate, tries to keep in target bitrate but rises bitrate a little bit if needed or lowers if frame can be compressed more.
 * [CABAC](http://en.wikipedia.org/wiki/Context-adaptive_binary_arithmetic_coding) is more efficient and resource intensive encoding option.
 * `Search range` is motion vector range. Specifies how wide the codec looks for moved pixels so it can just say that these pixels moved to x,y and just save that. Higher (max 36?) is better and more resource intensive encoding option.
 * `Profiles` / `levels`: start from http://en.wikipedia.org/wiki/H.264/MPEG-4_AVC#Profiles . Colorspace is limited to Y'UV420.

Probably not very accurate descriptions :P

Also:

 * `Send FPS` sets encoder framerate properties to video framerate, but not all framerates are supported by encoder. Untick to treat all videos as having 30 fps, but this may make encoding inefficient and increase bitrate more than necessary.
 * `Speedy Math` tries to speed up OpenCL floating point math by making it less accurate, but should be good enough.
 * `Switch byte order` : for the rare case when input bitmap is RGB(A) instead of BGR(A).
 * `Header insertion` : adds SPS/PPS to every frame, may make cutting/splitting video easier. More of a 'debug' feature.

**Quickset** buttons for speed vs quality:

 * `Speed` : encodes 1080p at 60+ fps (theoretical max 80+)
 * `Balanced` : encodes 1080p at 40+ fps
 * `Quality` : encodes 1080p at 30+ fps (can probably do 720p@60)


With newer AMD cards (hawaii+), seem to support B-frames, though VCE may not actually generate B-frames with OpenVideo, and AVI kinda sucks with these ([see](http://guru.multimedia.cx/avi-and-b-frames/)). You may need to remux to MKV/MP4 for better audio/video sync.
(Also [maybe](https://trac.ffmpeg.org/ticket/1979#comment:7) `ffmpeg -fflags +genpts`)
