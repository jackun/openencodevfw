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
	switch (errorCode)
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
	if (input == reference)
	{
		return SUCCESS;
	}
	else
	{
		if (isAPIerror)
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
	while (eventStatus != CL_COMPLETE)
	{
		status = f_clGetEventInfo(
			*event,
			CL_EVENT_COMMAND_EXECUTION_STATUS,
			sizeof(cl_int),
			&eventStatus,
			NULL);
		CHECK_OPENCL_ERROR(status, "clGetEventEventInfo Failed with Error Code:");
	}

	status = f_clReleaseEvent(*event);
	CHECK_OPENCL_ERROR(status, "clReleaseEvent Failed with Error Code:");

	return SUCCESS;
}

void clConvert::Cleanup_OpenCL()
{
	if (g_inputBuffer[0]) { f_clReleaseMemObject(g_inputBuffer[0]); g_inputBuffer[0] = NULL; }
	if (g_inputBuffer[1]) { f_clReleaseMemObject(g_inputBuffer[1]); g_inputBuffer[1] = NULL; }

	if (g_outputBuffer) { f_clReleaseMemObject(g_outputBuffer); g_outputBuffer = NULL; }
	if (g_y_kernel) { f_clReleaseKernel(g_y_kernel); g_y_kernel = NULL; }
	if (g_uv_kernel) { f_clReleaseKernel(g_uv_kernel); g_uv_kernel = NULL; }
	if (g_program) { f_clReleaseProgram(g_program); g_program = NULL; }
	//if( g_cmd_queue ) {f_clReleaseCommandQueue( g_cmd_queue ); g_cmd_queue = NULL;}
	//if( g_context ) {f_clReleaseContext( g_context ); g_context = NULL;}
	if (host_ptr) { free(host_ptr); host_ptr = NULL; }
	if (hRaw) { fclose(hRaw); hRaw = NULL; }
}

//Unused
int clConvert::setupCL()
{
	for (int i = 0; i < 2; i++)
	{
		g_cmd_queue[i] = f_clCreateCommandQueue(g_context, deviceID, 0, NULL);
		if (g_cmd_queue[i] == (cl_command_queue)0)
		{
			Cleanup_OpenCL();
			return FAILURE;
		}
	}
	return SUCCESS;
}


extern HMODULE hmoduleVFW;
int clConvert::createKernels(COLORMATRIX matrix)
{
	cl_int status;
	// create a CL program using the kernel source, load it from resource
	char* source;
	HRSRC hResource = FindResourceExA(hmoduleVFW, "STRING",
		MAKEINTRESOURCEA(IDR_OPENCL_KERNELS),
		MAKELANGID(LANG_NEUTRAL,
		SUBLANG_DEFAULT));

	if (hResource != NULL)
	{
		source = (char*)LoadResource(hmoduleVFW, hResource);
	}
	size_t sourceSize[] = { strlen(source) };
	g_program = f_clCreateProgramWithSource(g_context,
		1,
		(const char**)&source,
		sourceSize,
		&status);
	//free(source);
	CHECK_OPENCL_ERROR(status, "clCreateProgramWithSource failed.");

	std::string flagsStr(""); //"-save-temps"
	if (mOptimize)
		flagsStr.append("-cl-single-precision-constant -cl-mad-enable -cl-fast-relaxed-math -cl-unsafe-math-optimizations ");

	switch (matrix)
	{
	case BT709_FULL:
		flagsStr.append("-DBT709_FULL ");
		break;
	case BT601_FULL:
		flagsStr.append("-DBT601_FULL ");
		break;
	case BT709_LIMITED:
		flagsStr.append("-DBT709_LIMITED ");
		break;
	/*case BT601_LIMITED:
		flagsStr.append("-DBT601_LIMITED ");
		break;*/
	default:
		flagsStr.append("-DBT601_LIMITED ");
		break;
	}

	if (flagsStr.size() != 0)
		mLog->Log(L"Build Options are : %S\n", flagsStr.c_str());


	/* create a cl program executable for all the devices specified */
	status = f_clBuildProgram(g_program,
		1,
		&deviceID,
		flagsStr.c_str(),
		NULL,
		NULL);
	if (status != CL_SUCCESS)
	{
		if (status == CL_BUILD_PROGRAM_FAILURE)
		{
			cl_int logStatus;
			char * buildLog = NULL;
			size_t buildLogSize = 0;
			logStatus = f_clGetProgramBuildInfo(g_program,
				deviceID,
				CL_PROGRAM_BUILD_LOG,
				buildLogSize,
				buildLog,
				&buildLogSize);
			CHECK_OPENCL_ERROR(logStatus, "clGetProgramBuildInfo failed.");

			buildLog = (char*)malloc(buildLogSize);
			if (buildLog == NULL)
			{
				mLog->Log(L"Failed to allocate host memory.(buildLog)\n");
				return FAILURE;
			}
			memset(buildLog, 0, buildLogSize);

			logStatus = f_clGetProgramBuildInfo(g_program,
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

	char* kernels[][2] = {
			{ "BGRAtoNV12_Y", "BGRAtoNV12_UV" },
			{ "BGRtoNV12_Y", "BGRtoNV12_UV" },

			{ "RGBAtoNV12_Y", "RGBAtoNV12_UV" },
			{ "RGBtoNV12_Y", "RGBtoNV12_UV" },
	};

	int i = mRGB ? 2 : 0;
	if (bpp_bytes == 3)
		i++;

	g_y_kernel = f_clCreateKernel(g_program, kernels[i][0], &status);
	CHECK_OPENCL_ERROR(status, "clCreateKernel(Y) failed!");

	g_uv_kernel = f_clCreateKernel(g_program, kernels[i][1], &status);
	CHECK_OPENCL_ERROR(status, "clCreateKernel(UV) failed!");

	status = f_clGetKernelWorkGroupInfo(
		g_y_kernel,
		deviceID,
		CL_KERNEL_WORK_GROUP_SIZE,
		sizeof(temp),
		&temp,
		0);
	CHECK_OPENCL_ERROR(status, "clGetKernelWorkGroupInfo failed");

	mLog->Log(L"clGetKernelWorkGroupInfo: %d\n", temp);

	//TODO Should limit to half as work_dim is 2? May give invalid WG size on cpu atleast
	while (localThreads_Max[0] *
		localThreads_Max[1] < temp)
	{
		if (2 * localThreads_Max[0] *
			localThreads_Max[1] <= temp)
			localThreads_Max[0] *= 2;

		if (2 * localThreads_Max[0] *
			localThreads_Max[1] <= temp)
			localThreads_Max[1] *= 2;
	}

	return SUCCESS;
}

int clConvert::setKernelArgs(cl_kernel kernel, cl_mem input, cl_mem output)
{
	cl_int status = 0;

	// Set up kernel arguments
	status = f_clSetKernelArg(
		kernel,
		0,
		sizeof(cl_mem),
		&input);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg(input) failed!\n");

	status = f_clSetKernelArg(
		kernel,
		1,
		sizeof(cl_mem),
		&output);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg(output) failed!");

	status = f_clSetKernelArg(
		kernel,
		2,
		sizeof(int),
		&oAlignedWidth);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg(alignedWidth) failed!");
	return SUCCESS;
}

int clConvert::setKernelOffset(cl_kernel kernel, int offset)
{
	cl_int status = 0;

	// Set up kernel arguments
	status = f_clSetKernelArg(
		kernel,
		3,
		sizeof(int),
		&offset);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg(offset) failed!\n");
	return SUCCESS;
}

int clConvert::runKernel(cl_kernel kernel,
	cl_command_queue queue,
	size_t globalThreads[2],
	size_t localThreads[2],
	double *prof,
	bool wait)
{
	cl_int status = 0;

	cl_event ndrEvt;
	status = f_clEnqueueNDRangeKernel(
		queue,
		kernel,
		2,
		0,
		globalThreads,
		localThreads,
		0,
		0,
		wait ? &ndrEvt : NULL);
	CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed!");

	if (wait) {
		status = f_clFlush(queue);
		CHECK_OPENCL_ERROR(status, "clFlush failed");
	}

	// Wait for event and release event
	//status = waitForEventAndRelease(&ndrEvt);
	//CHECK_OPENCL_ERROR(status, "waitForEventAndRelease(ndrEvt) failed.");

	//set 'wait' to true for profiling. Also pass profiling option when creating command queues.
	if (wait) {
		status = f_clWaitForEvents(1, &ndrEvt);
		CHECK_OPENCL_ERROR(status, "clWaitForEvents failed.");
		profileEvent(ndrEvt, prof);
		status = f_clReleaseEvent(ndrEvt);
		CHECK_OPENCL_ERROR(status, "clRelease Event Failed");
	}
	return SUCCESS;
}

int clConvert::profileEvent(cl_event evt, double *prof)
{
	// Calculate performance
	cl_ulong startTime;
	cl_ulong endTime;
	cl_int status;

	if (!prof)
		return SUCCESS;

	// Get kernel profiling info
	status = f_clGetEventProfilingInfo(evt,
		CL_PROFILING_COMMAND_START,
		sizeof(cl_ulong),
		&startTime,
		0);
	CHECK_OPENCL_ERROR(status, "clGetEventProfilingInfo failed.(startTime)");

	status = f_clGetEventProfilingInfo(evt,
		CL_PROFILING_COMMAND_END,
		sizeof(cl_ulong),
		&endTime,
		0);
	CHECK_OPENCL_ERROR(status, "clGetEventProfilingInfo failed.(endTime)");

	// Cumulate time for each iteration
	*prof += 1e-9 * (endTime - startTime);
	return SUCCESS;
}

int clConvert::decodeInit()
{
	return FAILURE;
}

int clConvert::encodeInit(bool staggered, cl_mem dstBuffer)
{
	cl_int statusCL = CL_SUCCESS;
	profSecs1 = 0;
	profSecs2 = 0;
	prof2ndPass = false;

	//TODO Odd framebuffer sizes
	input_size = iWidth * iHeight * bpp_bytes;
	//int align = 4 * bpp_bytes - 1;//or always to 16 bytes (float4)?
	int align = 256 - 1;//or align to 256 bytes for faster memory access?
	int input_size_aligned = (input_size + align) & ~align;
	int input_size_half = iWidth * (iHeight >> 1) * bpp_bytes;

	//VCE encoder needs aligned input, adjust pitch here
	oAlignedWidth = ((iWidth + (256 - 1)) & ~(256 - 1));
	g_output_size = oAlignedWidth * oHeight * 3 / 2;

	// Create buffer to store the source frame
	g_inputBuffer[0] = f_clCreateBuffer(
		g_context,
		//1080p RGBA ~5GB/s *WriteBuffer, ~6.4GB/s map/memcpy, PCI-e x16 2.0
		//640x272 RGBA >~3GB/s map/memcpy, <~3GB/s WriteBuffer
		CL_MEM_READ_ONLY | CL_MEM_USE_PERSISTENT_MEM_AMD,
		//CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR, //~6GB/s map/memcpy
		staggered ? input_size_half : input_size_aligned,
		NULL,
		&statusCL);
	CHECK_OPENCL_ERROR(statusCL, "clCreateBuffer(g_inputBuffer[0]) failed!");
	/*cl_image_format fmt;
	fmt.image_channel_order = CL_BGRA;
	fmt.image_channel_data_type = CL_UNSIGNED_INT8;
	cl_image_desc desc;
	memset(&desc, 0, sizeof(desc));
	desc.image_type = CL_MEM_OBJECT_IMAGE2D;
	desc.image_height = iHeight;
	desc.image_width = iWidth;
	desc.image_array_size = 1;

	g_inputBuffer[0] = clCreateImage(g_context, CL_MEM_READ_ONLY,// | CL_MEM_COPY_HOST_PTR,
	&fmt, &desc, NULL, &statusCL);*/

	if (staggered) {
		g_inputBuffer[1] = f_clCreateBuffer(
			g_context,
			CL_MEM_READ_ONLY,
			input_size - input_size_half,
			NULL,
			&statusCL);
		CHECK_OPENCL_ERROR(statusCL, "clCreateBuffer(g_inputBuffer[1]) failed!");
	}
	else
		g_inputBuffer[1] = NULL;

	//overhead test
	setKernelArgs(g_y_kernel, g_inputBuffer[0], dstBuffer);
	setKernelArgs(g_uv_kernel, g_inputBuffer[0], dstBuffer);
	setKernelOffset(g_y_kernel, 0);
	setKernelOffset(g_uv_kernel, 0);

	if (hRaw) { fclose(hRaw); hRaw = NULL; }
	char tmp[1024];
	sprintf_s(tmp, "raw_%dx%d.nv12", oAlignedWidth, oHeight);
	//hRaw = fopen(tmp, "wb+");
	return SUCCESS;
}

//RGB(A) to NV12
//clFlushs may be unnecessery. nVidia OpenCL Best Practices oclCopyComputeOverlap sample.
//Cut buffer to half and start converting it as soon as 1st half is uploaded to device. Seems slower though.
int clConvert::convertStaggered(const uint8* srcPtr, cl_mem dstBuffer)
{
	cl_int status = CL_SUCCESS;
	bool flipped = true;
	size_t offset[] = { 0, 0 };
	size_t globalThreads[] = { iWidth, iHeight };
	size_t localThreads[] = { localThreads_Max[0],
		localThreads_Max[1] };

	g_outputBuffer = dstBuffer;
	int srcTotal = iWidth * iHeight * bpp_bytes;
	int srcSize1 = iWidth * (iHeight >> 1) * bpp_bytes;
	int srcSize2 = srcTotal - srcSize1;

#if 0
	//Debug, clear old output buffer
	cl_event inMapEvt;
	mapPtr = f_clEnqueueMapBuffer( g_cmd_queue[0], dstBuffer, CL_TRUE, CL_MAP_WRITE, 0, 
		oAlignedWidth * iHeight * 3/2, 0, NULL, NULL, &status);
	status = f_clFlush(g_cmd_queue[0]);

	//copy to mapped buffer
	memset(mapPtr, 128, oAlignedWidth * iHeight * 3/2);//set to gray

	cl_event unmapEvent;
	status = f_clEnqueueUnmapMemObject(g_cmd_queue[0], dstBuffer, mapPtr, 0, NULL, &unmapEvent);
	status = f_clFlush(g_cmd_queue[0]);
	waitForEventAndRelease(&unmapEvent);
#endif

	//**************************** Queue first half write ****************************
	status = f_clEnqueueWriteBuffer(g_cmd_queue[0], g_inputBuffer[0],
		CL_FALSE, 0, srcSize1, srcPtr, 0, NULL, NULL);
	CHECK_OPENCL_ERROR(status, "clEnqueueWriteBuffer() #1 failed");

	f_clFlush(g_cmd_queue[0]);

	//**************************** Run over first half ****************************
	globalThreads[0] = iWidth;
	globalThreads[1] = iHeight >> 1;
	//globalThreads[1] -= globalThreads[1] % 2;
	//TODO CPU driver will complain.
	localThreads[1] = localThreads_Max[1];// / (4 + thread_extra_div);

	setKernelArgs(g_y_kernel, g_inputBuffer[0], dstBuffer);
	setKernelArgs(g_uv_kernel, g_inputBuffer[0], dstBuffer);
	setKernelOffset(g_y_kernel, 0);

	int halfHeightFix = iHeight >> 1;
	halfHeightFix = -(halfHeightFix % 2) * 3;
	setKernelOffset(g_uv_kernel, halfHeightFix);

	// RGB -> NV12
	if (runKernel(g_y_kernel, g_cmd_queue[0], globalThreads, localThreads, &profSecs1, false))
	{
		mLog->Log(L"g_y_kernel #1 failed!\n");
		return FAILURE;
	}

	globalThreads[0] = iWidth / 2;
	globalThreads[1] = iHeight / 4;
	//TODO CPU driver will complain.
	localThreads[1] = localThreads_Max[1];// / (8 + thread_extra_div);

	if (runKernel(g_uv_kernel, g_cmd_queue[0], globalThreads, localThreads, &profSecs2, false))
	{
		mLog->Log(L"g_uv_kernel #1 failed!\n");
		return FAILURE;
	}

	//**************************** Write second half ****************************
	status = f_clEnqueueWriteBuffer(g_cmd_queue[1], g_inputBuffer[1],
		CL_FALSE, 0, srcSize2, srcPtr + srcSize1, 0, NULL, NULL);
	CHECK_OPENCL_ERROR(status, "clEnqueueWriteBuffer() #2 failed");

	// Push the compute for queue 0 & write for queue 1
	f_clFlush(g_cmd_queue[0]);
	f_clFlush(g_cmd_queue[1]);

	//**************************** Run over second half ****************************
	globalThreads[0] = iWidth;
	globalThreads[1] = iHeight >> 1;
	globalThreads[1] += globalThreads[1] % 2;
	//TODO CPU driver will complain.
	localThreads[1] = localThreads_Max[1];// / (4 + thread_extra_div);

	setKernelArgs(g_y_kernel, g_inputBuffer[1], dstBuffer);
	setKernelArgs(g_uv_kernel, g_inputBuffer[1], dstBuffer);

	if (flipped)
	{
		//may leave empty lines with odd sizes
		setKernelOffset(g_y_kernel, (iHeight >> 1));
		int quarterHeight = (iHeight >> 2) + halfHeightFix;
		setKernelOffset(g_uv_kernel, quarterHeight);
	}
	else
	{
		setKernelOffset(g_y_kernel, oAlignedWidth * (iHeight >> 1));
		setKernelOffset(g_uv_kernel, ((oAlignedWidth*iHeight) >> 2));
	}

	// RGB -> NV12
	if (runKernel(g_y_kernel, g_cmd_queue[1], globalThreads, localThreads, &profSecs1, false))
	{
		mLog->Log(L"g_y_kernel #2 failed!\n");
		return FAILURE;
	}

	globalThreads[0] = iWidth / 2;
	globalThreads[1] = iHeight / 4;
	//TODO CPU driver will complain.
	localThreads[1] = localThreads_Max[1];// / (8 + thread_extra_div);

	if (runKernel(g_uv_kernel, g_cmd_queue[1], globalThreads, localThreads, &profSecs2, false))
	{
		mLog->Log(L"g_uv_kernel #2 failed!\n");
		return FAILURE;
	}

	// Non Blocking Read of 1st half of output data, queue 0
	//clEnqueueReadBuffer(g_cmd_queue[0], g_outputBuffer,
	//	CL_FALSE, 0, szHalfBuffer, (void*)&fResult[0], 0, NULL, NULL);

	//**************************** FINISH ****************************
	// Push the compute for queue 1 & the read for queue 0
	//f_clFlush(g_cmd_queue[0]);
	//f_clFlush(g_cmd_queue[1]);

	f_clFinish(g_cmd_queue[0]);
	f_clFinish(g_cmd_queue[1]);

	g_outputBuffer = NULL;

	return SUCCESS;
}

//RGB(A) to NV12
int clConvert::convert(const uint8* srcPtr, cl_mem dstBuffer, bool profile)
{
	cl_int status = CL_SUCCESS;
	size_t offset[] = { 0, 0 };
	size_t globalThreads[] = { iWidth, iHeight };
	size_t localThreads[] = { localThreads_Max[0] *
		localThreads_Max[1], 1 };

	cl_event inMapEvt;
	cl_event unmapEvent;
	captureTimeStart(mProf, 5);
#if 1
	mapPtr = f_clEnqueueMapBuffer(g_cmd_queue[0],
		g_inputBuffer[0],
		//g_pinnedBuffer,
		CL_TRUE,
		//CL_FALSE,
		CL_MAP_WRITE_INVALIDATE_REGION,
		0,
		input_size,
		0,
		NULL,
		NULL,//&inMapEvt,
		&status);
	CHECK_OPENCL_ERROR(status, "clEnqueueMapBuffer() failed");
	//sync at unmapping
	//status = clFlush(g_cmd_queue);
	//waitForEventAndRelease(&inMapEvt);

	//copy to mapped buffer or clEnqueueWriteBuffer instead
	memcpy(mapPtr, srcPtr, input_size);

	status = f_clEnqueueUnmapMemObject(g_cmd_queue[0],
		g_inputBuffer[0],
		//g_pinnedBuffer,
		mapPtr,
		0,
		NULL,
		&unmapEvent);
	status = f_clFlush(g_cmd_queue[0]);
	waitForEventAndRelease(&unmapEvent);

#else
	size_t origin[] = {0,0,0};
	size_t region[] = {iWidth,iHeight,1};
	status = f_clEnqueueWriteImage(g_cmd_queue[0],
		g_inputBuffer[0],
		CL_TRUE,
		origin,
		region,
		iWidth*bpp_bytes,
		0,
		srcPtr,
		0,
		NULL,
		NULL);//&inMapEvt);
	CHECK_OPENCL_ERROR(status, "clEnqueueWriteBuffer() failed");
#endif
	captureTimeStop(mProf, 5);

	/*setKernelArgs(g_y_kernel, g_inputBuffer[0], dstBuffer);
	setKernelArgs(g_uv_kernel, g_inputBuffer[0], dstBuffer);
	setKernelOffset(g_y_kernel, 0);
	setKernelOffset(g_uv_kernel, 0);*/

	if (runKernel(g_y_kernel, g_cmd_queue[0], globalThreads, NULL/*localThreads*/, &profSecs1, profile))
	{
		mLog->Log(L"kernelY failed!\n");
		return FAILURE;
	}

	//encoder should be feeding divideable by 2 frames anyway
	globalThreads[0] = (iWidth >> 1);
	//globalThreads[0] -= globalThreads[0] % 2;
	globalThreads[1] = (iHeight >> 1);
	//globalThreads[1] -= globalThreads[1] % 2;
	//mLog->Log(L"GID: %dx%d\n", globalThreads[0],globalThreads[1]);
	if (runKernel(g_uv_kernel, g_cmd_queue[0], globalThreads, NULL/*localThreads*/, &profSecs2, profile))
	{
		mLog->Log(L"kernelUV failed!\n");
		return FAILURE;
	}

	f_clFinish(g_cmd_queue[0]);
	//f_clFinish(g_cmd_queue[1]);

	//average from second sample
	if (prof2ndPass) {
		profSecs1 /= 2;
		profSecs2 /= 2;
	}
	else
		prof2ndPass = true;

	if (hRaw) {
		mapPtr = f_clEnqueueMapBuffer(g_cmd_queue[0],
			dstBuffer,
			CL_TRUE,
			CL_MAP_READ,
			0,
			g_output_size,
			0,
			NULL,
			&inMapEvt,
			&status);
		CHECK_OPENCL_ERROR(status, "clEnqueueMapBuffer() failed");
		status = f_clFlush(g_cmd_queue[0]);
		waitForEventAndRelease(&inMapEvt);
		fwrite(mapPtr, 1, g_output_size, hRaw);
		status = f_clEnqueueUnmapMemObject(g_cmd_queue[0],
			dstBuffer,
			mapPtr,
			0,
			NULL,
			&unmapEvent);
		status = f_clFlush(g_cmd_queue[0]);
		waitForEventAndRelease(&unmapEvent);
		fclose(hRaw); hRaw = NULL;

		hRaw = fopen("rgb.raw", "wb");
		fwrite(srcPtr, 1, input_size, hRaw);

		fclose(hRaw); hRaw = NULL;
	}
	return SUCCESS;
}
