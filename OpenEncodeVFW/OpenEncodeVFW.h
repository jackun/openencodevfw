#ifndef _MAIN_HEADER
#define _MAIN_HEADER

#include "OVEncodeDyn.h"
#include "OVEncodeTypes.h"
#include "CL\cl.h"
#include "OvEncodeTypedef.h"
#include "perf.h"
#include "bitstream.h"
#include "clconvert.h"

using namespace std;

#ifdef OPENENCODEVFW_EXPORTS
#define OPENENCODEVFW_API __declspec(dllexport)
#else
#define OPENENCODEVFW_API __declspec(dllimport)
#endif

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

// y must be 2^n
#define align_round(x,y) ((((unsigned int)(x))+(y-1))&(~(y-1)))

#define return_badformat() return (DWORD)ICERR_BADFORMAT;

#define MIN(a, b) (((a)<(b)) ? (a) : (b))
#define MAX(a, b) (((a)>(b)) ? (a) : (b))
#define CLIP(v, min, max) (((v)<(min)) ? (min) : ((v)>(max)) ? (max) : (v))

/* Registry */
#define OVE_REG_KEY    HKEY_CURRENT_USER
#define OVE_REG_PARENT L"Software"
#define OVE_REG_CHILD  L"OpenEncodeVFW"
#define OVE_REG_CLASS  L"config"

extern CRITICAL_SECTION ove_CS;

typedef struct _OVConfigCtrl
{
	uint32                    height;
	uint32                    width;
	OVE_ENCODE_MODE                 encodeMode;

	OVE_PROFILE_LEVEL               profileLevel; /**< Profile Level                       */

	OVE_PICTURE_FORMAT              pictFormat;   /**< Profile format                      */
	OVE_ENCODE_TASK_PRIORITY        priority;     /**< priority settings                   */

	OVE_CONFIG_PICTURE_CONTROL      pictControl;  /**< Picture control                     */
	OVE_CONFIG_RATE_CONTROL         rateControl;  /**< Rate contorl config                 */
	OVE_CONFIG_MOTION_ESTIMATION    meControl;    /**< Motion Estimation settings          */
	OVE_CONFIG_RDO                  rdoControl;   /**< Rate distorsion optimization control*/
} OvConfigCtrl, far * pConfig;

/******************************************************************************/
/* Input surface used for encoder                                             */
/******************************************************************************/
#define			            MAX_INPUT_SURFACE      1 //VFW can't do multiple encodes at once anyway?

typedef struct OVDeviceHandle
{
	ovencode_device_info *deviceInfo; /**< Pointer to device info        */
	uint32                numDevices; /**< Number of devices available   */
	cl_platform_id        platform;   /**< Platform                      */
}OVDeviceHandle;

/******************************************************************************/
/* Encoder Handle for sharing context between create process and destroy      */
/******************************************************************************/
typedef struct OVEncodeHandle
{
	ove_session          session;       /**< Pointer to encoder session   */
	OPMemHandle		     inputSurfaces[MAX_INPUT_SURFACE]; /**< input buffer  */
	cl_command_queue     clCmdQueue[2];    /**< command queue  */
}OVEncodeHandle;

typedef std::map<cl_device_id, std::wstring> DeviceMap;

/* Return the maximum number of bytes a single compressed frame can occupy */
LRESULT x264vfw_compress_get_size(LPBITMAPINFOHEADER lpbiOut);
bool isH264iFrame(int8 *frame);
void ConvertRGB24toYV12_SSE2(const uint8 *src, uint8 *ydest, uint8 *udest, uint8 *vdest, unsigned int w, unsigned int h);
void ConvertRGB32toYV12_SSE2(const uint8 *src, uint8 *ydest, uint8 *udest, uint8 *vdest, unsigned int w, unsigned int h);
void ff_rgb24toyv12_c(const uint8 *src, uint8 *ydst, uint8 *udst,
				   uint8 *vdst, int width, int height, int lumStride,
				   int chromStride, int srcStride, int32 *rgb2yuv);
void RGBtoNV12 (const uint8 * rgb,
	uint8 * yuv,
	unsigned rgbIncrement,
	uint8 flip, uint8 isBGR,
	int srcFrameWidth, int srcFrameHeight, uint32 alignedWidth);

class CodecInst {
public:
	Logger *mLog;
	bool mMsgBox;
	bool mWarnedBuggy;
	FILE* mRaw;
	int started;			//if the codec has been properly initalized yet
	
	unsigned int mLength;
	unsigned int mWidth;
	unsigned int mHeight;
	unsigned int mFormat;	//input format for compressing, output format for decompression. Also the bitdepth.
	unsigned int mCompression;
	uint32 mAlignedSurfaceWidth;
	uint32 mAlignedSurfaceHeight;
	int32  mHostPtrSize;
	int32  mIDRFrames;
	bool   mProfKernels;
	
	unsigned int mCompressed_size;
	clConvert *mCLConvert;
	bool mUseCLConv;		// Use openCL on GPU for rgb-to-nv12 or just cpu
	bool mUseCPU;			// Use openCL on CPU for RGB to NV12 conversion
	bool mDialogUpdated;	// Used with configuration dialog to avoid loop-de-loops
	
	/* ICM_COMPRESS_FRAMES_INFO params */
	int frame_total;
	uint32 fps_num;
	uint32 fps_den;
	uint32 mFrameNum; //may overflow, don't care (maybe VFW does)

	cl_device_id clDeviceID;
	
	cl_context			mCpuCtx;
	cl_device_id		mCpuDev;
	cl_command_queue	mCpuCmdQueue[2];
	
	/**************************************************************************/
	/* Create profile counters                                                */
	/**************************************************************************/
	//OVprofile perfCounter;
	OVDeviceHandle		mDeviceHandle;
	OPContextHandle		mOveContext;
	/**************************************************************************/
	/* Create encoder handle                                                  */
	/**************************************************************************/
	OVEncodeHandle		mEncodeHandle;

	OVprofile			mProfile;

	bool status;

	/**************************************************************************/
	/* Currently the OpenEncode support is only for vista and w7              */
	/**************************************************************************/
	bool isVistaOrNewer;
	OvConfigCtrl            mConfigCtrl;
	map<string,int32>		mConfigTable;
	DeviceMap mDevList;

	//H264 ES parser
	Parser *mParser;
	bool mHasIDR;

	CodecInst();
	~CodecInst();

	DWORD GetState(LPVOID pv, DWORD dwSize);
	DWORD SetState(LPVOID pv, DWORD dwSize);
	DWORD Configure(HWND hwnd);
	DWORD GetInfo(ICINFO* icinfo, DWORD dwSize);

	DWORD CompressQuery(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
	DWORD CompressGetFormat(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
	DWORD CompressBegin(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
	DWORD CompressGetSize(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
	DWORD Compress(ICCOMPRESS* icinfo, DWORD dwSize);
	DWORD CompressEnd();
	DWORD CompressFramesInfo(ICCOMPRESSFRAMES *);

	DWORD DecompressQuery(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
	DWORD DecompressGetFormat(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
	DWORD DecompressBegin(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
	DWORD Decompress(ICDECOMPRESS* icinfo, DWORD dwSize);
	DWORD DecompressGetPalette(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
	DWORD DecompressEnd();

	BOOL QueryConfigure();

	void LogMsg(bool msgBox, const wchar_t *psz_fmt, ...);

	bool readRegistry();
	bool saveRegistry();
	void quickSet(int qs);

	DeviceMap getDeviceList();
	bool createCPUContext(cl_platform_id platform);

	/** 
	 *******************************************************************************
	 *  @fn     prepareConfigMap
	 *  @brief  configuration mapping table, used for mapping values from user     
	 *          configuration to config control structure 
	 *           
	 *  @param[in/out] pConfigTable   : Pointer to the configuration map table
	 *          
	 *  @return bool : true if successful; otherwise false.
	 *******************************************************************************
	 */
	void prepareConfigMap(bool quickset = false);

	/** 
	 *******************************************************************************
	 *  @fn     readConfigFile
	 *  @brief  Reading in user-specified configuration file
	 *           
	 *  @param[in] fileName           : user specified configuration file name
	 *  @param[in/out] pConfig       : Pointer to the configuration structure
	 *  @param[in/out] pConfigTable   : Pointer to the configuration map table
	 *          
	 *  @return bool : true if successful; otherwise false.
	 *******************************************************************************
	 */
	bool readConfigFile(int8 *fileName, OvConfigCtrl *pConfig,
					std::map<std::string,int32>* pConfigTable);
	/** 
	 *******************************************************************************
	 *  @fn     encodeSetParam
	 *  @brief  Setting up configuration parameters
	 *           
	 *  @param[in/out] pConfig   : Pointer to the configuration structure 
	 *  @param[in] pConfigTable  : Pointer to the configuration map table
	 *          
	 *  @return void
	 *******************************************************************************
	 */
	void encodeSetParam(OvConfigCtrl *pConfig, std::map<std::string,int32>* pConfigTable);

	/** 
	 *******************************************************************************
	 *  @fn     setEncodeConfig
	 *  @brief  This function sets the encoder configuration by using user 
	 *          supplied configuration information from .cfg file
	 *           
	 *  @param[in] session : Encoder session for which encoder configuration to be
	 *                       set
	 *  @param[in] pConfig : pointer to the user configuration from .cfg file
	 *          
	 *  @return bool : true if successful; otherwise false.
	 *******************************************************************************
	 */
	bool setEncodeConfig(ove_session session, OvConfigCtrl *pConfig);

	/** 
	 *******************************************************************************
	 *  @fn     getDevice
	 *  @brief  returns the platform and devices found
	 *           
	 *  @param[in/out] deviceHandle : Hanlde for the device information
	 *          
	 *  @return bool : true if successful; otherwise false.
	 *******************************************************************************
	 */
	bool getDevice(OVDeviceHandle *deviceHandle);

	/** 
	 *******************************************************************************
	 *  @fn     getDeviceCap
	 *  @brief  This function returns the device capabilities.
	 *           
	 *  @param[in] oveContext   : Encoder context 
	 *  @param[in] oveDeviceID  : Device ID
	 *  @param[out] encodeCaps : pointer to encoder capabilities structure
	 *          
	 *  @return bool : true if successful; otherwise false.
	 *******************************************************************************
	 */
	bool getDeviceCap(OPContextHandle oveContext,uint32 oveDeviceID,
				OVE_ENCODE_CAPS *encodeCaps);

	/** 
	 *******************************************************************************
	 *  @fn     getDeviceInfo
	 *  @brief  returns device information
	 *           
	 *  @param[out] deviceInfo  : Device info
	 *  @param[out] numDevices  : Number of devices present
	 *          
	 *  @return bool : true if successful; otherwise false.
	 *******************************************************************************
	 */
	bool getDeviceInfo(ovencode_device_info **deviceInfo, uint32 *numDevices);

	/** 
	 *******************************************************************************
	 *  @fn     gpuCheck
	 *  @brief  Checks for GPU present or not
	 *           
	 *  @param[in] platform : Platform id
	 *  @param[out] dType   : Device type returned GPU/CPU
	 *          
	 *  @return bool : true if successful; otherwise false.
	 *******************************************************************************
	 */
	bool gpuCheck(cl_platform_id platform,cl_device_type* dType);

	/** 
	 *******************************************************************************
	 *  @fn     getPlatform
	 *  @brief  Get platform to run
	 *           
	 *  @param[in] platform : Platform id
	 *          
	 *  @return bool : true if successful; otherwise false.
	 *******************************************************************************
	 */
	bool getPlatform(cl_platform_id &platform);

	/** 
	 *******************************************************************************
	 *  @fn     yuvToNV12
	 *  @brief   Read yuv video file and converts it to NV12 format
	 *           
	 *  @param[in] fr  : pointer to the input picture data 
	 *  @param[in] uiHeight : video frame height
	 *  @param[in] uiWidth  : video frame width
	 *  @param[in] alignedSurfaceWidth  : aligned frame width
	 *  @param[out] *pBitstreamData : input surface buffer pointer
	 *          
	 *  @return bool : true if successful; otherwise false.
	 *******************************************************************************
	 */

	bool yuvToNV12(const uint8 *inData, uint32 uiHeight, uint32 uiWidth, 
				   uint32 alignedSurfaceWidth, int8 *pBitstreamData);
	bool yv12ToNV12(const uint8 *inData, uint32 uiHeight, uint32 uiWidth, 
				   uint32 alignedSurfaceWidth, int8 *pBitstreamData);

	bool nv12ToNV12Aligned(const uint8 *inData, uint32 uiHeight, uint32 uiWidth, 
			   uint32 alignedSurfaceWidth, int8 *pBitstreamData);
	/** 
	 *******************************************************************************
	 *  @fn     encodeCreate
	 *  @brief  Creates encoder context 
	 *           
	 *  @param[in/out] oveContext   : Hanlde to the encoder context
	 *  @param[in] deviceID         : Device on which encoder context to be created
	 *  @param[in] deviceHandle     : Hanlde for the device information
	 *          
	 *  @return bool : true if successful; otherwise false.
	 *******************************************************************************
	 */
	bool encodeCreate(OPContextHandle *oveContext,uint32 deviceId,
					  OVDeviceHandle *deviceHandle);

	/** 
	 *******************************************************************************
	 *  @fn     encodeOpen
	 *  @brief  Creates encoder session, buffers and initilizes 
	 *          configuration for the encoder session
	 *           
	 *  @param[in/out] encodeHandle : Hanlde to the encoder instance
	 *  @param[in] oveContext       : Hanlde to the encoder context
	 *  @param[in] deviceID         : Device on which encoder context to be created
	 *  @param[in] deviceHandle     : Hanlde for the device information
	 *          
	 *  @return bool : true if successful; otherwise false.
	 *******************************************************************************
	 */
	bool encodeOpen(OVEncodeHandle *encodeHandle,OPContextHandle oveContext,
					uint32 deviceId,OvConfigCtrl *pConfig);

	/** 
	 *******************************************************************************
	 *  @fn     encodeProcess
	 *  @brief  Encode an input video file and output encoded H.264 video file
	 *           
	 *  @param[in] encodeHandle : Hanlde for the encoder 
	 *  @param[in] inFile		: input video file to be encoded
	 *  @param[out] outFile		: output encoded H.264 video file
	 *  @param[in] pConfig		: pointer to custom configuration setting file
	 *  @param[out] profileCnt  : pointer to profile couters 
	 *          
	 *  @return bool : true if successful; otherwise false.
	 *******************************************************************************
	 */
	bool encodeProcess(OVEncodeHandle *encodeHandle, const uint8 *inData, uint8 *outData, DWORD buf_size,
						OvConfigCtrl *pConfig/*, OVprofile *profileCnt*/);

	/** 
	 *******************************************************************************
	 *  @fn     encodeClose
	 *  @brief  This function destroys the resources used by the encoder session
	 *
	 *  @param[in] encodeHandle : Handle for the encoder context
	 *          
	 *  @return bool : true if successful; otherwise false.
	 *******************************************************************************
	 */
	bool encodeClose(OVEncodeHandle *encodeHandle);

	/** 
	 *******************************************************************************
	 *  @fn     encodeDestroy
	 *  @brief  Destroy encoder context
	 *
	 *  @param[in] oveContext : Handle for the encoder context
	 *          
	 *  @return bool : true if successful; otherwise false.
	 *******************************************************************************
	 */
	bool encodeDestroy(OPContextHandle oveContext);

	/** 
	 *******************************************************************************
	 *  @fn     waitForEvent
	 *  @brief  This function waits for the event completion 
	 *           
	 *  @param[in] inMapEvt : Event for which it has to wait for completion
	 *          
	 *  @return bool : true if successful; otherwise false.
	 *******************************************************************************
	 */
	void waitForEvent(cl_event inMapEvt);

};

CodecInst* Open(ICOPEN* icinfo);
DWORD Close(CodecInst* pinst);

#endif