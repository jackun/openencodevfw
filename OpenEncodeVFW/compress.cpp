#include "stdafx.h"
#include "OpenEncodeVFW.h"


// check if the codec can compress the given format to the desired format
DWORD CodecInst::CompressQuery(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut){

	Log(L"Compression query: %d %x %dx%d\n", lpbiIn->biBitCount, lpbiIn->biCompression, lpbiIn->biWidth, lpbiIn->biHeight);
	
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
		if ( lpbiIn->biBitCount != 12 ) {
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

// return the intended compress format for the given input format
DWORD CodecInst::CompressGetFormat(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut){

	if ( !lpbiOut){
		return sizeof(BITMAPINFOHEADER);
	}

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
	if ( started == 0x1337 ){
		CompressEnd();
	}
	started = 0;

	if ( int error = CompressQuery(lpbiIn,lpbiOut) != ICERR_OK ){
		return error;
	}

	mConfigTable[L"pictureWidth"] = mWidth = lpbiIn->biWidth;
	mConfigTable[L"pictureHeight"] = mHeight = lpbiIn->biHeight;
	//mConfigTable[L"encCropLeftOffset"] = 16 - mWidth % 16; 
	//mConfigTable[L"encCropBottomOffset"] = 16 - mHeight % 16; 
	mConfigTable[L"encNumMBsPerSlice"] = (int32)(ceil((float)mWidth/16.f) * ceil((float)mHeight/16.f));
	mConfigTable[L"encVBVBufferSize"] = mConfigTable[L"encRateControlTargetBitRate"] >> 1; //half of bitrate
	
	if(fps_den > 0 && fps_num>0)
	{
		Log(L"Framerate: %d / %d %s\n", fps_num, fps_den, ( mConfigTable[L"sendFPS"]==0? L"(ignored)" : L""));
		//Just ignore, seems to work regardless (though maybe less efficient?)
		if(mConfigTable[L"sendFPS"] == 1)
		{
			mConfigTable[L"encRateControlFrameRateNumerator"] = fps_num;
			mConfigTable[L"encRateControlFrameRateDenominator"] = fps_den;
		}
	}

	OvConfigCtrl *pConfigCtrl = (OvConfigCtrl *) &mConfigCtrl;
    memset (pConfigCtrl, 0, sizeof (OvConfigCtrl));
	encodeSetParam(pConfigCtrl, &mConfigTable);

	mCompression = lpbiIn->biCompression;
	mFormat = lpbiIn->biBitCount;
	mLength = mWidth*mHeight*mFormat/8;
	mCompressed_size = 0;
	mParser->init();
	mWarnedBuggy = false;

	Log(L"Initializing Encoder...\n");
	status = getDevice(&mDeviceHandle);
	if (status == false)
	{
		Log(L"Failed to initializing Encoder...\n");
        return ICERR_INTERNAL;
	}

	/**************************************************************************/
	/* Check deviceHandle.numDevices for number of devices and choose the     */
	/* device on which user wants to create the encoder                       */
	/* In this case device 0 is choosen                                       */
	/**************************************************************************/
	uint32 deviceId = mDeviceHandle.deviceInfo[0].device_id;
	clDeviceID = reinterpret_cast<cl_device_id>(deviceId);
	Log(L"Devices: %d\n", mDeviceHandle.numDevices);
	
	 // print device name
	size_t valueSize;
	clGetDeviceInfo(clDeviceID, CL_DEVICE_NAME, 0, NULL, &valueSize);
	char* value = (char*) malloc(valueSize);
	clGetDeviceInfo(clDeviceID, CL_DEVICE_NAME, valueSize, value, NULL);
	Log(L"Device 1 @ 0x%08x: %S\n", clDeviceID, value);
	free(value);

	// print parallel compute units
	cl_uint maxComputeUnits;
	clGetDeviceInfo(clDeviceID, CL_DEVICE_MAX_COMPUTE_UNITS,
		sizeof(maxComputeUnits), &maxComputeUnits, NULL);
	Log(L"Parallel compute units: %d\n", maxComputeUnits);

	if( !(encodeCreate(&mOveContext, deviceId, &mDeviceHandle) &&
		encodeOpen(&mEncodeHandle, mOveContext, deviceId, pConfigCtrl)))
	{
		delete [] mDeviceHandle.deviceInfo;
		Log(L"Failed to create encoder...\n");
		return ICERR_INTERNAL;
	}

	if(mUseCLConv)
	{
		mCLConvert = new clConvert((cl_context)mOveContext, clDeviceID, mEncodeHandle.clCmdQueue, mWidth, mHeight, mFormat / 8, mLog, mConfigTable[L"SpeedyMath"]==1);
		if(!mCLConvert->init())
		{
			Log(L"Failed to initialize OpenCL colorspace conversion!\n");
			CompressEnd();
			return ICERR_INTERNAL;
		}
	}

	started = 0x1337;

	Log(L"CompressBegin\n");
	return ICERR_OK;
}

// get the maximum size a compressed frame will take;
// 105% of image size + 1KB should be plenty even for random static

DWORD CodecInst::CompressGetSize(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut){
	//return (DWORD)( align_round(lpbiIn->biWidth,16)*lpbiIn->biHeight*lpbiIn->biBitCount/8*1.05 + 1024);
	return x264vfw_compress_get_size(lpbiOut);
}

// release resources when compression is done

DWORD CodecInst::CompressEnd(){

	//TODO 
	if(mCLConvert)
	{
		delete mCLConvert;
		mCLConvert = NULL;
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

	if(prev) 
	{
		free(prev);
		prev = NULL;
	}

	if(buffer2)
	{
		free(buffer2);
		buffer2 = NULL;
	}
		
	Log(L"CompressEnd\n");

	if ( started  == 0x1337 ){

		displayFps(&mProfile, clDeviceID);
	}
	started = 0;

	if(mRaw)
	{
		fclose(mRaw);
		mRaw = NULL;
	}

	if(mLog) mLog->close();

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
	
	out = (uint8 *)icinfo->lpOutput;
	in  = (uint8 *)icinfo->lpInput;
	BITMAPINFOHEADER *inhdr  = icinfo->lpbiInput;
    BITMAPINFOHEADER *outhdr = icinfo->lpbiOutput;

	mFrameNum = icinfo->lFrameNum;
	if ( icinfo->lFrameNum == 0 ){
		if ( started != 0x1337 ){
			if ( int error = CompressBegin(icinfo->lpbiInput,icinfo->lpbiOutput) != ICERR_OK )
				return error;
		}
	}

	if ( icinfo->lpckid ){
		*icinfo->lpckid = 'cd'; // 'dc' Compressed video frame
	}

	int ret_val = ICERR_ERROR;
	
#if NO_BUGGY_APPS
	//Log(L"Buffer size: %d  Bits: %d %d \n", outhdr->biSizeImage, inhdr->biBitCount, inhdr->biCompression);
	//Dxtory sends new buffer with previous compressed size and so it may be smaller than needed.
	//So hopefully new buffer is big enough
	if(prev)
	{
		if( outhdr->biSizeImage >= mCompressed_size )
		{
			Log(L"Feeding previous buffer.\n");
			ret_val = ICERR_OK;
			memcpy((uint8*)out, prev, outhdr->biSizeImage);
			free(prev);
			prev = NULL;
		}
	}
	else
#endif
	/*
	if(mConfigTable[L"blend"] == 1)
	{
		if(icinfo->lFrameNum % 2 == 0)
		{
			if(!buffer2)
				buffer2 = (uint8*)malloc(x264vfw_compress_get_size(inhdr));
			memcpy(buffer2, in, inhdr->biSizeImage);
			ret_val = ICERR_OK;
		}

		if(icinfo->lFrameNum % 2 == 1 || icinfo->lFrameNum == 0)
		if(encodeProcess(&mEncodeHandle, in, out, outhdr->biSizeImage, &mConfigCtrl))
			ret_val = ICERR_OK;
	} else */
	
	if(encodeProcess(&mEncodeHandle, in, out, outhdr->biSizeImage, &mConfigCtrl))
		ret_val = ICERR_OK;
	
	//FIXME Keyframe flag
	if (ret_val == ICERR_OK && (icinfo->lFrameNum % 250 == 0 /*|| mParser->b_key*/) )
	{
		//if(isH264iFrame(out))
		*icinfo->lpdwFlags = AVIIF_KEYFRAME;
		Log(L"Keyframe: %d\n", icinfo->lFrameNum);
	}
	else
		*icinfo->lpdwFlags = 0;

	//Requesting bigger buffer leads to corrupt video from Dxtory :( so disregard output buffer biSizeImage
#if NO_BUGGY_APPS
	if(prev)
	{
		((uint8*)icinfo->lpOutput)[0] = 0x7f;
		outhdr->biSizeImage = 1;
		outhdr->biCompression = FOURCC_OPEN;
	}
	else
#endif
	{
		outhdr->biSizeImage = mCompressed_size;
	}
	outhdr->biCompression = FOURCC_OPEN;


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

	/**************************************************************************/
    /* Create OpenCL context from device's id                                 */
	/**************************************************************************/
    //clDeviceID = reinterpret_cast<cl_device_id>(deviceId);
    *oveContext  = clCreateContext(properties, 1, &clDeviceID, 0, 0, &err);
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
	if(deviceId == 0)
    {
        Log(L"No suitable devices found!\n");
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

	/**************************************************************************/
    /* Create a command queue                                                 */
    /**************************************************************************/
   
    encodeHandle->clCmdQueue = clCreateCommandQueue((cl_context)oveContext,
		                               clDeviceID, 0, &err);
    if(err != CL_SUCCESS)
    {
        Log(L"\nCreate command queue failed! Error : %d\n", err);
        return false;
    }

	/**************************************************************************/
	/* Make sure the surface is byte aligned                                  */
	/**************************************************************************/
    uint32 alignedSurfaceWidth = ((pConfig->width + (256 - 1)) & ~(256 - 1));
    uint32 alignedSurfaceHeight = (true) ? (pConfig->height + 31) & ~31 : 
									  (pConfig->height + 15) & ~15;
	/**************************************************************************/
	/* NV12 is 3/2                                                            */
	/**************************************************************************/
    int32 hostPtrSize = alignedSurfaceHeight * alignedSurfaceWidth * 3/2;

    for(int32 i=0; i<MAX_INPUT_SURFACE; i++)
    {
        encodeHandle->inputSurfaces[i] = clCreateBuffer((cl_context)oveContext,
                                            CL_MEM_READ_WRITE,
                                            hostPtrSize, 
                                            NULL, 
                                            &err);
        if (err != CL_SUCCESS) 
        {
            Log(L"\nclCreateBuffer returned error %d\n", err);
            return false;
        }
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
	
	OPMemHandle		            inputSurface;
	ove_session                 session=encodeHandle->session;
    OVresult  res = 0;
    OPEventHandle eventRunVideoProgram;
    
	// Make sure the surface is byte aligned
    uint32 alignedSurfaceWidth = ((pConfig->width + (256 - 1)) & ~(256 - 1));
    uint32 alignedSurfaceHeight = (true) ? (pConfig->height + 31) & ~31 : 
									  (pConfig->height + 15) & ~15;
	
	// NV12 is 3/2
    int32 hostPtrSize = alignedSurfaceHeight * alignedSurfaceWidth * 3/2; 

	// Only encoding 1 frame
	uint32 framecount=1;
	
    /**************************************************************************/
    /* Setup the picture parameters                                           */
    /**************************************************************************/
    OVE_ENCODE_PARAMETERS_H264 pictureParameter;
    uint32 numEncodeTaskInputBuffers = 1;
    OVE_INPUT_DESCRIPTION *encodeTaskInputBufferList  
            = (OVE_INPUT_DESCRIPTION *) malloc(sizeof(OVE_INPUT_DESCRIPTION) * 
				numEncodeTaskInputBuffers);

    /**************************************************************************/
    /* For the Query Output                                                   */
    /**************************************************************************/
    uint32 iTaskID;
    uint32 numTaskDescriptionsRequested = 1;
    uint32 numTaskDescriptionsReturned = 0;
	//uint32 framenum = 0;
    OVE_OUTPUT_DESCRIPTION pTaskDescriptionList[1];
	//TODO Move stuff up there somewhere else

    /**************************************************************************/
    /* Okay, now it's time to read/encode frame by frame                      */
    /**************************************************************************/
	
    //while (framenum < framecount)
    {
        cl_event inMapEvt;
        cl_int   status;

		inputSurface = encodeHandle->inputSurfaces[mFrameNum%MAX_INPUT_SURFACE];

		/**********************************************************************/
		/* Read the input file frame by frame                                 */
		/**********************************************************************/

        void* mapPtr = NULL;

		if(!mUseCLConv)
		{
			mapPtr = clEnqueueMapBuffer( encodeHandle->clCmdQueue,
                                            (cl_mem)inputSurface,
                                            CL_TRUE, //CL_FALSE,
                                            CL_MAP_READ | CL_MAP_WRITE,
                                            0,
                                            hostPtrSize,
                                            0,
                                            NULL,
                                            &inMapEvt,
                                            &status);

			status = clFlush(encodeHandle->clCmdQueue);
			waitForEvent(inMapEvt);
			status = clReleaseEvent(inMapEvt);
		}
		
		/**********************************************************************/
        /* Convert input buffer to something VCE can eat aka NV12             */
		/**********************************************************************/
		captureTimeStart(&mProfile, 2);
		bool convertFail = false;
		if(mFormat == 32 || mFormat == 24)
		{
			if(mUseCLConv)
			{
				uint32 srcSize = pConfig->width * pConfig->height * (mFormat / 8);
				//if(mConfigTable[L"blend"] == 1 && mFrameNum > 0)
				//	convertFail = mCLConvert->blendAndEncode(buffer2, srcSize, inData, srcSize,(uint8*)mapPtr, hostPtrSize) != 0;
				//else
					//convertFail = mCLConvert->encode(inData, srcSize, (uint8*)mapPtr, hostPtrSize) != 0;
				convertFail = mCLConvert->encode(inData, srcSize, (cl_mem)inputSurface) != 0;
			}
			else
				RGBtoNV12 (inData, (uint8 *)mapPtr, mFormat/8, 1, pConfig->width, pConfig->height, alignedSurfaceWidth);
		}
		else if(mFormat == 12 && (mCompression == FOURCC_NV12 || mCompression == FOURCC_YV12))
		{
			if(mConfigTable[L"YV12AsNV12"] == 1 || mCompression == FOURCC_NV12)
				convertFail = !nv12ToNV12Aligned(inData, pConfig->height, pConfig->width, alignedSurfaceWidth, (int8 *)mapPtr);
			else
				convertFail = !yv12ToNV12(inData, pConfig->height, pConfig->width, alignedSurfaceWidth, (int8 *)mapPtr);
		}
		else
		{
			//Unmap buffer and goto fail
			convertFail = true;
		}
		
		captureTimeStop(&mProfile, 2);

		if(!mUseCLConv && mapPtr)
		{
			cl_event unmapEvent;
			status = clEnqueueUnmapMemObject(encodeHandle->clCmdQueue,
											(cl_mem)inputSurface,
											mapPtr,
											0,
											NULL,
											&unmapEvent);
			status = clFlush(encodeHandle->clCmdQueue);
			waitForEvent(unmapEvent);
			status = clReleaseEvent(unmapEvent);
		}

		if(convertFail)
			goto fail;

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
        pictureParameter.insertSPS = (OVE_BOOL)(mFrameNum == 0)?true:false;
        pictureParameter.pictureStructure = OVE_PICTURE_STRUCTURE_H264_FRAME;
        pictureParameter.forceRefreshMap = (OVE_BOOL)true;
        pictureParameter.forceIMBPeriod = 0;
		//Force keyframe every 250 frames (like x264)
		pictureParameter.forcePicType = mFrameNum % 250 == 0 ? OVE_PICTURE_TYPE_H264_IDR : OVE_PICTURE_TYPE_H264_NONE;

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

        err = clWaitForEvents(1, (cl_event *)&(eventRunVideoProgram));
        if(err != CL_SUCCESS) 
        {
            Log(L"\nlWaitForEvents returned error %d\n", err);
            //return false;
			ret = false;
			goto fail;
        }
		captureTimeStop(&mProfile,0);
        /**********************************************************************/
        /* Query output                                                       */
        /**********************************************************************/

        numTaskDescriptionsReturned = 0;
        memset(pTaskDescriptionList,0,sizeof(OVE_OUTPUT_DESCRIPTION)*numTaskDescriptionsRequested);
        pTaskDescriptionList[0].size = sizeof(OVE_OUTPUT_DESCRIPTION);
		captureTimeStart(&mProfile,1);
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
		captureTimeStop(&mProfile,1);
        /**********************************************************************/
        /*  Write compressed frame to the output                              */
        /**********************************************************************/
		
		//FIXME correct?
		mCompressed_size = 0;
        for(uint32 i=0;i<numTaskDescriptionsReturned;i++)
        {
            if((pTaskDescriptionList[i].status == OVE_TASK_STATUS_COMPLETE) 
                && pTaskDescriptionList[i].size_of_bitstream_data > 0)
            {
				mCompressed_size += pTaskDescriptionList[i].size_of_bitstream_data;
            }
        }

		uint8* finalBuffer = outData;

		//Requesting bigger buffer leads to corrupt video from Dxtory :( so disregard output buffer biSizeImage
		if(!mWarnedBuggy && buf_size < mCompressed_size)
		{
			Log(L"Output buffer is too small: %d. Needs %d. Might crash now.\n", buf_size, mCompressed_size);
			
#if NO_BUGGY_APPS
			prev = (uint8*)malloc(mCompressed_size); //FIXME Save currently encoded and send again in new Compress() call
			finalBuffer = prev;
			ret = false;
#else
			mWarnedBuggy = true;
#endif
		}

		for(uint32 i=0;i<numTaskDescriptionsReturned;i++)
        {
            if((pTaskDescriptionList[i].status == OVE_TASK_STATUS_COMPLETE) 
                && pTaskDescriptionList[i].size_of_bitstream_data > 0)
            {
				//Copy to output buffer
				
				/// Getting keyframe from bitstream doesn't seem to work
				//uint8 * p = (uint8*)pTaskDescriptionList[i].bitstream_data, *p_next;
				//uint8* end = ((uint8*)pTaskDescriptionList[i].bitstream_data) + pTaskDescriptionList[i].size_of_bitstream_data;

				////Log(L"Output: %d Format:%d  %08x %08x\n", buf_size, mFormat, ((int*)p)[0], ((int*)p)[1]);

				////Parse some h264 bitstream for keyframe flag
				//
				//while( p < end - 3 )
				//{
				//	if( p[0] == 0x00 && p[1] == 0x00 && p[2] == 0x01 )
				//	{
				//		break;
				//	}
				//	p++;
				//}

				///* Search end of NAL */
				//p_next = p + 3;
				//while( p_next < end - 3 )
				//{
				//	if( p_next[0] == 0x00 && p_next[1] == 0x00 && p_next[2] == 0x01 )
				//	{
				//		break;
				//	}
				//	p_next++;
				//}

				///* Compute NAL size */
				//int i_size = (int)(p_next - p - 3);

				//int b_flush = 0;
				//int b_start;


				////FIXME Hmm, only SPS and/or PPS nal types
				///* Nal start at p+3 with i_size length */
				//mParser->nal_decode(p+3, i_size < 2048 ? i_size : 2048 ); //get SPS
				//int i_type = mParser->nal.i_type;
				//mParser->parse(&b_start);
				////mParser->nal_decode(p_next+3, i_size < 2048 ? i_size : 2048 );//get PPS
				//mParser->b_key = mParser->h264.b_key;
				//
				//if( b_start && mParser->b_slice )
				//{
				//	b_flush = 1;
				//	mParser->b_slice = 0;
				//}

				//if( mParser->nal.i_type >= NAL_SLICE && mParser->nal.i_type <= NAL_SLICE_IDR )
				//{
				//	mParser->b_slice = 1;
				//}

				//Log(L"nal %d: %d %d, Slice:%d\n", mFrameNum, i_type, mParser->nal.i_type, b_start);

				//TODO Can output buffer be directly mapped to CL buffer?
				memcpy(finalBuffer, (uint8*)pTaskDescriptionList[i].bitstream_data, pTaskDescriptionList[i].size_of_bitstream_data);
				
				//if(mRaw)
				//	fwrite((uint8*)pTaskDescriptionList[i].bitstream_data, 1, pTaskDescriptionList[i].size_of_bitstream_data, mRaw);

                finalBuffer += pTaskDescriptionList[i].size_of_bitstream_data;

                res = OVEncodeReleaseTask( session, pTaskDescriptionList[i].taskID);
            }
        }

        if(eventRunVideoProgram)
		    clReleaseEvent((cl_event) eventRunVideoProgram);
    } /* End of read/encode/write loop*/
	
fail:
    /**************************************************************************/
    /* Free memory resources now that we're through with them.                */
    /**************************************************************************/
    free(encodeTaskInputBufferList);
    
    return status;
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
			err = clReleaseMemObject((cl_mem)inputSurfaces[i]);
        if(err != CL_SUCCESS)
        {
            Log(L"\nclReleaseMemObject returned error %d\n", err);
            return false;
        }
    }

	if(encodeHandle->clCmdQueue)
    err = clReleaseCommandQueue(encodeHandle->clCmdQueue);
	if(err != CL_SUCCESS)
    {
        Log(L"Error releasing Command queue\n");
        return false;
    }

	if(encodeHandle->session)
    oveErr = OVEncodeDestroySession(encodeHandle->session);
    if(!oveErr)
    {
        Log(L"Error releasing OVE Session\n");
        return false;
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
        err = clReleaseContext((cl_context)oveContext);
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
		status = clGetEventInfo(
	                inMapEvt, 
	                CL_EVENT_COMMAND_EXECUTION_STATUS, 
	                sizeof(cl_int),
	                &eventStatus,
	                NULL);
	}
}
