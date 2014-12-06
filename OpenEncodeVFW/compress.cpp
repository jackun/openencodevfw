#include "stdafx.h"
#include "OpenEncodeVFW.h"

#define Log(...) LogMsg(false, __VA_ARGS__)
#define UNMAKEFOURCC(cc) \
				(char)(cc & 0xFF), (char)((cc >> 8) & 0xFF), \
				(char)((cc >> 16) & 0xFF), (char)((cc >> 24 ) & 0xFF)

// check if the codec can compress the given format to the desired format
DWORD CodecInst::CompressQuery(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut){

	if(mLog) mLog->enableLog(mConfigTable["Log"] == 1);

	Log(L"Compression query: %d %x %dx%d\n", lpbiIn->biBitCount, lpbiIn->biCompression, lpbiIn->biWidth, lpbiIn->biHeight);
	//if(lpbiIn->biCompression)//needs checks or writes '\0' to log, hence fucking it up
	//	Log(L"FourCC: %c%c%c%c\n", UNMAKEFOURCC(lpbiIn->biCompression));

	/*if(lpbiIn->biCompression == BI_BITFIELDS)
	{
		RGBQUAD *pColor = (RGBQUAD*)((LPSTR)lpbiIn + lpbiIn->biSize);
	}*/

	// check for valid format and bitdepth
	if ( lpbiIn->biCompression == 0){
		if(lpbiIn->biBitCount != 24 && lpbiIn->biBitCount != 32)
			return_badformat()
	} 
	/*else if ( lpbiIn->biCompression == FOURCC_YUY2 || lpbiIn->biCompression == FOURCC_UYVY || lpbiIn->biCompression == FOURCC_YV16 ){
		if ( lpbiIn->biBitCount != 16 ) {
			return_badformat()
		}
	}*/
	else if ( lpbiIn->biCompression == FOURCC_YV12 //decoder has different ideas what YV12 is than yuvToNV12() :S
			|| lpbiIn->biCompression == FOURCC_NV12 //have to implement compression check before this, still needs alignment
			){
		if ( lpbiIn->biBitCount != 12 && lpbiIn->biBitCount != 16 ) { //Virtualdub sends NV12 as 16bits???
			return_badformat()
		}

	} else {
		return_badformat()
	}

	mWidth = lpbiIn->biWidth;
	mHeight = lpbiIn->biHeight;

	/* We need x2 width/height although it gets aligned to 16 by 16
	*/
	if (mWidth % 2 || mHeight % 2)// || mWidth > 1920 || mHeight > 1088) //TODO get max w/h from caps
		return ICERR_BADFORMAT; //Should probably be ICERR_BADIMAGESIZE

	Log(L"CompressQuery OK \r\n");
	return (DWORD)ICERR_OK;
}

/* Return the maximum number of bytes a single compressed frame can occupy */
LRESULT x264vfw_compress_get_size(LPBITMAPINFOHEADER lpbiOut)
{
	return ((lpbiOut->biWidth + 15) & ~15) * ((lpbiOut->biHeight + 31) & ~31) * 3 + 4096;
}

// get the maximum size a compressed frame will take;
// 105% of image size + 1KB should be plenty even for random static
DWORD CodecInst::CompressGetSize(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut){
	//return (DWORD)( align_round(lpbiIn->biWidth,16)*lpbiIn->biHeight*lpbiIn->biBitCount/8*1.05 + 1024);
	return x264vfw_compress_get_size(lpbiOut);
}

// return the intended compress format for the given input format
DWORD CodecInst::CompressGetFormat(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut){

	if ( !lpbiOut){
		return sizeof(BITMAPINFOHEADER);
	}
	if(mLog) mLog->enableLog(mConfigTable["Log"] == 1);
	Log(L"Compression query: %d %d %dx%d\n", lpbiIn->biBitCount, lpbiIn->biCompression, lpbiIn->biWidth, lpbiIn->biHeight);

	// make sure the input is an acceptable format
	if ( CompressQuery(lpbiIn, NULL) == ICERR_BADFORMAT ){
		return_badformat()
	}

	//FIXME 
	*lpbiOut = *lpbiIn;
	lpbiOut->biSize = sizeof(BITMAPINFOHEADER);
	lpbiOut->biCompression = FOURCC_OPEN;
	//x264vfw
	lpbiOut->biBitCount = (lpbiIn->biBitCount == 32 || lpbiIn->biBitCount == 24) ? lpbiIn->biBitCount : 24;
	lpbiOut->biPlanes = 1;
	lpbiOut->biSizeImage = x264vfw_compress_get_size(lpbiOut);
	lpbiOut->biXPelsPerMeter = 0;
	lpbiOut->biYPelsPerMeter = 0;
	lpbiOut->biClrUsed = 0;
	lpbiOut->biClrImportant = 0;

	return (DWORD)ICERR_OK;
}

// initalize the codec for compression
DWORD CodecInst::CompressBegin(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut){
	if(mLog) mLog->enableLog(mConfigTable["Log"] == 1);

	if ( started == 0x1337 ){
		CompressEnd();
	}
	started = 0;

	if ( int error = CompressQuery(lpbiIn,lpbiOut) != ICERR_OK ){
		return error;
	}

	mConfigTable["pictureWidth"] = mWidth = lpbiIn->biWidth;
	mConfigTable["pictureHeight"] = mHeight = lpbiIn->biHeight;
	
	int numH = ((mHeight + 15) / 16), numW = ((mWidth + 15) / 16);
	//mConfigTable["encCropRightOffset"] = (numW * 16 - mWidth) >> 1;
	if(mConfigTable["crop"])
	{
		mConfigTable["encCropBottomOffset"] = (numH * 16 - mHeight) >> 1;
		mConfigTable["encCropRightOffset"] = (numW * 16 - mWidth) >> 1;
	}
	mConfigTable["encNumMBsPerSlice"] = numW * numH;
	mConfigTable["encVBVBufferSize"] = mConfigTable["encRateControlTargetBitRate"] >> 1; //half of bitrate
	
	if(fps_den > 0 && fps_num>0)
	{
		Log(L"Framerate: %d / %d %s\n", fps_num, fps_den, ( mConfigTable["sendFPS"]==0? L"(ignored)" : L""));
		//Just ignore, seems to work regardless (though maybe less efficient?)
		if(mConfigTable["sendFPS"] == 1)
		{
			mConfigTable["encRateControlFrameRateNumerator"] = fps_num;
			mConfigTable["encRateControlFrameRateDenominator"] = fps_den;
		}
	}

	OvConfigCtrl *pConfigCtrl = (OvConfigCtrl *) &mConfigCtrl;
	memset (pConfigCtrl, 0, sizeof (OvConfigCtrl));
	encodeSetParam(pConfigCtrl, &mConfigTable);

	mIDRFrames = mConfigTable["IDRframes"];
	mProfKernels = (mConfigTable["ProfileKernels"] == 1);
	// Align SPS/PPS with IDR
	pConfigCtrl->pictControl.encHeaderInsertionSpacing =
		mConfigTable["encHeaderInsertionSpacing"] > 0 ? mIDRFrames : 0;
	mCompression = lpbiIn->biCompression;
	mFormat = lpbiIn->biBitCount;
	mLength = mWidth*mHeight*mFormat/8;
	mCompressed_size = 0;
	//mParser->init();
	mWarnedBuggy = false;

	Log(L"Initializing Encoder...\n");
	status = getDevice(&mDeviceHandle);
	if (status == false)
	{
		LogMsg(mMsgBox, L"Failed to get devices...\n");
		return ICERR_INTERNAL;
	}

	/**************************************************************************/
	/* Check deviceHandle.numDevices for number of devices and choose the     */
	/* device on which user wants to create the encoder                       */
	/**************************************************************************/
	Log(L"Device count: %d\n", mDeviceHandle.numDevices);
	if(mDeviceHandle.numDevices == 0)
		return ICERR_INTERNAL;

	uint32 idx = MIN((uint32)mConfigTable["UseDevice"], mDeviceHandle.numDevices - 1);
	Log(L"Selecting device: %d\n", idx);
	uint32 deviceId = mDeviceHandle.deviceInfo[idx].device_id;
	clDeviceID = reinterpret_cast<cl_device_id>(deviceId);

#ifdef _M_X64
	// May ${DEITY} have mercy on us all.
	intptr_t ptr = intptr_t((intptr_t*)&clDeviceID);
	clDeviceID = (cl_device_id)((intptr_t)clDeviceID | (ptr & 0xFFFFFFFF00000000));
#endif
	
	 // print device name
	size_t valueSize;
	f_clGetDeviceInfo(clDeviceID, CL_DEVICE_NAME, 0, NULL, &valueSize);
	char* value = (char*) malloc(valueSize);
	f_clGetDeviceInfo(clDeviceID, CL_DEVICE_NAME, valueSize, value, NULL);
	Log(L"Device %d @ 0x%08x: %S\n", idx, clDeviceID, value);
	free(value);

	// print parallel compute units
	cl_uint maxComputeUnits;
	f_clGetDeviceInfo(clDeviceID, CL_DEVICE_MAX_COMPUTE_UNITS,
		sizeof(maxComputeUnits), &maxComputeUnits, NULL);
	Log(L"Parallel compute units: %d\n", maxComputeUnits);

	if(!encodeCreate(&mOveContext, deviceId, &mDeviceHandle))
	{
		CompressEnd();
		LogMsg(mMsgBox, L"Failed to create encoder...\n");
		return ICERR_INTERNAL;
	}

	if(!encodeOpen(&mEncodeHandle, mOveContext, deviceId, pConfigCtrl))
	{
		CompressEnd();
		LogMsg(mMsgBox, L"Failed to open encoder session.\n"
						L"You may need to restart your computer.\n");
		return ICERR_INTERNAL;
	}

	if(mUseCLConv)
	{
		if(mUseCPU) {
			if(createCPUContext(mDeviceHandle.platform))
				Log(L"Using CPU for RGB to NV12 conversion.\n");
			else {
				Log(L"Failed to create CPU OpenCL context.\n");
				return ICERR_INTERNAL;
			}
		}

		mCLConvert = new clConvert(
			mUseCPU ? mCpuCtx : (cl_context)mOveContext,
			mUseCPU ? mCpuDev : clDeviceID,
			mUseCPU ? mCpuCmdQueue : mEncodeHandle.clCmdQueue,
			mWidth,
			mHeight,
			mFormat / 8,
			mLog,
			&mProfile,
			mConfigTable["SpeedyMath"]==1,
			mConfigTable["SwitchByteOrder"]==1);

		COLORMATRIX matrix = static_cast<COLORMATRIX>(mConfigTable["colormatrix"]);
		if(!(mCLConvert->createKernels(matrix) == SUCCESS && 
			mCLConvert->encodeInit(false, (cl_mem)mEncodeHandle.inputSurfaces[0]) == SUCCESS))
		{
			LogMsg(mMsgBox, L"Failed to initialize OpenCL colorspace conversion!\n");
			CompressEnd();
			return ICERR_INTERNAL;
		}
	}

	started = 0x1337;

	Log(L"CompressBegin\n");
	return ICERR_OK;
}

// release resources when compression is done

DWORD CodecInst::CompressEnd(){

	if(mCpuCtx)
	{
		f_clReleaseCommandQueue(mCpuCmdQueue[0]);
		f_clReleaseCommandQueue(mCpuCmdQueue[1]);
		//f_clReleaseDevice(mCpuDev);//no need
		f_clReleaseContext(mCpuCtx);
		mCpuCtx = NULL;
		mCpuDev = NULL;
		mCpuCmdQueue[0] = NULL;
		mCpuCmdQueue[1] = NULL;
	}

	status = encodeClose(&mEncodeHandle);
	//FIXME what to do?
	/*if (status == false)
	{
		return 1;
	}*/
	mCompressed_size = 0;
	status = encodeDestroy(mOveContext);

	if(mDeviceHandle.deviceInfo)
	{
		delete [] mDeviceHandle.deviceInfo;
		mDeviceHandle.deviceInfo = NULL;
	}

	Log(L"CompressEnd\n");

	if ( started  == 0x1337 ){
		displayFps(mLog, &mProfile, clDeviceID);
		if(mCLConvert && mConfigTable["ProfileKernels"]==1) {
			Log(L"Y kernel                      : %f seconds (avg)\n", mCLConvert->profSecs1);
			Log(L"UV kernel                     : %f seconds (avg)\n", mCLConvert->profSecs2);
		}
	}

	started = 0;

	if(mRaw)
	{
		fclose(mRaw);
		mRaw = NULL;
	}

	if(mLog) mLog->close();

	if(mCLConvert)
	{
		delete mCLConvert;
		mCLConvert = NULL;
	}

	return ICERR_OK;
}

DWORD CodecInst::CompressFramesInfo(ICCOMPRESSFRAMES *icf)
{
	frame_total = icf->lFrameCount;
	fps_num = icf->dwRate;
	fps_den = icf->dwScale;
	return ICERR_OK;
}

// called to compress a frame; the actual compression will be
// handed off to other functions depending on the color space and settings

DWORD CodecInst::Compress(ICCOMPRESS* icinfo, DWORD dwSize) {
	const uint8 * in;
	uint8 * out;
	out = (uint8 *)icinfo->lpOutput;
	in  = (const uint8 *)icinfo->lpInput;
	BITMAPINFOHEADER *inhdr  = icinfo->lpbiInput;
	BITMAPINFOHEADER *outhdr = icinfo->lpbiOutput;

	mFrameNum = icinfo->lFrameNum;
	if ( icinfo->lFrameNum == 0 ){
		if ( started != 0x1337 ){
			if ( int error = CompressBegin(icinfo->lpbiInput,icinfo->lpbiOutput) != ICERR_OK )
				return error;
		}
	}

	//Whole compression
	//captureTimeStart(&mProfile, 10);

	if ( icinfo->lpckid ){
		*icinfo->lpckid = 'cd'; // 'dc' Compressed video frame
	}

	int ret_val = ICERR_ERROR;

	captureTimeStart(&mProfile, 4);
	if(encodeProcess(&mEncodeHandle, in, out, outhdr->biSizeImage, &mConfigCtrl))
		ret_val = ICERR_OK;
	captureTimeStop(&mProfile, 4);
	
	//FIXME Keyframe flag
	if (ret_val == ICERR_OK &&
		(mHasIDR || (mIDRFrames > 0 && icinfo->lFrameNum % mIDRFrames == 0))
	)
	{
		*icinfo->lpdwFlags = AVIIF_KEYFRAME;
		//Log(L"Keyframe: %d\n", icinfo->lFrameNum);
	}
	else
		*icinfo->lpdwFlags = 0;

	outhdr->biSizeImage = mCompressed_size;
	outhdr->biCompression = FOURCC_OPEN;

	//captureTimeStop(&mProfile, 10);
	return (DWORD)ret_val;
}

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
bool CodecInst::encodeCreate(OPContextHandle *oveContext,uint32 deviceId,
				  OVDeviceHandle *deviceHandle)
{
	//cl_device_id		 clDeviceID;
	bool status;
	cl_int err;
	*oveContext = NULL;
	/**************************************************************************/
	/* Create the CL Context - nothing works without a context handle.*/
	/**************************************************************************/
	/**************************************************************************/
	/* Create a variable for the open video encoder device id                 */
	/**************************************************************************/
 
	intptr_t properties[] =
	{
		CL_CONTEXT_PLATFORM, (cl_context_properties)deviceHandle->platform,
		0
	};

	if(deviceId == 0)
	{
		Log(L"No suitable devices found!\n");
		return false;
	}

	/**************************************************************************/
	/* Create OpenCL context from device's id                                 */
	/**************************************************************************/
	//clDeviceID = reinterpret_cast<cl_device_id>(deviceId);
	*oveContext  = f_clCreateContext(properties, 1, &clDeviceID, 0, 0, &err);
	if(*oveContext  == (cl_context)0) 
	{
		Log(L"\nCannot create cl_context\n");
		return false;
	}

	if(err != CL_SUCCESS) 
	{
		Log(L"Error in clCreateContext %d\n", err); 
		return false;
	}

	/**************************************************************************/
	/* Read the device capabilities...                                        */
	/* Device capabilities should be used to validate against the             */
	/* configuration set by the user for the codec                            */
	/**************************************************************************/

	OVE_ENCODE_CAPS encodeCaps;
	OVE_ENCODE_CAPS_H264 encode_cap_full;
	encodeCaps.caps.encode_cap_full = (OVE_ENCODE_CAPS_H264 *)&encode_cap_full;
	status = getDeviceCap(*oveContext ,deviceId,&encodeCaps);
	
	if(!status)
	{
		Log(L"OVEncodeGetDeviceCap failed!\n");
		return false;
	}
	
	Log(L"**** CAPS ****\n* Bitrate Max: %d Min: %d\n", encodeCaps.caps.encode_cap_full->max_bit_rate, encodeCaps.caps.encode_cap_full->min_bit_rate);
	Log(L"* Picture size Max: %d Min: %d (Macroblocks (width/16) * (height/16))\n* Profiles:\n", encodeCaps.caps.encode_cap_full->max_picture_size_in_MB, encodeCaps.caps.encode_cap_full->min_picture_size_in_MB );
	for(int i = 0; i< 20/*encodeCaps.caps.encode_cap_full->num_Profile_level*/; i++)
		Log(L"*\tProf: %d Level: %d\n", encodeCaps.caps.encode_cap_full->supported_profile_level[i].profile, encodeCaps.caps.encode_cap_full->supported_profile_level[i].level);
	Log(L"**************\n");

	return true;
}
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
bool CodecInst::encodeOpen(OVEncodeHandle *encodeHandle,OPContextHandle oveContext,
				uint32 deviceId,OvConfigCtrl *pConfig)
{
	//cl_device_id clDeviceID = reinterpret_cast<cl_device_id>(deviceId);
	OVresult  res = 0;
	cl_int err;
	
	/**************************************************************************/
	/* Create a command queue                                                 */
	/**************************************************************************/
   
	cl_command_queue_properties prop = 0;
	if(mConfigTable["ProfileKernels"] == 1)
		prop |= CL_QUEUE_PROFILING_ENABLE;

	encodeHandle->clCmdQueue[0] = f_clCreateCommandQueue((cl_context)oveContext,
									   clDeviceID, prop, &err);
	if(err != CL_SUCCESS)
	{
		Log(L"\nCreate command queue #0 failed! Error : %d\n", err);
		return false;
	}

	encodeHandle->clCmdQueue[1] = f_clCreateCommandQueue((cl_context)oveContext,
									   clDeviceID, prop, &err);
	if(err != CL_SUCCESS)
	{
		Log(L"\nCreate command queue #1 failed! Error : %d\n", err);
		return false;
	}

	/**************************************************************************/
	/* Make sure the surface is byte aligned                                  */
	/**************************************************************************/
	mAlignedSurfaceWidth = ((pConfig->width + (256 - 1)) & ~(256 - 1));
	mAlignedSurfaceHeight = (true) ? (pConfig->height + 31) & ~31 : 
									  (pConfig->height + 15) & ~15;
	/**************************************************************************/
	/* NV12 is 3/2                                                            */
	/**************************************************************************/
	mHostPtrSize = mAlignedSurfaceHeight * mAlignedSurfaceWidth * 3/2;
	//cl_int mode[] = {CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY};
	cl_int mode[] = {CL_MEM_WRITE_ONLY};
	for(int32 i=0; i<MAX_INPUT_SURFACE; i++)
	{
		encodeHandle->inputSurfaces[i] = f_clCreateBuffer((cl_context)oveContext,
											mode[i%MAX_INPUT_SURFACE],
											mHostPtrSize, 
											NULL, 
											&err);
		if (err != CL_SUCCESS) 
		{
			Log(L"\nclCreateBuffer returned error %d\n", err);
			return false;
		}
	}

	/**************************************************************************/
	/* Create an OVE Session                                                  */
	/**************************************************************************/
	encodeHandle->session = OVEncodeCreateSession(oveContext,  /**<Platform context */
									deviceId,               /**< device id */
									pConfig->encodeMode,    /**< encode mode */
									pConfig->profileLevel,  /**< encode profile */
									pConfig->pictFormat,    /**< encode format */
									pConfig->width,         /**< width */
									pConfig->height,        /**< height */
									pConfig->priority);     /**< encode task priority, ie. FOR POSSIBLY LOW LATENCY OVE_ENCODE_TASK_PRIORITY_LEVEL2 */
	if(encodeHandle->session == NULL) 
	{
		Log(L"\nOVEncodeCreateSession failed.\n");
		return false;
	}
	/**************************************************************************/
	/* Configure the encoding engine based upon the config file               */
	/* specifications                                                         */
	/**************************************************************************/
	res = setEncodeConfig(encodeHandle->session,pConfig);
	if (!res)
	{
		return false;
	}
	return true;
}

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
bool CodecInst::encodeProcess(OVEncodeHandle *encodeHandle, const uint8 *inData, uint8 *outData, DWORD buf_size,
				 OvConfigCtrl *pConfig)
{
	bool ret = true;
	cl_int err;
	uint32             numEventInWaitList = 0;
	
	OPMemHandle                 inputSurface;
	ove_session                 session=encodeHandle->session;
	OVresult  res = 0;
	OPEventHandle eventRunVideoProgram;
	
	// Only encoding 1 frame
	//uint32 framecount=1;
	
	/**************************************************************************/
	/* Setup the picture parameters                                           */
	/**************************************************************************/
	OVE_ENCODE_PARAMETERS_H264 pictureParameter;
	uint32 numEncodeTaskInputBuffers = 1;
	//OVE_INPUT_DESCRIPTION *encodeTaskInputBufferList  
	//        = (OVE_INPUT_DESCRIPTION *) malloc(sizeof(OVE_INPUT_DESCRIPTION) * 
	//            numEncodeTaskInputBuffers);
	OVE_INPUT_DESCRIPTION encodeTaskInputBufferList[1];

	/**************************************************************************/
	/* For the Query Output                                                   */
	/**************************************************************************/
	uint32 iTaskID;
	uint32 numTaskDescriptionsRequested = 1;
	uint32 numTaskDescriptionsReturned = 0;
	//uint32 framenum = 0;
	OVE_OUTPUT_DESCRIPTION pTaskDescriptionList[1];
	//TODO Move stuff up there somewhere else

	cl_event inMapEvt;
	cl_int   status = CL_SUCCESS;

	inputSurface = encodeHandle->inputSurfaces[0];//[mFrameNum%MAX_INPUT_SURFACE];

	captureTimeStart(&mProfile, 2);

	void* mapPtr = NULL;
	if(!mUseCLConv || 
		(
			(mFormat == 12 || mFormat == 16) && 
			(mCompression == FOURCC_NV12 || mCompression == FOURCC_YV12)
		)
	)
	{
		mapPtr = f_clEnqueueMapBuffer(encodeHandle->clCmdQueue[0],
					(cl_mem)encodeHandle->inputSurfaces[0],
					CL_TRUE, //CL_FALSE,
					CL_MAP_WRITE,
					0,
					mHostPtrSize,
					0,
					NULL,
					&inMapEvt,
					&status);

		status = f_clFlush(encodeHandle->clCmdQueue[0]);
		waitForEvent(inMapEvt);
		status = f_clReleaseEvent(inMapEvt);
	}

	/**********************************************************************/
	/* Convert input buffer to something VCE can eat aka NV12             */
	/**********************************************************************/
	bool convertFail = false;
	if(mFormat == 32 || mFormat == 24)
	{
		if(mUseCLConv)
		{
			//uint32 srcSize = pConfig->width * pConfig->height * (mFormat / 8);
			convertFail = mCLConvert->convert(inData, (cl_mem)encodeHandle->inputSurfaces[0], mProfKernels) != 0;
			/*status = f_clEnqueueCopyBuffer(encodeHandle->clCmdQueue[0], 
				(cl_mem)encodeHandle->inputSurfaces[1],
				(cl_mem)encodeHandle->inputSurfaces[0],
				0, 0, hostPtrSize,
				0, NULL, NULL);
			status = f_clFlush(encodeHandle->clCmdQueue[0]);*/
		}
		else
			RGBtoNV12 (inData, (uint8 *)mapPtr, mFormat/8, 1, mConfigTable["SwitchByteOrder"], pConfig->width, pConfig->height, mAlignedSurfaceWidth);
	}
	else if(
		(mFormat == 12 || mFormat == 16) && 
		(mCompression == FOURCC_NV12 || mCompression == FOURCC_YV12)
	)
	{
		if (mCompression == FOURCC_NV12 /*|| mConfigTable["YV12AsNV12"] == 1*/)
			convertFail = !nv12ToNV12Aligned(inData, pConfig->height, pConfig->width, mAlignedSurfaceWidth, (int8 *)mapPtr);
		else
			convertFail = !yv12ToNV12(inData, pConfig->height, pConfig->width, mAlignedSurfaceWidth, (int8 *)mapPtr);
	}
	else
	{
		//Unmap buffer and goto fail
		convertFail = true;
	}

	if(mapPtr)
	{
		cl_event unmapEvent;
		status = f_clEnqueueUnmapMemObject(encodeHandle->clCmdQueue[0],
										(cl_mem)encodeHandle->inputSurfaces[0],
										mapPtr,
										0,
										NULL,
										&unmapEvent);
		status = f_clFlush(encodeHandle->clCmdQueue[0]);
		waitForEvent(unmapEvent);
		status = f_clReleaseEvent(unmapEvent);
	}

	captureTimeStop(&mProfile, 2);

	if(convertFail) {
		Log(L"Conversion failed!\n");
		goto fail;
	}

	/**********************************************************************/
	/* use the input surface buffer as our Picture                        */
	/**********************************************************************/
	encodeTaskInputBufferList[0].bufferType = OVE_BUFFER_TYPE_PICTURE;
	encodeTaskInputBufferList[0].buffer.pPicture =  (OVE_SURFACE_HANDLE) inputSurface;

	/**********************************************************************/
	/* Setup the picture parameters                                       */
	/**********************************************************************/
	memset(&pictureParameter, 0, sizeof(OVE_ENCODE_PARAMETERS_H264));
	pictureParameter.size = sizeof(OVE_ENCODE_PARAMETERS_H264);
	pictureParameter.flags.value = 0;
	pictureParameter.flags.flags.reserved = 0;
	pictureParameter.insertSPS = (OVE_BOOL)(mFrameNum == 0);
	pictureParameter.pictureStructure = OVE_PICTURE_STRUCTURE_H264_FRAME;
	pictureParameter.forceRefreshMap = (OVE_BOOL)true;
	pictureParameter.forceIMBPeriod = 0;
	//Force keyframe every 250 frames (like x264)
	if(mIDRFrames > 0)
		pictureParameter.forcePicType = (mFrameNum % mIDRFrames == 0) ? OVE_PICTURE_TYPE_H264_IDR : OVE_PICTURE_TYPE_H264_NONE;
	else
		pictureParameter.forcePicType = OVE_PICTURE_TYPE_H264_NONE;

	//framenum++;

	/**********************************************************************/
	/* encode a single picture.                                           */
	/**********************************************************************/

	/**********************************************************************/
	/* Start the timer before calling VCE for frame encode                */
	/**********************************************************************/
	captureTimeStart(&mProfile,0);
	res = OVEncodeTask(session,
						numEncodeTaskInputBuffers,
						encodeTaskInputBufferList,
						&pictureParameter,
						&iTaskID,
						numEventInWaitList,
						NULL,
						&eventRunVideoProgram);
	if (!res) 
	{
		Log(L"\nOVEncodeTask returned error %fd\n", res);
		//return false;
		ret = false;
		goto fail;
	}
		 
	/**********************************************************************/
	/* Wait for Encode session completes                                  */
	/**********************************************************************/

	err = f_clWaitForEvents(1, (cl_event *)&(eventRunVideoProgram));
	if(err != CL_SUCCESS) 
	{
		Log(L"\nlWaitForEvents returned error %d\n", err);
		//return false;
		ret = false;
		goto fail;
	}

	/**********************************************************************/
	/* Query output                                                       */
	/**********************************************************************/
	numTaskDescriptionsReturned = 0;
	memset(pTaskDescriptionList,0,sizeof(OVE_OUTPUT_DESCRIPTION)*numTaskDescriptionsRequested);
	pTaskDescriptionList[0].size = sizeof(OVE_OUTPUT_DESCRIPTION);

	do
	{
		res = OVEncodeQueryTaskDescription(session,
											numTaskDescriptionsRequested,
											&numTaskDescriptionsReturned,
											pTaskDescriptionList);
		if (!res)
		{
			Log(L"\nOVEncodeQueryTaskDescription returned error %fd\n", err);
			//return false;
			ret = false;
			goto fail;
		}
			
	} while(pTaskDescriptionList->status == OVE_TASK_STATUS_NONE);
	captureTimeStop(&mProfile,0);

	/**********************************************************************/
	/*  Write compressed frame to the output                              */
	/**********************************************************************/
	mCompressed_size = 0;
	captureTimeStart(&mProfile, 3);

	uint8* finalBuffer = outData;

	for(uint32 i=0;i<numTaskDescriptionsReturned;i++)
	{
		if((pTaskDescriptionList[i].status == OVE_TASK_STATUS_COMPLETE) 
			&& pTaskDescriptionList[i].size_of_bitstream_data > 0)
		{
			//Copy to output buffer
			memcpy(finalBuffer, (uint8*)pTaskDescriptionList[i].bitstream_data, pTaskDescriptionList[i].size_of_bitstream_data);
			finalBuffer += pTaskDescriptionList[i].size_of_bitstream_data;
			mCompressed_size += pTaskDescriptionList[i].size_of_bitstream_data;

			res = OVEncodeReleaseTask( session, pTaskDescriptionList[i].taskID);
		}
	}

	if(eventRunVideoProgram)
		f_clReleaseEvent((cl_event) eventRunVideoProgram);

	mHasIDR = false;
	uint8 *start = (uint8 *)outData;
	uint8 *end = start + mCompressed_size;
	const static uint8 start_seq[] = { 0, 0, 1 };
	start = std::search(start, end, start_seq, start_seq + 3);

	while (start != end)
	{
		decltype(start) next = std::search(start + 1, end, start_seq, start_seq + 3);

		//int i_ref_idc = (start[3] >> 5) & 3;
		int i_type = start[3] & 0x1f;
		if (i_type == NAL_SLICE_IDR)
		{
			mHasIDR = true;
			break;
		}
		//Log(L"nal type: %d idc: %d\n" ,i_type, i_ref_idc);
		start = next;
	}

	captureTimeStop(&mProfile, 3);
fail:
	/**************************************************************************/
	/* Free memory resources now that we're through with them.                */
	/**************************************************************************/
	//free(encodeTaskInputBufferList);
	return status == CL_SUCCESS;
}

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
bool CodecInst::encodeClose(OVEncodeHandle *encodeHandle)
{
	bool oveErr = false;
	cl_int err = CL_SUCCESS;
	OPMemHandle *inputSurfaces = encodeHandle->inputSurfaces;

	for(int32 i=0; i<MAX_INPUT_SURFACE ;i++)
	{
		err = CL_SUCCESS;
		if(inputSurfaces[i])
			err = f_clReleaseMemObject((cl_mem)inputSurfaces[i]);
		inputSurfaces[i] = NULL;
		if(err != CL_SUCCESS)
		{
			Log(L"\nclReleaseMemObject returned error %d\n", err);
		}
	}

	for(int i=0; i<2; i++) {
		if(encodeHandle->clCmdQueue[i])
			err = f_clReleaseCommandQueue(encodeHandle->clCmdQueue[i]);
		encodeHandle->clCmdQueue[i] = NULL;
		if(err != CL_SUCCESS)
		{
			Log(L"Error releasing Command queue #%d\n", i);
		}
	}

	if(encodeHandle->session)
		oveErr = OVEncodeDestroySession(encodeHandle->session);
	if(!oveErr)
	{
		Log(L"Error releasing OVE Session\n");
	}
	encodeHandle->session = NULL;
	return true;
}

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
bool CodecInst::encodeDestroy(OPContextHandle oveContext)
{
	cl_int err;
	
	if((cl_context)oveContext) 
	{
		err = f_clReleaseContext((cl_context)oveContext);
		if(err != CL_SUCCESS) 
		{
			Log(L"Error releasing cl context\n");
			return false;
		}
	}
	oveContext = NULL;
	return true;
}

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
void CodecInst::waitForEvent(cl_event inMapEvt)
{
	cl_int eventStatus = CL_QUEUED;
	cl_int   status;

	while(eventStatus != CL_COMPLETE)
	{
		status = f_clGetEventInfo(
					inMapEvt, 
					CL_EVENT_COMMAND_EXECUTION_STATUS, 
					sizeof(cl_int),
					&eventStatus,
					NULL);
	}
}
