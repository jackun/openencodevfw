#ifndef _CLCONVERT
#define _CLCONVERT

#include <string>
#include <map>
#include "OVEncodeDyn.h"
#include "OVEncodeTypes.h"
#include <cl/cl.h>
#include "OvEncodeTypedef.h"
#include "log.h"
#include "perf.h"
#include "colorspace.h"

#define SUCCESS 0
#define FAILURE 1
#define EXPECTED_FAILURE 2

#define CHECK_ALLOCATION(actual, msg) \
        if(actual == NULL) \
        { \
            mLog->Log(L"Location : %S : %d\n", __FILE__, __LINE__); \
            return FAILURE; \
        }


#define CHECK_ERROR(actual, reference, msg) \
        if(actual != reference) \
        { \
            mLog->Log(L"Location : %S : %d\n", __FILE__, __LINE__); \
            return FAILURE; \
        }

#define CHECK_OPENCL_ERROR(actual, msg) \
    if(checkVal(actual, CL_SUCCESS, msg, true)) \
        { \
            mLog->Log(L"Location : %S : %d\n", __FILE__, __LINE__); \
            return FAILURE; \
        } 

#define OPENCL_EXPECTED_ERROR(msg) \
        { \
            mLog->Log(L"Expected Error %S\n", msg); \
            return EXPECTED_FAILURE; \
        }

class clConvert
{
public:
	double	profSecs1,profSecs2;
	bool prof2ndPass;

	clConvert(cl_context ctx, cl_device_id dev, cl_command_queue cmdqueue[2], 
			int width, int height, unsigned int _bpp_bytes, Logger *lg, OVprofile *prof,
			bool opt = true, bool rgb = false):
		g_context(ctx), deviceID(dev),
		iWidth(width), oWidth(width), 
		iHeight(height), oHeight(height), bpp_bytes(_bpp_bytes),
		g_y_kernel(NULL), g_uv_kernel(NULL),
		host_ptr(NULL), g_output_size(0), g_outputBuffer(NULL),
		g_program(NULL), mLog(lg), mProf(prof),
		mOptimize(opt),
		profSecs1(0), profSecs2(0), prof2ndPass(false),
		hRaw(NULL), mRGB(rgb)
	{
		localThreads_Max[0] = 1;
		localThreads_Max[1] = 1;

		g_cmd_queue[0] = cmdqueue[0];
		g_cmd_queue[1] = cmdqueue[1];
		g_inputBuffer[0] = NULL;
		g_inputBuffer[1] = NULL;
	}

	~clConvert()
	{
		Cleanup_OpenCL();
	}

	/*bool init()
	{
		//if(setupCL() == SUCCESS && )
		if(createKernels() == SUCCESS && encodeInit(false) == SUCCESS)
			return true;
		return false;
	}*/

	int convert(const uint8* srcPtr, cl_mem dstBuffer, bool profile);

	int decodeInit();
	int encodeInit(cl_mem dstBuffer);
	int createKernels(COLORMATRIX matrix);

private:
	//cl_platform_id		platform; //OVEncode CL platform ?
	
	std::string			deviceType;// = "cpu";
	unsigned int		num_event_in_wait_list;
	int					iWidth; //input
	int					iHeight;
	int					oWidth; //output
	int					oHeight;
	int					oAlignedWidth;
	unsigned int		bpp_bytes;
	void				*host_ptr;
	void				*mapPtr;
	std::map<const uint8*, cl_mem>	bufferMap;
	int					input_size;
	int					g_output_size;
	cl_mem				g_inputBuffer[2];
	cl_mem				g_outputBuffer;
	cl_context			g_context;
	cl_command_queue 	g_cmd_queue[2];
	cl_program			g_program;
	FILE				*hRaw;
	OVprofile			*mProf;

	// Kernels
	cl_kernel           g_y_kernel;
	cl_kernel           g_uv_kernel;
	size_t localThreads_Max[2];// = {1, 1};

	bool g_bRunOnGPU;// = false;
	cl_device_id deviceID;// = 0;
	Logger *mLog;
	bool	mOptimize;
	bool	mRGB;

	int setupCL();
	int waitForEventAndRelease(cl_event *event);
	void Cleanup_OpenCL();

	int setKernelArgs(cl_kernel kernel, cl_mem input, cl_mem output);
	int setKernelOffset(cl_kernel kernel, int offset);
	int runKernel(cl_kernel kernel,
				cl_command_queue queue,
				size_t globalThreads[2],
				size_t localThreads[2],
				double *prof,
				bool wait);
	int profileEvent(cl_event evt, double *prof);

	template<typename T>
	int checkVal(
		T input, 
		T reference, 
		std::string message,
		bool isAPIerror);
};


#endif