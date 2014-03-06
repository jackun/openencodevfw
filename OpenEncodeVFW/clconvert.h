#ifndef _CLCONVERT
#define _CLCONVERT

#include <string>
#include <map>
#include <OpenVideo\OVEncode.h>
#include <OpenVideo\OVEncodeTypes.h>
#include <cl\cl.h>
#include <CL/cl_gl.h>
#include <CL/cl_ext.h>
#include "OvEncodeTypedef.h"
#include "log.h"

#define SUCCESS 0
#define FAILURE 1
#define EXPECTED_FAILURE 2

#define CHECK_ALLOCATION(actual, msg) \
        if(actual == NULL) \
        { \
            std::cerr << "Location : " << __FILE__ << ":" << __LINE__<< std::endl; \
            return FAILURE; \
        }


#define CHECK_ERROR(actual, reference, msg) \
        if(actual != reference) \
        { \
            std::cerr << "Location : " << __FILE__ << ":" << __LINE__<< std::endl; \
            return FAILURE; \
        }

#define CHECK_OPENCL_ERROR(actual, msg) \
	if(checkVal(actual, CL_SUCCESS, msg, true)) \
        { \
            std::cerr << "Location : " << __FILE__ << ":" << __LINE__<< std::endl; \
            return FAILURE; \
        } 

#define OPENCL_EXPECTED_ERROR(msg) \
        { \
            std::cerr<<"Expected Error: "<<msg<<std::endl;\
            return EXPECTED_FAILURE; \
        }

class clConvert
{
public:


	clConvert(cl_context ctx, cl_device_id dev, cl_command_queue cmdqueue, 
			int width, int height, unsigned int _bpp_bytes, Logger *log, 
			bool opt = true, bool limit = true):
		g_context(ctx), deviceID(dev),
		iWidth(width), oWidth(width), 
		iHeight(height), oHeight(height), bpp_bytes(_bpp_bytes),
		g_nv12_to_rgba_kernel(NULL), g_rgba_to_nv12_kernel(NULL),
		g_nv12_to_rgb_kernel(NULL), g_rgb_to_nv12_kernel(NULL), g_rgb_blend_kernel(NULL), g_rgba_blend_kernel(NULL),
		host_ptr(NULL), g_output_size(0), g_inputBuffer(NULL), g_pinnedBuffer(NULL), g_outputBuffer(NULL), g_blendBuffer(NULL),
		g_cmd_queue(cmdqueue), g_program(NULL), g_decoded_frame(NULL), mLog(log),
		mOptimize(opt), mColSpaceLimit(limit)
	{
		localThreads_nv12_to_rgba_kernel[0] = 1;
		localThreads_nv12_to_rgba_kernel[1] = 1;
		localThreads_rgba_to_nv12_kernel[0] = 1;
		localThreads_rgba_to_nv12_kernel[1] = 1;
	}

	~clConvert()
	{
		Cleanup_OpenCL();
	}

	bool init()
	{
		//if(setupCL() == SUCCESS && )
		if(createKernels() == SUCCESS && encodeInit() == SUCCESS)
			return true;
		return false;
	}

	
	int encode(const uint8* srcPtr, uint32 srcSize, cl_mem dstBuffer);
	int blendAndEncode(const uint8* srcPtr1, uint32 srcSize1, 
		const uint8* srcPtr2, uint32 srcSize2,
		uint8* dstPtr, uint32 dstSize);

private:
	cl_platform_id		platform; //OVEncode CL platform ?
	
	std::string			deviceType;// = "cpu";
	unsigned int		num_event_in_wait_list;
	int					iWidth; //input
	int					iHeight;
	int					oWidth; //output
	int					oHeight;
	int					oAlignedWidth;
	unsigned int		bpp_bytes;
	void				*host_ptr;
	void 				*g_decoded_frame;
	void				*mapPtr;
	std::map<const uint8*, cl_mem>	bufferMap;
	int					g_output_size;
	cl_mem				g_inputBuffer;
	cl_mem				g_pinnedBuffer;
	cl_mem				g_outputBuffer;
	cl_mem				g_blendBuffer;
	cl_context			g_context;
	cl_command_queue 	g_cmd_queue;
	cl_program			g_program;

	// Kernels
	cl_kernel           g_nv12_to_rgba_kernel;
	cl_kernel           g_nv12_to_rgb_kernel;
	cl_kernel           g_rgba_to_nv12_kernel;
	cl_kernel           g_rgba_to_uv_kernel;
	cl_kernel           g_rgb_to_nv12_kernel;
	cl_kernel           g_rgb_to_uv_kernel;
	cl_kernel           g_rgb_blend_kernel;
	cl_kernel           g_rgba_blend_kernel;
	size_t localThreads_nv12_to_rgba_kernel[2];// = {1, 1};
	size_t localThreads_rgba_to_nv12_kernel[2];// = {1, 1};

	bool g_bRunOnGPU;// = false;
	cl_device_id deviceID;// = 0;
	Logger *mLog;
	bool	mOptimize;
	bool	mColSpaceLimit;

	int setupCL();
	int decodeInit();
	int encodeInit();
	int waitForEventAndRelease(cl_event *event);
	void Cleanup_OpenCL();
	int createKernels();
	bool runNV12ToRGBKernel(
						size_t globalThreads[2],
						size_t localThreads[2]);
	bool runRGBToNV12Kernel(
						cl_kernel kernel,
						size_t globalThreads[2],
						size_t localThreads[2], bool blend);
	template<typename T>
	int checkVal(
		T input, 
		T reference, 
		std::string message,
		bool isAPIerror);
};


#endif