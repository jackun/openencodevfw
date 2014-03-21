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
    if( g_inputBuffer[0] ) {clReleaseMemObject( g_inputBuffer[0] ); g_inputBuffer[0] = NULL;}
    if( g_inputBuffer[1] ) {clReleaseMemObject( g_inputBuffer[1] ); g_inputBuffer[1] = NULL;}

    if( g_outputBuffer ) {clReleaseMemObject( g_outputBuffer ); g_outputBuffer = NULL;}
    if( g_nv12_to_rgba_kernel ) {clReleaseKernel( g_nv12_to_rgba_kernel ); g_nv12_to_rgba_kernel = NULL;}
    if( g_rgba_to_nv12_kernel ) {clReleaseKernel( g_rgba_to_nv12_kernel ); g_rgba_to_nv12_kernel = NULL;}
    if( g_rgba_to_uv_kernel ) {clReleaseKernel( g_rgba_to_uv_kernel ); g_rgba_to_uv_kernel = NULL;}
    if( g_rgb_to_nv12_kernel ) {clReleaseKernel( g_rgb_to_nv12_kernel ); g_rgb_to_nv12_kernel = NULL;}
    if( g_rgb_to_uv_kernel ) {clReleaseKernel( g_rgb_to_uv_kernel ); g_rgb_to_uv_kernel = NULL;}
    if( g_program ) {clReleaseProgram( g_program ); g_program = NULL;}
    //if( g_cmd_queue ) {clReleaseCommandQueue( g_cmd_queue ); g_cmd_queue = NULL;}
    //if( g_context ) {clReleaseContext( g_context ); g_context = NULL;}
    if(host_ptr) { free(host_ptr); host_ptr = NULL;}
    if(g_decoded_frame) {free(g_decoded_frame); g_decoded_frame = NULL;} 
}

//Unused
int clConvert::setupCL()
{
    for(int i = 0; i < 2; i++)
    {
        g_cmd_queue[i] = clCreateCommandQueue(g_context, deviceID, 0, NULL);
        if (g_cmd_queue[i] == (cl_command_queue)0)
        {
            Cleanup_OpenCL();
            return FAILURE;
        }
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
    CHECK_OPENCL_ERROR(status, "clCreateKernel(RGBAtoNV12) #1 failed!");

    //TODO Reusing localThreads_Max, should be fine?
    g_rgb_to_nv12_kernel = clCreateKernel(g_program, "RGBtoNV12", &status);
    CHECK_OPENCL_ERROR(status, "clCreateKernel(RGBtoNV12) failed!");

    g_rgba_to_uv_kernel = clCreateKernel(g_program, "RGBAtoNV12_UV", &status);
    CHECK_OPENCL_ERROR(status, "clCreateKernel(RGBAtoNV12_UV) failed!");

    g_rgb_to_uv_kernel = clCreateKernel(g_program, "RGBtoNV12_UV", &status);
    CHECK_OPENCL_ERROR(status, "clCreateKernel(RGBtoNV12_UV) failed!");

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
    while(localThreads_Max[0] * 
          localThreads_Max[1] < temp)
    {
        if(2 * localThreads_Max[0] *
           localThreads_Max[1] <= temp)
            localThreads_Max[0] *= 2;

        if(2 * localThreads_Max[0] *
           localThreads_Max[1] <= temp)
            localThreads_Max[1] *= 2;
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
                &g_inputBuffer[0]);
    CHECK_OPENCL_ERROR(status, "clSetKernelArg(g_inputBuffer) failed!\n");


    status = clSetKernelArg(
                kernel, 
                1, 
                sizeof(cl_mem), 
                &g_outputBuffer);
    CHECK_OPENCL_ERROR(status, "clSetKernelArg(g_outputBuffer) failed!");

    cl_event ndrEvt;
    status = clEnqueueNDRangeKernel(
                g_cmd_queue[0],
                kernel,
                2,
                0,
                globalThreads,
                localThreads,
                0, 
                0, 
                &ndrEvt);
    CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel(g_nv12_to_rgb(a)_kernel) failed!");

    status = clFlush(g_cmd_queue[0]);
    CHECK_OPENCL_ERROR(status, "clFlush failed");

    // Wait for event and release event
    status = waitForEventAndRelease(&ndrEvt);
    CHECK_OPENCL_ERROR(status, "waitForEventAndRelease(ndrEvt) failed.");

    return SUCCESS;
}

bool clConvert::runRGBToNV12Kernel(
                        cl_kernel kernel,
                        size_t globalThreads[2],
                        size_t localThreads[2], bool blend)
{
    cl_int status = 0;

    // Set up kernel arguments
    status = clSetKernelArg(
                kernel, 
                0, 
                sizeof(cl_mem), 
                &g_inputBuffer[0]);
    CHECK_OPENCL_ERROR(status, "clSetKernelArg(g_inputBuffer) failed!\n");


    status = clSetKernelArg(
                kernel, 
                1, 
                sizeof(cl_mem), 
                &g_outputBuffer);
    CHECK_OPENCL_ERROR(status, "clSetKernelArg(g_outputBuffer) failed!");

    status = clSetKernelArg(
                kernel, 
                2, 
                sizeof(int), 
                &oAlignedWidth);
    CHECK_OPENCL_ERROR(status, "clSetKernelArg(alignedWidth) failed!");

    cl_event ndrEvt;
    status = clEnqueueNDRangeKernel(
                g_cmd_queue[0],
                kernel,
                2,
                0,
                globalThreads,
                localThreads,
                0, 
                0, 
                &ndrEvt);
    CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed!");

    status = clFlush(g_cmd_queue[0]);
    CHECK_OPENCL_ERROR(status, "clFlush failed");

    // Wait for event and release event
    status = waitForEventAndRelease(&ndrEvt);
    CHECK_OPENCL_ERROR(status, "waitForEventAndRelease(ndrEvt) failed.");

    return SUCCESS;
}

int clConvert::setKernelArgs(cl_kernel kernel, cl_mem input, cl_mem output)
{
    cl_int status = 0;

    // Set up kernel arguments
    status = clSetKernelArg(
                kernel, 
                0, 
                sizeof(cl_mem), 
                &input);
    CHECK_OPENCL_ERROR(status, "clSetKernelArg(input) failed!\n");

    status = clSetKernelArg(
                kernel, 
                1, 
                sizeof(cl_mem), 
                &output);
    CHECK_OPENCL_ERROR(status, "clSetKernelArg(output) failed!");

    status = clSetKernelArg(
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
    status = clSetKernelArg(
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
    status = clEnqueueNDRangeKernel(
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

    if(wait) {
        status = clFlush(queue);
        CHECK_OPENCL_ERROR(status, "clFlush failed");
    }

    // Wait for event and release event
    //status = waitForEventAndRelease(&ndrEvt);
    //CHECK_OPENCL_ERROR(status, "waitForEventAndRelease(ndrEvt) failed.");
    
    //set 'wait' to true for profiling. Also pass profiling option when creating command queues.
    if(wait) {
        status = clWaitForEvents(1, &ndrEvt);
        CHECK_OPENCL_ERROR(status, "clWaitForEvents failed.");
        
        // Calculate performance
        cl_ulong startTime;
        cl_ulong endTime;
        
        // Get kernel profiling info
        status = clGetEventProfilingInfo(ndrEvt,
                                         CL_PROFILING_COMMAND_START,
                                         sizeof(cl_ulong),
                                         &startTime,
                                         0);
        CHECK_OPENCL_ERROR(status, "clGetEventProfilingInfo failed.(startTime)");

        status = clGetEventProfilingInfo(ndrEvt,
                                         CL_PROFILING_COMMAND_END,
                                         sizeof(cl_ulong),
                                         &endTime,
                                         0);
        CHECK_OPENCL_ERROR(status, "clGetEventProfilingInfo failed.(endTime)");

        // Cumulate time for each iteration
        if(prof)
            *prof += 1e-9 * (endTime - startTime);

        status = clReleaseEvent(ndrEvt);
        CHECK_OPENCL_ERROR(status, "clRelease Event Failed");
    }
    return SUCCESS;
}

int clConvert::decodeInit()
{
    return FAILURE;
}

int clConvert::encodeInit(bool staggered)
{
    cl_int statusCL = CL_SUCCESS;
    profSecs1 = 0;
    profSecs2 = 0;

    //TODO Odd framebuffer sizes
    input_size = iWidth * iHeight * bpp_bytes;
    int input_size_half = iWidth * (iHeight>>1) * bpp_bytes;

    //VCE encoder needs aligned input, adjust pitch here
    oAlignedWidth = ((iWidth + (256 - 1)) & ~(256 - 1));
    g_output_size = oAlignedWidth * oHeight * 3 / 2;

    // Create buffer to store the YUV image
    g_inputBuffer[0] = clCreateBuffer(
                                g_context, 
                                CL_MEM_READ_ONLY,
                                staggered ? input_size_half : input_size, 
                                NULL, 
                                &statusCL);
    CHECK_OPENCL_ERROR(statusCL , "clCreateBuffer(g_inputBuffer[0]) failed!");

    if(staggered) {
        g_inputBuffer[1] = clCreateBuffer(
                                    g_context, 
                                    CL_MEM_READ_ONLY,
                                    input_size - input_size_half, 
                                    NULL, 
                                    &statusCL);
        CHECK_OPENCL_ERROR(statusCL , "clCreateBuffer(g_inputBuffer[1]) failed!");
    } else
        g_inputBuffer[1] = NULL;

    return SUCCESS;
}

//RGB(A) to NV12
//clFlushs may be unnecessery. nVidia OpenCL Best Practices oclCopyComputeOverlap sample.
//Cut buffer to half and start converting it as soon as 1st half is uploaded to device. Seems slower though.
int clConvert::convertStaggered(const uint8* srcPtr, cl_mem dstBuffer)
{
    cl_int status = CL_SUCCESS;
    bool flipped = true;
    size_t offset[] = {0, 0};
    size_t globalThreads[] = {iWidth, iHeight};
    size_t localThreads[] = {localThreads_Max[0],
                             localThreads_Max[1]};

    g_outputBuffer = dstBuffer;
    int srcTotal = iWidth * iHeight * bpp_bytes;
    int srcSize1 = iWidth * (iHeight>>1) * bpp_bytes;
    int srcSize2 = srcTotal - srcSize1;

#if 0
    //Debug, clear old output buffer
    cl_event inMapEvt;
    mapPtr = clEnqueueMapBuffer( g_cmd_queue[0], dstBuffer, CL_TRUE, CL_MAP_WRITE, 0, 
        oAlignedWidth * iHeight * 3/2, 0, NULL, NULL, &status);
    status = clFlush(g_cmd_queue[0]);

    //copy to mapped buffer
    memset(mapPtr, 128, oAlignedWidth * iHeight * 3/2);//set to gray

    cl_event unmapEvent;
    status = clEnqueueUnmapMemObject(g_cmd_queue[0], dstBuffer, mapPtr, 0, NULL, &unmapEvent);
    status = clFlush(g_cmd_queue[0]);
    waitForEventAndRelease(&unmapEvent);
#endif

    //**************************** Queue first half write ****************************
    status = clEnqueueWriteBuffer(g_cmd_queue[0], g_inputBuffer[0],
        CL_FALSE, 0, srcSize1, srcPtr, 0, NULL, NULL);
    CHECK_OPENCL_ERROR(status, "clEnqueueWriteBuffer() #1 failed");

    clFlush(g_cmd_queue[0]);

    //**************************** Run over first half ****************************
    globalThreads[0] = iWidth;
    globalThreads[1] = iHeight >> 1;
    //globalThreads[1] -= globalThreads[1] % 2;
    //TODO CPU driver will complain.
    localThreads[1] = localThreads_Max[1];// / (4 + thread_extra_div);

    setKernelArgs(g_rgba_to_nv12_kernel, g_inputBuffer[0], dstBuffer);
    setKernelArgs(g_rgba_to_uv_kernel,   g_inputBuffer[0], dstBuffer);
    setKernelOffset(g_rgba_to_nv12_kernel, 0);
    
    int halfHeightFix = iHeight>>1;
    halfHeightFix = -(halfHeightFix%2)*3;
    setKernelOffset(g_rgba_to_uv_kernel, halfHeightFix);

    // RGB -> NV12
    if(runKernel(g_rgba_to_nv12_kernel, g_cmd_queue[0], globalThreads, localThreads, &profSecs1, false))
    {
        mLog->Log(L"g_rgba_to_nv12_kernel #1 failed!\n");
        return FAILURE;
    }

    globalThreads[0] = iWidth / 2;
    globalThreads[1] = iHeight / 4;
    //TODO CPU driver will complain.
    localThreads[1] = localThreads_Max[1];// / (8 + thread_extra_div);

    if(runKernel(g_rgba_to_uv_kernel, g_cmd_queue[0], globalThreads, localThreads, &profSecs2, false))
    {
        mLog->Log(L"g_rgba_to_uv_kernel failed!\n");
        return FAILURE;
    }

    //**************************** Write second half ****************************
    status = clEnqueueWriteBuffer(g_cmd_queue[1], g_inputBuffer[1],
        CL_FALSE, 0, srcSize2, srcPtr + srcSize1, 0, NULL, NULL);
    CHECK_OPENCL_ERROR(status, "clEnqueueWriteBuffer() #2 failed");

    // Push the compute for queue 0 & write for queue 1
    clFlush(g_cmd_queue[0]);
    clFlush(g_cmd_queue[1]);

    //**************************** Run over second half ****************************
    globalThreads[0] = iWidth;
    globalThreads[1] = iHeight >> 1;
    globalThreads[1] += globalThreads[1] % 2;
    //TODO CPU driver will complain.
    localThreads[1] = localThreads_Max[1];// / (4 + thread_extra_div);

    setKernelArgs(g_rgba_to_nv12_kernel, g_inputBuffer[1], dstBuffer);
    setKernelArgs(g_rgba_to_uv_kernel,   g_inputBuffer[1], dstBuffer);

    if(flipped)
    {
        //may leave empty lines with odd sizes
        setKernelOffset(g_rgba_to_nv12_kernel, (iHeight>>1));
        int quarterHeight = (iHeight>>2) + halfHeightFix;
        setKernelOffset(g_rgba_to_uv_kernel,   quarterHeight);
    }
    else 
    {
        setKernelOffset(g_rgba_to_nv12_kernel, oAlignedWidth * (iHeight>>1));
        setKernelOffset(g_rgba_to_uv_kernel,   ((oAlignedWidth*iHeight)>>2));
    }

    // RGB -> NV12
    if(runKernel(g_rgba_to_nv12_kernel, g_cmd_queue[1], globalThreads, localThreads, &profSecs1, false))
    {
        mLog->Log(L"g_rgba_to_nv12_kernel #1 failed!\n");
        return FAILURE;
    }

    globalThreads[0] = iWidth / 2;
    globalThreads[1] = iHeight / 4;
    //TODO CPU driver will complain.
    localThreads[1] = localThreads_Max[1];// / (8 + thread_extra_div);

    if(runKernel(g_rgba_to_uv_kernel, g_cmd_queue[1], globalThreads, localThreads, &profSecs2, false))
    {
        mLog->Log(L"g_rgba_to_uv_kernel failed!\n");
        return FAILURE;
    }

    // Non Blocking Read of 1st half of output data, queue 0
    //clEnqueueReadBuffer(g_cmd_queue[0], g_outputBuffer,
    //	CL_FALSE, 0, szHalfBuffer, (void*)&fResult[0], 0, NULL, NULL);
        
    //**************************** FINISH ****************************
    // Push the compute for queue 1 & the read for queue 0
    clFlush(g_cmd_queue[0]);
    clFlush(g_cmd_queue[1]);

    clFinish(g_cmd_queue[0]);
    clFinish(g_cmd_queue[1]);
    
    g_outputBuffer = NULL;

    return SUCCESS;
}

//RGB(A) to NV12
int clConvert::convert(const uint8* srcPtr, cl_mem dstBuffer, bool profile)
{
    cl_int status = CL_SUCCESS;
    size_t offset[] = {0, 0};
    size_t globalThreads[] = {iWidth, iHeight};
    size_t localThreads[] = {localThreads_Max[0],
                             localThreads_Max[1]};
    cl_kernel kernelY, kernelUV;

    //could move to init probably
    if(bpp_bytes == 4) {
        kernelY = g_rgba_to_nv12_kernel;
        kernelUV = g_rgba_to_uv_kernel;
    } else {
        kernelY = g_rgb_to_nv12_kernel;
        kernelUV = g_rgb_to_uv_kernel;
    }

    cl_event inMapEvt;
    mapPtr = clEnqueueMapBuffer( g_cmd_queue[0],
                        g_inputBuffer[0],
                        //g_pinnedBuffer,
                        CL_TRUE, 
                        //CL_FALSE,
                        CL_MAP_WRITE,
                        0,
                        input_size,
                        0,
                        NULL,
                        NULL,//&inMapEvt,
                        &status);
    //sync at unmapping
    //status = clFlush(g_cmd_queue);
    //waitForEventAndRelease(&inMapEvt);
	CHECK_OPENCL_ERROR(status, "clEnqueueMapBuffer() failed");

    //copy to mapped buffer or clEnqueueWriteBuffer instead
    memcpy(mapPtr, srcPtr, input_size);

    //I guess the whole point here is that it wouldn't be a blocked write, so no use?
    //status = clEnqueueWriteBuffer(g_cmd_queue[0],
    //	g_inputBuffer[0],
    //	CL_TRUE, //blocking
    //	0,
    //	input_size,
    //	mapPtr, //mapped to g_pinnedBuffer
    //	0,
    //	NULL,
    //	0);
    //CHECK_OPENCL_ERROR(status, "clEnqueueWriteBuffer() failed");

    cl_event unmapEvent;
    status = clEnqueueUnmapMemObject(g_cmd_queue[0],
                                    g_inputBuffer[0], 
                                    //g_pinnedBuffer,
                                    mapPtr,
                                    0,
                                    NULL,
                                    &unmapEvent);
    status = clFlush(g_cmd_queue[0]);
    waitForEventAndRelease(&unmapEvent);

    setKernelArgs(kernelY, g_inputBuffer[0], dstBuffer);
    setKernelArgs(kernelUV, g_inputBuffer[0], dstBuffer);
    setKernelOffset(kernelY, 0);
    setKernelOffset(kernelUV, 0);

    if(runKernel(kernelY, g_cmd_queue[0], globalThreads, localThreads, &profSecs1, profile))
    {
        mLog->Log(L"kernelY failed!\n");
        return FAILURE;
    }

    //encoder should be feeding divideable by 16 frames anyway
    globalThreads[0] = (globalThreads[0] >> 1);
    //globalThreads[0] -= globalThreads[0] % 2;
    globalThreads[1] = (globalThreads[1] >> 1);
    //globalThreads[1] -= globalThreads[1] % 2;
    //mLog->Log(L"GID: %dx%d\n", globalThreads[0],globalThreads[1]);
    if(runKernel(kernelUV, g_cmd_queue[1], globalThreads, localThreads, &profSecs2, profile))
    {
        mLog->Log(L"kernelUV failed!\n");
        return FAILURE;
    }

    clFinish(g_cmd_queue[0]);
    clFinish(g_cmd_queue[1]);

    //average from second sample
    if(profSecs1 > 0 ) {
        profSecs1 /= 2;
        profSecs2 /= 2;
    }

    return SUCCESS;
}
