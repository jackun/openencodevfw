#include "stdafx.h"
#include "clconvert.h"

//void error(std::string err)
//{
//	std::cerr << err << std::endl;
//}

const char* 
getOpenCLErrorCodeStr(std::string input)
{
    return "unknown error code"; 
}

template<typename T>
const char* 
getOpenCLErrorCodeStr(T input)
{
    int errorCode = (int)input;
    switch(errorCode)
    {
        case CL_DEVICE_NOT_FOUND:
            return "CL_DEVICE_NOT_FOUND";
        case CL_DEVICE_NOT_AVAILABLE:
            return "CL_DEVICE_NOT_AVAILABLE";               
        case CL_COMPILER_NOT_AVAILABLE:
            return "CL_COMPILER_NOT_AVAILABLE";           
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            return "CL_MEM_OBJECT_ALLOCATION_FAILURE";      
        case CL_OUT_OF_RESOURCES:
            return "CL_OUT_OF_RESOURCES";                    
        case CL_OUT_OF_HOST_MEMORY:
            return "CL_OUT_OF_HOST_MEMORY";                 
        case CL_PROFILING_INFO_NOT_AVAILABLE:
            return "CL_PROFILING_INFO_NOT_AVAILABLE";        
        case CL_MEM_COPY_OVERLAP:
            return "CL_MEM_COPY_OVERLAP";                    
        case CL_IMAGE_FORMAT_MISMATCH:
            return "CL_IMAGE_FORMAT_MISMATCH";               
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
            return "CL_IMAGE_FORMAT_NOT_SUPPORTED";         
        case CL_BUILD_PROGRAM_FAILURE:
            return "CL_BUILD_PROGRAM_FAILURE";              
        case CL_MAP_FAILURE:
            return "CL_MAP_FAILURE";                         
        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
            return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
            return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
        case CL_INVALID_VALUE:
            return "CL_INVALID_VALUE";                      
        case CL_INVALID_DEVICE_TYPE:
            return "CL_INVALID_DEVICE_TYPE";               
        case CL_INVALID_PLATFORM:
            return "CL_INVALID_PLATFORM";                   
        case CL_INVALID_DEVICE:
            return "CL_INVALID_DEVICE";                    
        case CL_INVALID_CONTEXT:
            return "CL_INVALID_CONTEXT";                    
        case CL_INVALID_QUEUE_PROPERTIES:
            return "CL_INVALID_QUEUE_PROPERTIES";           
        case CL_INVALID_COMMAND_QUEUE:
            return "CL_INVALID_COMMAND_QUEUE";              
        case CL_INVALID_HOST_PTR:
            return "CL_INVALID_HOST_PTR";                   
        case CL_INVALID_MEM_OBJECT:
            return "CL_INVALID_MEM_OBJECT";                  
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
            return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";    
        case CL_INVALID_IMAGE_SIZE:
             return "CL_INVALID_IMAGE_SIZE";                 
        case CL_INVALID_SAMPLER:
            return "CL_INVALID_SAMPLER";                    
        case CL_INVALID_BINARY:
            return "CL_INVALID_BINARY";                     
        case CL_INVALID_BUILD_OPTIONS:
            return "CL_INVALID_BUILD_OPTIONS";              
        case CL_INVALID_PROGRAM:
            return "CL_INVALID_PROGRAM";                    
        case CL_INVALID_PROGRAM_EXECUTABLE:
            return "CL_INVALID_PROGRAM_EXECUTABLE";          
        case CL_INVALID_KERNEL_NAME:
            return "CL_INVALID_KERNEL_NAME";                
        case CL_INVALID_KERNEL_DEFINITION:
            return "CL_INVALID_KERNEL_DEFINITION";          
        case CL_INVALID_KERNEL:
            return "CL_INVALID_KERNEL";                     
        case CL_INVALID_ARG_INDEX:
            return "CL_INVALID_ARG_INDEX";                   
        case CL_INVALID_ARG_VALUE:
            return "CL_INVALID_ARG_VALUE";                   
        case CL_INVALID_ARG_SIZE:
            return "CL_INVALID_ARG_SIZE";                    
        case CL_INVALID_KERNEL_ARGS:
            return "CL_INVALID_KERNEL_ARGS";                
        case CL_INVALID_WORK_DIMENSION:
            return "CL_INVALID_WORK_DIMENSION";              
        case CL_INVALID_WORK_GROUP_SIZE:
            return "CL_INVALID_WORK_GROUP_SIZE";             
        case CL_INVALID_WORK_ITEM_SIZE:
            return "CL_INVALID_WORK_ITEM_SIZE";             
        case CL_INVALID_GLOBAL_OFFSET:
            return "CL_INVALID_GLOBAL_OFFSET";              
        case CL_INVALID_EVENT_WAIT_LIST:
            return "CL_INVALID_EVENT_WAIT_LIST";             
        case CL_INVALID_EVENT:
            return "CL_INVALID_EVENT";                      
        case CL_INVALID_OPERATION:
            return "CL_INVALID_OPERATION";                 
        case CL_INVALID_GL_OBJECT:
            return "CL_INVALID_GL_OBJECT";                  
        case CL_INVALID_BUFFER_SIZE:
            return "CL_INVALID_BUFFER_SIZE";                 
        case CL_INVALID_MIP_LEVEL:
            return "CL_INVALID_MIP_LEVEL";                   
        case CL_INVALID_GLOBAL_WORK_SIZE:
            return "CL_INVALID_GLOBAL_WORK_SIZE";            
        case CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR:
            return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
        case CL_PLATFORM_NOT_FOUND_KHR:
            return "CL_PLATFORM_NOT_FOUND_KHR";
        //case CL_INVALID_PROPERTY_EXT:
        //    return "CL_INVALID_PROPERTY_EXT";
        case CL_DEVICE_PARTITION_FAILED_EXT:
            return "CL_DEVICE_PARTITION_FAILED_EXT";
        case CL_INVALID_PARTITION_COUNT_EXT:
            return "CL_INVALID_PARTITION_COUNT_EXT"; 
        default:
            return "unknown error code";
    }

    return "unknown error code";
}

template<typename T>
int clConvert::checkVal(
    T input, 
    T reference, 
    std::string message,
    bool isAPIerror)
{
    if(input==reference)
    {
        return SUCCESS;
    }
    else
    {
        if(isAPIerror)
        {
            //std::cout<<"Error: "<< message << " Error code : ";
            //std::cout << getOpenCLErrorCodeStr(input) << std::endl;
			// TODO unicode vs ansi
			mLog->Log(L"Error: %S Error code: %S\n", message.c_str(), getOpenCLErrorCodeStr(input));
        }
        else
            //error(message);
			mLog->Log(L"%S", message);
        return FAILURE;
    }
}

int clConvert::waitForEventAndRelease(cl_event *event)
{
	cl_int status = CL_SUCCESS;
	cl_int eventStatus = CL_QUEUED;
	while(eventStatus != CL_COMPLETE)
	{
		status = clGetEventInfo(
						*event, 
						CL_EVENT_COMMAND_EXECUTION_STATUS, 
						sizeof(cl_int),
						&eventStatus,
						NULL);
		CHECK_OPENCL_ERROR(status, "clGetEventEventInfo Failed with Error Code:");
	}

	status = clReleaseEvent(*event);
	CHECK_OPENCL_ERROR(status, "clReleaseEvent Failed with Error Code:");

	return SUCCESS;
}

void clConvert::Cleanup_OpenCL()
{
    if( g_inputBuffer ) {clReleaseMemObject( g_inputBuffer ); g_inputBuffer = NULL;}
    if( g_outputBuffer ) {clReleaseMemObject( g_outputBuffer ); g_outputBuffer = NULL;}
	if( g_blendBuffer ) {clReleaseMemObject( g_blendBuffer ); g_blendBuffer = NULL;}
    if( g_nv12_to_rgba_kernel ) {clReleaseKernel( g_nv12_to_rgba_kernel ); g_nv12_to_rgba_kernel = NULL;}
    if( g_rgba_to_nv12_kernel ) {clReleaseKernel( g_rgba_to_nv12_kernel ); g_rgba_to_nv12_kernel = NULL;}
	if( g_rgb_to_nv12_kernel ) {clReleaseKernel( g_rgb_to_nv12_kernel ); g_rgb_to_nv12_kernel = NULL;}
	if( g_rgb_blend_kernel ) {clReleaseKernel( g_rgb_blend_kernel ); g_rgb_blend_kernel = NULL;}
	if( g_rgba_blend_kernel ) {clReleaseKernel( g_rgba_blend_kernel ); g_rgba_blend_kernel = NULL;}
    if( g_program ) {clReleaseProgram( g_program ); g_program = NULL;}
    //if( g_cmd_queue ) {clReleaseCommandQueue( g_cmd_queue ); g_cmd_queue = NULL;}
    //if( g_context ) {clReleaseContext( g_context ); g_context = NULL;}
    if(host_ptr) { free(host_ptr); host_ptr = NULL;}
	if(g_decoded_frame) {free(g_decoded_frame); g_decoded_frame = NULL;} 
}

int clConvert::setupCL()
{
    //cl_int status = CL_SUCCESS;
    //cl_device_type dType;
    //cl_device_id devices[16];
    //size_t cb;

    /*
     * Have a look at the available platforms and pick either
     * the AMD one if available or a reasonable default.
     */
    /*cl_platform_id platform = NULL;
    int retValue = getPlatform(platform);
    CHECK_ERROR(retValue, SUCCESS, "getPlatform() failed");

	cl_context_properties context_properties[3] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };
	
	// create the OpenCL context on a CPU/PG 
	if(g_bRunOnGPU)
	{
		g_context = clCreateContextFromType(context_properties, CL_DEVICE_TYPE_GPU, NULL, NULL, &status);
	}
	else
	{
		g_context = clCreateContextFromType(context_properties, CL_DEVICE_TYPE_CPU, NULL, NULL, &status);
	}
    if (g_context == (cl_context)0)
    {
		CHECK_OPENCL_ERROR(status, "clCreateContextFromType failed");
        return FAILURE;
	}
    
    
    // get the list of CPU devices associated with context
    status = clGetContextInfo(g_context, CL_CONTEXT_DEVICES, 0, NULL, &cb);
    clGetContextInfo(g_context, CL_CONTEXT_DEVICES, cb, devices, NULL);

	deviceID = devices[0];
	*/

    g_cmd_queue = clCreateCommandQueue(g_context, deviceID, 0, NULL);
    if (g_cmd_queue == (cl_command_queue)0)
    {
        Cleanup_OpenCL();
        return FAILURE;
    }
        
    return SUCCESS;
}


extern HMODULE hmoduleVFW;
int clConvert::createKernels()
{
    cl_int status;
	//#define SRCSIZE 32000 + 32000%2
    // create a CL program using the kernel source, load it from resource
	char* source;// = (LPSTR)malloc(SRCSIZE);
	//memset(source, 0, SRCSIZE);
	HRSRC hResource = FindResourceExA( hmoduleVFW, "STRING",
                                MAKEINTRESOURCEA(IDR_OPENCL_KERNELS),
                                MAKELANGID(LANG_NEUTRAL,
                                            SUBLANG_DEFAULT) );

	if( hResource != NULL )
    {
		source = (char*)LoadResource( hmoduleVFW, hResource );
	}
	//LoadStringA(hmoduleVFW, IDR_OPENCL_KERNELS, source, SRCSIZE);
	size_t sourceSize[] = {strlen(source)};
	g_program = clCreateProgramWithSource(g_context,
										1,
										(const char**)&source,
										sourceSize,
										&status);
	//free(source);
	CHECK_OPENCL_ERROR(status, "clCreateProgramWithSource failed.");

    std::string flagsStr(""); //"-save-temps"
	if(mOptimize)
		flagsStr.append("-cl-single-precision-constant -cl-mad-enable -cl-fast-relaxed-math -cl-unsafe-math-optimizations ");
	if(mColSpaceLimit)
		flagsStr.append("-DCOLORSPACE_LIMIT");

    if(flagsStr.size() != 0)
        mLog->Log(L"Build Options are : %S\n", flagsStr.c_str());
    

    /* create a cl program executable for all the devices specified */
    status = clBuildProgram(g_program, 
                            1, 
                            &deviceID, 
                            flagsStr.c_str(), 
                            NULL, 
                            NULL);
    if(status != CL_SUCCESS)
    {
        if(status == CL_BUILD_PROGRAM_FAILURE)
        {
            cl_int logStatus;
            char * buildLog = NULL;
            size_t buildLogSize = 0;
            logStatus = clGetProgramBuildInfo(g_program, 
                                              deviceID, 
                                              CL_PROGRAM_BUILD_LOG, 
                                              buildLogSize, 
                                              buildLog, 
                                              &buildLogSize);
            CHECK_OPENCL_ERROR(logStatus, "clGetProgramBuildInfo failed.");
            
            buildLog = (char*)malloc(buildLogSize);
            if(buildLog == NULL)
            {
                mLog->Log(L"Failed to allocate host memory.(buildLog)\n");
                return FAILURE;
            }
            memset(buildLog, 0, buildLogSize);

            logStatus = clGetProgramBuildInfo(g_program, 
                                              deviceID, 
                                              CL_PROGRAM_BUILD_LOG, 
                                              buildLogSize, 
                                              buildLog, 
                                              NULL);
            CHECK_OPENCL_ERROR(logStatus, "clGetProgramBuildInfo failed.");
			mLog->Log(
					L"\n\t\t\tBUILD LOG\n"
					L" ************************************************\n"
					L" %S"
					L" ************************************************\n",
					buildLog);
            free(buildLog);
        }

        CHECK_OPENCL_ERROR(status, "clBuildProgram() failed.");
    }

	size_t temp = 0;

    /* get a kernel object handle for a kernel with the given name */
    /*remove_pitch_kernel = clCreateKernel(program, "removePitch", &status);
    CHECK_OPENCL_ERROR(status, "clCreateKernel(removePitch) failed!");

    status = clGetKernelWorkGroupInfo(
                remove_pitch_kernel,
                deviceID,
                CL_KERNEL_WORK_GROUP_SIZE,
                sizeof(temp),
                &temp,
                0);
    CHECK_OPENCL_ERROR(status, "clGetKernelWorkGroupInfo failed");

    while(localThreads_remove_pitch_kernel[0] *
          localThreads_remove_pitch_kernel[1] < temp)
    {
        if(2 * localThreads_remove_pitch_kernel[0] *
           localThreads_remove_pitch_kernel[1] <= temp)
            localThreads_remove_pitch_kernel[0] *= 2;

        if(2 * localThreads_remove_pitch_kernel[0] *
           localThreads_remove_pitch_kernel[1] <= temp)
            localThreads_remove_pitch_kernel[1] *= 2;
    }*/

	//TODO Enable if you need NV12-to-RGB(A)
	//g_nv12_to_rgb_kernel = clCreateKernel(g_program, "NV12toRGB", &status);
    //CHECK_OPENCL_ERROR(status, "clCreateKernel(NV12toRGB) failed!");

    //g_nv12_to_rgba_kernel = clCreateKernel(g_program, "NV12toRGBA", &status);
    //CHECK_OPENCL_ERROR(status, "clCreateKernel(NV12toRGBA) failed!");

	 /*status = clGetKernelWorkGroupInfo(
                g_nv12_to_rgba_kernel,
                deviceID,
                CL_KERNEL_WORK_GROUP_SIZE,
                sizeof(temp),
                &temp,
                0);
    CHECK_OPENCL_ERROR(status, "clGetKernelWorkGroupInfo failed");

	std::cout << "clGetKernelWorkGroupInfo: " << temp << std::endl;
	
    while(localThreads_nv12_to_rgba_kernel[0] * 
          localThreads_nv12_to_rgba_kernel[1] < temp)
    {
        if(2 * localThreads_nv12_to_rgba_kernel[0] *
           localThreads_nv12_to_rgba_kernel[1] <= temp)
            localThreads_nv12_to_rgba_kernel[0] *= 2;

        if(2 * localThreads_nv12_to_rgba_kernel[0] *
           localThreads_nv12_to_rgba_kernel[1] <= temp)
            localThreads_nv12_to_rgba_kernel[1] *= 2;
    }*/

	g_rgba_to_nv12_kernel = clCreateKernel(g_program, "RGBAtoNV12", &status);
    CHECK_OPENCL_ERROR(status, "clCreateKernel(RGBAtoNV12) failed!");

	//TODO Reusing localThreads_rgba_to_nv12_kernel, should be fine?
	g_rgb_to_nv12_kernel = clCreateKernel(g_program, "RGBtoNV12", &status);
    CHECK_OPENCL_ERROR(status, "clCreateKernel(RGBtoNV12) failed!");

	g_rgb_blend_kernel = clCreateKernel(g_program, "RGBBlend", &status);
    CHECK_OPENCL_ERROR(status, "clCreateKernel(RGBtoNV12) failed!");

	status = clGetKernelWorkGroupInfo(
                g_rgb_to_nv12_kernel,
                deviceID,
                CL_KERNEL_WORK_GROUP_SIZE,
                sizeof(temp),
                &temp,
                0);
    CHECK_OPENCL_ERROR(status, "clGetKernelWorkGroupInfo failed");

	mLog->Log(L"clGetKernelWorkGroupInfo: %d\n", temp);
	
	//TODO Should limit to half as work_dim is 2? May give invalid WG size on cpu atleast
    while(localThreads_rgba_to_nv12_kernel[0] * 
          localThreads_rgba_to_nv12_kernel[1] < temp)
    {
        if(2 * localThreads_rgba_to_nv12_kernel[0] *
           localThreads_rgba_to_nv12_kernel[1] <= temp)
            localThreads_rgba_to_nv12_kernel[0] *= 2;

        if(2 * localThreads_rgba_to_nv12_kernel[0] *
           localThreads_rgba_to_nv12_kernel[1] <= temp)
            localThreads_rgba_to_nv12_kernel[1] *= 2;
    }


    return SUCCESS;
}

bool clConvert::runNV12ToRGBKernel(
						size_t globalThreads[2],
						size_t localThreads[2])
{
    cl_int status = 0;
	cl_kernel kernel;

	if(bpp_bytes == 4)
		kernel = g_nv12_to_rgba_kernel;
	else if(bpp_bytes == 3)
		kernel = g_nv12_to_rgb_kernel;
	else
		return false;
    // Set up kernel arguments
    status = clSetKernelArg(
                kernel, 
                0, 
                sizeof(cl_mem), 
                &g_inputBuffer);
    CHECK_OPENCL_ERROR(status, "clSetKernelArg(g_inputBuffer) failed!\n");


    status = clSetKernelArg(
                kernel, 
                1, 
                sizeof(cl_mem), 
                &g_outputBuffer);
    CHECK_OPENCL_ERROR(status, "clSetKernelArg(g_outputBuffer) failed!");

    cl_event ndrEvt;
    status = clEnqueueNDRangeKernel(
                g_cmd_queue,
                kernel,
                2,
                0,
                globalThreads,
                localThreads,
                0, 
                0, 
                &ndrEvt);
    CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel(g_nv12_to_rgb(a)_kernel) failed!");

    status = clFlush(g_cmd_queue);
    CHECK_OPENCL_ERROR(status, "clFlush failed");

    // Wait for event and release event
    status = waitForEventAndRelease(&ndrEvt);
    CHECK_OPENCL_ERROR(status, "waitForEventAndRelease(ndrEvt) failed.");

    return SUCCESS;
}

bool clConvert::runRGBToNV12Kernel(
						size_t globalThreads[2],
						size_t localThreads[2], bool blend)
{
    cl_int status = 0;
	cl_kernel kernel;

	if(bpp_bytes == 4)
		kernel = blend ? g_rgba_blend_kernel : g_rgba_to_nv12_kernel;
	else if(bpp_bytes == 3)
		kernel = blend ? g_rgb_blend_kernel : g_rgb_to_nv12_kernel;
	else
		return false;

    // Set up kernel arguments
    status = clSetKernelArg(
                kernel, 
                0, 
                sizeof(cl_mem), 
                &g_inputBuffer);
    CHECK_OPENCL_ERROR(status, "clSetKernelArg(g_outputBuffer) failed!\n");


    status = clSetKernelArg(
                kernel, 
                1, 
                sizeof(cl_mem), 
                &g_outputBuffer);
    CHECK_OPENCL_ERROR(status, "clSetKernelArg(g_inputBuffer) failed!");

	status = clSetKernelArg(
                kernel, 
                2, 
                sizeof(int), 
                &oAlignedWidth);
    CHECK_OPENCL_ERROR(status, "clSetKernelArg(alignedWidth) failed!");

	if(blend)
	status = clSetKernelArg(
                kernel, 
                3, 
                sizeof(cl_mem), 
                &g_blendBuffer);
    CHECK_OPENCL_ERROR(status, "clSetKernelArg(g_blendBuffer) failed!\n");

    cl_event ndrEvt;
    status = clEnqueueNDRangeKernel(
                g_cmd_queue,
                kernel,
                2,
                0,
                globalThreads,
                localThreads,
                0, 
                0, 
                &ndrEvt);
    CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel(g_rgb(a)_to_nv12_kernel) failed!");

    status = clFlush(g_cmd_queue);
    CHECK_OPENCL_ERROR(status, "clFlush failed");

    // Wait for event and release event
    status = waitForEventAndRelease(&ndrEvt);
    CHECK_OPENCL_ERROR(status, "waitForEventAndRelease(ndrEvt) failed.");

    return SUCCESS;
}

int clConvert::decodeInit()
{
	cl_int statusCL = CL_SUCCESS;
	
	// Size of NV12 format
    int host_ptr_size = iWidth * iHeight * 3 / 2;
    //host_ptr = malloc(host_ptr_size);
    //CHECK_ALLOCATION(host_ptr, "Failed to allocate memory(host_ptr)");
    
    // RGBA host buffer
    // RGB might need alignment to power of 2
    // " A built-in data type that is not a power of two bytes in size must be aligned to the next larger power of two. 
    //		This rule applies to built-in types only, not structs or unions. "
	oAlignedWidth = ((iWidth + (256 - 1)) & ~(256 - 1));
    g_output_size = oWidth * oHeight * bpp_bytes;
    //g_decoded_frame = malloc(g_output_size);
    //CHECK_ALLOCATION(g_decoded_frame, "Failed to allocate memory(g_decoded_frame)");
    
	// Create buffer to store the YUV image
    g_inputBuffer = clCreateBuffer(
                                g_context, 
                                CL_MEM_READ_ONLY, //_WRITE,
                                host_ptr_size, 
                                NULL, 
                                &statusCL);
    CHECK_OPENCL_ERROR(statusCL , "clCreateBuffer(g_inputBuffer) failed!");
    
    // Create buffer to store the output of NV12ToRGBA kernel
    g_outputBuffer = clCreateBuffer(
                    g_context, 
                    CL_MEM_READ_WRITE,
                    g_output_size,
                    NULL, 
                    &statusCL);
    CHECK_OPENCL_ERROR(statusCL, "clCreateBuffer(g_outputBuffer) failed!");
    
    return SUCCESS;
}

int clConvert::encodeInit()
{
	cl_int statusCL = CL_SUCCESS;
	
    int input_size = iWidth * iHeight * bpp_bytes;

	//VCE encoder needs aligned input, adjust pitch here
	oAlignedWidth = ((iWidth + (256 - 1)) & ~(256 - 1));
    g_output_size = oAlignedWidth * oHeight * 3 / 2;
    
	// Create buffer to store the YUV image
    g_inputBuffer = clCreateBuffer(
                                g_context, 
                                CL_MEM_READ_ONLY, //_WRITE,
                                input_size, 
                                NULL, 
                                &statusCL);
    CHECK_OPENCL_ERROR(statusCL , "clCreateBuffer(g_inputBuffer) failed!");
    
    // Create buffer to store the output
    /*g_outputBuffer = clCreateBuffer(
                    g_context, 
                    CL_MEM_READ_WRITE,
                    g_output_size,
                    NULL, 
                    &statusCL);
    CHECK_OPENCL_ERROR(statusCL, "clCreateBuffer(g_outputBuffer) failed!");*/
    
    g_blendBuffer = clCreateBuffer(
                                g_context, 
                                CL_MEM_READ_WRITE,
                                input_size, 
                                NULL, 
                                &statusCL);
    //CHECK_OPENCL_ERROR(statusCL , "clCreateBuffer(g_blendBuffer) failed!");
	if(checkVal(statusCL, CL_SUCCESS, "clCreateBuffer(g_blendBuffer) failed!", true) == FAILURE)
	{
		g_blendBuffer = NULL;
	}

    return SUCCESS;
}

//RGB to NV12
int clConvert::encode(const uint8* srcPtr, uint32 srcSize, cl_mem dstBuffer)
{
	cl_int status = CL_SUCCESS;
	size_t offset[] = {0, 0};
	size_t globalThreads[] = {iWidth, iHeight};
	size_t localThreads[] = {localThreads_rgba_to_nv12_kernel[0],
							 localThreads_rgba_to_nv12_kernel[1]};
	
	
	/*status = clEnqueueWriteBuffer(g_cmd_queue,
		g_inputBuffer,
		CL_TRUE,
		0,
		srcSize, //iWidth * iHeight * bpp_bytes,
		srcPtr,
		0,
		NULL,
		0);

	CHECK_OPENCL_ERROR(status, "clEnqueueWriteBuffer() failed");*/
	
	cl_event inMapEvt;
	void *mapPtr = clEnqueueMapBuffer( g_cmd_queue,
                        (cl_mem)g_inputBuffer,
                        CL_TRUE, //CL_FALSE,
                        CL_MAP_WRITE,
                        0,
                        srcSize,
                        0,
                        NULL,
                        &inMapEvt,
                        &status);

	status = clFlush(g_cmd_queue);
	waitForEventAndRelease(&inMapEvt);

	//copy to mapped buffer
	memcpy(mapPtr, srcPtr, srcSize);

	cl_event unmapEvent;
	status = clEnqueueUnmapMemObject(g_cmd_queue,
									g_inputBuffer,
									mapPtr,
									0,
									NULL,
									&unmapEvent);
	status = clFlush(g_cmd_queue);
	waitForEventAndRelease(&unmapEvent);
	
	g_outputBuffer = dstBuffer;
	if(runRGBToNV12Kernel(globalThreads, localThreads, false))
	{
		mLog->Log(L"runRGB(A)ToNV12Kernel failed!\n");
		return FAILURE;
	}
	g_outputBuffer = NULL;

	/*status = clEnqueueReadBuffer(
				g_cmd_queue, 
				g_outputBuffer, 
				CL_TRUE, 
				0, 
				g_output_size, 
				dstPtr, 
				0, 
				NULL, 
				0);
	CHECK_OPENCL_ERROR(status, "clEnqueueReadBuffer() failed");*/
	
	
	return SUCCESS;
}

//Pointless until we can drop frames from output somehow, don't use
int clConvert::blendAndEncode(const uint8* srcPtr1, uint32 srcSize1, 
	const uint8* srcPtr2, uint32 srcSize2,
	uint8* dstPtr, uint32 dstSize)
{
	cl_int status = CL_SUCCESS;
	size_t offset[] = {0, 0};
	size_t globalThreads[] = {iWidth, iHeight};
	size_t localThreads[] = {localThreads_rgba_to_nv12_kernel[0],
							 localThreads_rgba_to_nv12_kernel[1]};
	
	
	status = clEnqueueWriteBuffer(g_cmd_queue,
		g_inputBuffer,
		CL_TRUE /* blocking_write */,
		0 /* offset */,
		srcSize1, //iWidth * iHeight * bpp_bytes,
		srcPtr2,
		0      /* num_events_in_wait_list */,
		NULL   /* event_wait_list */,
		0      /* event */);

	CHECK_OPENCL_ERROR(status, "clEnqueueWriteBuffer() failed");
	
	status = clEnqueueWriteBuffer(g_cmd_queue,
		g_blendBuffer,
		CL_TRUE /* blocking_write */,
		0 /* offset */,
		srcSize2, //iWidth * iHeight * bpp_bytes,
		srcPtr2,
		0      /* num_events_in_wait_list */,
		NULL   /* event_wait_list */,
		0      /* event */);

	CHECK_OPENCL_ERROR(status, "clEnqueueWriteBuffer() failed");

	if(runRGBToNV12Kernel(globalThreads, localThreads, true))
	{
		mLog->Log(L"runRGB(A)ToNV12Kernel failed!\n");
		return FAILURE;
	}

	status = clEnqueueReadBuffer(
				g_cmd_queue, 
				g_outputBuffer, 
				CL_TRUE, 
				0, 
				g_output_size, 
				dstPtr, 
				0, 
				NULL, 
				0);
	CHECK_OPENCL_ERROR(status, "clEnqueueReadBuffer() failed");
	

	return SUCCESS;
}