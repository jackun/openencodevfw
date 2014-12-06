#pragma once

//Virtualdub doesn't seem to load if fourcc is different then it is in registry after VIDC.*
#define FOURCC_OPEN mmioFOURCC('H','2','6','4')
#define FOURCC_H264 mmioFOURCC('H','2','6','4')

/* YUV 4:2:0 planar */
#define FOURCC_I420 mmioFOURCC('I','4','2','0')
#define FOURCC_IYUV mmioFOURCC('I','Y','U','V')
#define FOURCC_YV12 mmioFOURCC('Y','V','1','2')
/* YUV 4:2:2 planar */
#define FOURCC_YV16 mmioFOURCC('Y','V','1','6')
/* YUV 4:4:4 planar */
#define FOURCC_YV24 mmioFOURCC('Y','V','2','4')
/* YUV 4:2:0, with one Y plane and one packed U+V */
#define FOURCC_NV12 mmioFOURCC('N','V','1','2')
/* YUV 4:2:2 packed */
#define FOURCC_YUYV mmioFOURCC('Y','U','Y','V')
#define FOURCC_YUY2 mmioFOURCC('Y','U','Y','2')
#define FOURCC_UYVY mmioFOURCC('U','Y','V','Y')
#define FOURCC_HDYC mmioFOURCC('H','D','Y','C')

// possible colorspaces
#define RGB24	24
#define RGB32	32
#define YUY2	16
#define YV12	12

enum COLORMATRIX
{
	BT709_FULL = 0,
	BT601_FULL,
	BT709_LIMITED,
	BT601_LIMITED,
	COLORMATRIX_COUNT
};