#include "stdafx.h"
#include "OpenEncodeVFW.h"

#define Log(...) LogMsg(false, __VA_ARGS__)

/** 
*******************************************************************************
*  @fn     getDevice
*  @brief  returns the platform and devices found
*           
*  @param[in/out] deviceHandle : Handle for the device information
*          
*  @return bool : true if successful; otherwise false.
*******************************************************************************
*/
bool CodecInst::getDevice(OVDeviceHandle *deviceHandle)
{
	bool status;

    /**************************************************************************/
    /* Get the Platform                                                       */
    /**************************************************************************/
	deviceHandle->platform = NULL;
	status = getPlatform(deviceHandle->platform);
	if(status == false)
	{
		return false;
	}

    /// STEP 1: Check for GPU
	cl_device_type dType = CL_DEVICE_TYPE_GPU;
	status = gpuCheck(deviceHandle->platform,&dType);
	if(status == false)
	{
		return false;
	}

    /// STEP 2: Get the number of devices
    deviceHandle->numDevices = 0;
	deviceHandle->deviceInfo = NULL;

	/**************************************************************************/
	/* Memory for deviceInfo gets allocated inside the getDeviceInfo        */
	/* function depending on numDevices. This needs to be freed after the     */
	/* usage                                                                  */
	/**************************************************************************/
	status = getDeviceInfo(&deviceHandle->deviceInfo,&deviceHandle->numDevices);
	if(status == false)
	{
		return false;
	}
	return true;
}

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
bool CodecInst::getDeviceCap(OPContextHandle oveContext,uint32 oveDeviceID,
				OVE_ENCODE_CAPS *encodeCaps)
{
    uint32 numCaps=1;
	bool status;
   
	/**************************************************************************/
    /* initialize the encode capabilities variable                            */
	/**************************************************************************/
    encodeCaps->EncodeModes          = OVE_AVC_FULL;
    encodeCaps->encode_cap_size      = sizeof(OVE_ENCODE_CAPS);
    encodeCaps->caps.encode_cap_full->max_picture_size_in_MB    = 0;
    encodeCaps->caps.encode_cap_full->min_picture_size_in_MB    = 0;
    encodeCaps->caps.encode_cap_full->num_picture_formats       = 0;
    encodeCaps->caps.encode_cap_full->num_Profile_level         = 0;
    encodeCaps->caps.encode_cap_full->max_bit_rate              = 0;
    encodeCaps->caps.encode_cap_full->min_bit_rate              = 0;
	encodeCaps->caps.encode_cap_full->supported_task_priority   = OVE_ENCODE_TASK_PRIORITY_LEVEL1;

    for(int32 j=0; j<OVE_MAX_NUM_PICTURE_FORMATS_H264; j++)
        encodeCaps->caps.encode_cap_full->supported_picture_formats[j] = OVE_PICTURE_FORMAT_NONE;

    for(int32 j=0; j<OVE_MAX_NUM_PROFILE_LEVELS_H264; j++)
    {
        encodeCaps->caps.encode_cap_full->supported_profile_level[j].profile = 0;
        encodeCaps->caps.encode_cap_full->supported_profile_level[j].level   = 0;
    }

	/**************************************************************************/
	/* Get the device capabilities                                            */
	/**************************************************************************/
    status = OVEncodeGetDeviceCap(oveContext,
                                    oveDeviceID,
                                    encodeCaps->encode_cap_size,
                                    &numCaps,
                                    encodeCaps);
    return(status);
}

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
bool CodecInst::getDeviceInfo(ovencode_device_info **deviceInfo,
					 uint32 *numDevices)
{
	bool status;
	status = OVEncodeGetDeviceInfo(numDevices, 0);
    if(!status)
    {
        Log(L"OVEncodeGetDeviceInfo failed!\n");
        return false;
    }
    else
    {
        if(*numDevices == 0)
        {
            Log(L"No suitable devices found!\n");
            return false;
        }
    }
	/**************************************************************************/
    /* Get information about each device found                                */
	/**************************************************************************/
    *deviceInfo = new ovencode_device_info[*numDevices];
	memset(*deviceInfo,0,sizeof(ovencode_device_info)* (*numDevices));
    status = OVEncodeGetDeviceInfo(numDevices, *deviceInfo);
    if(!status)
    {
        Log(L"OVEncodeGetDeviceInfo failed!\n");
        return false;
    }
	return true;
}

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
bool CodecInst::gpuCheck(cl_platform_id platform,cl_device_type* dType)
{
	cl_int err;
	cl_context_properties cps[3] = 
	{
		CL_CONTEXT_PLATFORM, 
		(cl_context_properties)platform, 
		0
	};

	cl_context context = f_clCreateContextFromType(cps,
												(*dType),
												NULL,
												NULL,
												&err);
	f_clReleaseContext(context);

	if(err == CL_DEVICE_NOT_FOUND)
	{
		Log(L"GPU not found. Fallback to CPU\n");
		*dType = CL_DEVICE_TYPE_CPU;
		return false;
	}

	return true;
}
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
bool CodecInst::getPlatform(cl_platform_id &platform)
{
    cl_uint numPlatforms;
	cl_int err = f_clGetPlatformIDs(0, NULL, &numPlatforms);
    if (CL_SUCCESS != err)
	{
        Log(L"clGetPlatformIDs() failed %d\n", err);
        return false;
    }
	/**************************************************************************/
    /* If there are platforms, make sure they are AMD.                        */
	/**************************************************************************/
    if (0 < numPlatforms) 
	{
        cl_platform_id* platforms = new cl_platform_id[numPlatforms];
        err = f_clGetPlatformIDs(numPlatforms, platforms, NULL);
        if (CL_SUCCESS != err) 
		{
            Log(L"clGetPlatformIDs() failed %d\n", err);
            delete [] platforms;
            return false;
		}
		/**********************************************************************/
        /* Loop through all the platforms looking for an AMD system.          */
		/**********************************************************************/
        for (uint32 i = 0; i < numPlatforms; ++i) 
        {
            int8 pbuf[100];
            err = f_clGetPlatformInfo(platforms[i],
                                    CL_PLATFORM_VENDOR,
                                    sizeof(pbuf),
                                    pbuf,
                                    NULL);
			/******************************************************************/
            /* Stop at the first platform that is an AMD system.              */
			/******************************************************************/
            if (!strcmp(pbuf, "Advanced Micro Devices, Inc.")) 
            {
                platform = platforms[i];
                break;
            }
        }
        delete [] platforms;
    }

    if (NULL == platform) 
    {
        Log(L"Couldn't find AMD platform, cannot proceed.\n");
        return false;
    }

    return true;
}

/**
	Utility function for configuration dialog.
*/
DeviceMap CodecInst::getDeviceList()
{
	DeviceMap devs;
	OVDeviceHandle hDev;
	memset(&hDev, 0, sizeof(hDev));

	wchar_t tmp[1024];
	
	if(getDevice(&hDev))
	{
		for(uint32 i=0; i < hDev.numDevices; i++)
		{
			uint32 deviceId = hDev.deviceInfo[i].device_id;
			cl_device_id clDevId = reinterpret_cast<cl_device_id>(deviceId);
#ifdef _M_X64
			// May ${DEITY} have mercy on us all.
			intptr_t ptr = intptr_t((intptr_t*)&clDevId);
			clDevId = (cl_device_id)((intptr_t)clDevId | (ptr & 0xFFFFFFFF00000000));
#endif

			// print device name
			size_t valueSize = 0;
			f_clGetDeviceInfo(clDevId, CL_DEVICE_NAME, 0, NULL, &valueSize);
			char* value = (char*) malloc(valueSize);
			f_clGetDeviceInfo(clDevId, CL_DEVICE_NAME, valueSize, value, NULL);
			cl_int iVal;
			//f_clGetDeviceInfo(clDevId, CL_DEVICE_ENDIAN_LITTLE, sizeof(iVal), &iVal, NULL);
			f_clGetDeviceInfo(clDevId, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(iVal), &iVal, NULL);
			
			//swprintf(tmp, 1023, L"%S (%S)", value, iVal == CL_TRUE ? "LE" : "BE");
			swprintf(tmp, 1023, L"%S (%d CU)", value, iVal);
			wstring wstr = tmp;
			devs.insert(pair<cl_device_id, wstring>(clDevId, wstr));
			free(value);
		}
	}

	if(hDev.deviceInfo) delete [] hDev.deviceInfo;

	return devs;
}

bool CodecInst::createCPUContext(cl_platform_id platform)
{
	cl_int err;
	cl_context_properties cps[3] = 
	{
		CL_CONTEXT_PLATFORM, 
		(cl_context_properties)platform, 
		0
	};

	mCpuCtx = f_clCreateContextFromType(cps,
				CL_DEVICE_TYPE_CPU,
				NULL,
				NULL,
				&err);

	if(err != CL_SUCCESS) {
		Log(L"Could not create CPU CL context. Error: %d.\n", err);
		return false;
	}

	cl_uint count = 1;
	err = f_clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &mCpuDev, &count);
	if(err != CL_SUCCESS || count == 0){
		Log(L"Could not get CPU device id. Error: %d.\n", err);
		return false;
	}

	mCpuCmdQueue = f_clCreateCommandQueue(mCpuCtx, mCpuDev, 0, &err);
	if(err != CL_SUCCESS) {
		Log(L"\nCreate command queue #0 failed! Error : %d\n", err);
		return false;
	}
	//clReleaseContext(context);
	return true;
}